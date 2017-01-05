/*
 * Copyright (C) 2017 Fredrik Noring. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "pec-decoder.h"
#include "pec-encoder.h"
#include "pes-encoder.h"

struct pes_stitch {
	int thread_index;
	float x;
	float y;
	bool jump;
};

struct pes_bounds {
	float min_x;
	float min_y;
	float max_x;
	float max_y;
	bool valid;
};

struct pes_encoder {
	struct pes_bounds bounds;
	struct pes_transform affine_transform;
	struct {
		float x;
		float y;
	} translation;

	int thread_count;
	struct pec_thread thread_list[PES_MAX_THREADS];

	int stitch_count;
	int stitch_capacity;
	struct pes_stitch *stitch_list;

	int block_count;

	struct pec_encoder *pec_encoder;
};

static bool encoded_size(const void * const data, const size_t size,
	void * const arg)
{
	int * const s = arg;

	*s += size;

	return *s >= 0;
}

static bool encode_u16lsb(const int value,
	const pes_encode_callback encode_cb, void * const arg)
{
	const uint8_t data[] = {
		(value >>  0) & 0xFF,
		(value >>  8) & 0xFF
	};

	return 0 <= value && value <= 0xFFFF &&
		encode_cb(data, sizeof(data), arg);
}

static bool encode_i16lsb(const int value,
	const pes_encode_callback encode_cb, void * const arg)
{
	return -0x8000 <= value && value <= 0x7FFF &&
		encode_u16lsb(value & 0xFFFF, encode_cb, arg);
}

static bool encode_i32lsb(const int value,
	const pes_encode_callback encode_cb, void * const arg)
{
	const uint8_t data[] = {
		(value >>  0) & 0xFF,
		(value >>  8) & 0xFF,
		(value >> 16) & 0xFF,
		(value >> 24) & 0xFF
	};

	return encode_cb(data, sizeof(data), arg);
}

static bool encode_f32lsb(const float f,
	const pes_encode_callback encode_cb, void * const arg)
{
	uint32_t raw;

	if (sizeof(raw) != sizeof(f))
		return false;
	memcpy(&raw, &f, sizeof(raw));

	const int value = (int)raw;
	return encode_i32lsb(value, encode_cb, arg);
}

static bool encode_string(const char * const s,
	const pes_encode_callback encode_cb, void * const arg)
{
	const size_t size = strlen(s);

	return size <= 0xFFFF &&
	       encode_u16lsb((int)size, encode_cb, arg) &&
	       encode_cb(s, (int)size, arg);
}

static void update_bounds(struct pes_bounds * const bounds,
	const float x, const float y)
{
	if (!bounds->valid) {
		bounds->min_x = x;
		bounds->min_y = y;
		bounds->max_x = x;
		bounds->max_y = y;
		bounds->valid = true;
	} else {
		if (x < bounds->min_x)
			bounds->min_x = x;
		if (y < bounds->min_y)
			bounds->min_y = y;
		if (x > bounds->max_x)
			bounds->max_x = x;
		if (y > bounds->max_y)
			bounds->max_y = y;
	}
}

static bool encode_transform(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg)
{
	struct pes_transform t = encoder->affine_transform;
	const float physical_translation_scale = 1.0f /
		pec_physical_coordinate(1);

	t.matrix[2][0] *= physical_translation_scale;
	t.matrix[2][1] *= physical_translation_scale;

	return encode_f32lsb(t.matrix[0][0], encode_cb, arg) &&
	       encode_f32lsb(t.matrix[0][1], encode_cb, arg) &&
	       encode_f32lsb(t.matrix[1][0], encode_cb, arg) &&
	       encode_f32lsb(t.matrix[1][1], encode_cb, arg) &&
	       encode_f32lsb(t.matrix[2][0], encode_cb, arg) &&
	       encode_f32lsb(t.matrix[2][1], encode_cb, arg);
}

static int is_block(const struct pes_stitch * const stitch_list,
	const int stitch_count, const int stitch_index)
{
	return stitch_list[stitch_index].jump || stitch_index == 0 ||
	       stitch_list[stitch_index - 1].thread_index !=
	       stitch_list[stitch_index - 0].thread_index;
}

static bool encode_cembone(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg)
{
	static const uint8_t footer[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	/*
	 * FIXME: Bounds cannot be stored and must be computed since
	 * the affine transform affects them. Also apply rotational
	 * part for a general matrix multiplication of all coordinates
	 * to compute the bounds. Try WLD01.pes.
	 */
	const int t_x = pec_raw_coordinate(encoder->affine_transform.matrix[2][0]);
	const int t_y = pec_raw_coordinate(encoder->affine_transform.matrix[2][1]);

	const int min_x = pec_raw_coordinate(encoder->bounds.min_x) + t_x;
	const int min_y = pec_raw_coordinate(encoder->bounds.min_y) + t_y;
	const int max_x = pec_raw_coordinate(encoder->bounds.max_x) + t_x;
	const int max_y = pec_raw_coordinate(encoder->bounds.max_y) + t_y;

	const int width  = !encoder->bounds.valid ? 0 : max_x - min_x;
	const int height = !encoder->bounds.valid ? 0 : max_y - min_y;

	const int block_count = encoder->block_count;

	return encode_string("CEmbOne", encode_cb, arg) &&
	       encode_i16lsb(min_x, encode_cb, arg) &&
	       encode_i16lsb(min_y, encode_cb, arg) &&
	       encode_i16lsb(max_x, encode_cb, arg) &&
	       encode_i16lsb(max_y, encode_cb, arg) &&
	       encode_i16lsb(min_x, encode_cb, arg) &&
	       encode_i16lsb(min_y, encode_cb, arg) &&
	       encode_i16lsb(max_x, encode_cb, arg) &&
	       encode_i16lsb(max_y, encode_cb, arg) &&
	       encode_transform(encoder, encode_cb, arg) &&
	       encode_u16lsb(1, encode_cb, arg) &&           /* FIXME: Unknown data */
	       encode_i16lsb((int)roundf(encoder->translation.x), encode_cb, arg) &&
	       encode_i16lsb((int)roundf(encoder->translation.y), encode_cb, arg) &&
	       encode_u16lsb(width, encode_cb, arg) &&
	       encode_u16lsb(height, encode_cb, arg) &&
	       encode_cb(footer, sizeof(footer), arg) &&     /* FIXME: Unknown data */
	       encode_u16lsb(block_count, encode_cb, arg) && /* FIXME: Unknown data */
	       encode_u16lsb(0xFFFF, encode_cb, arg) &&      /* FIXME: Unknown data */
	       encode_u16lsb(0x0000, encode_cb, arg);        /* FIXME: Unknown data */
}

static bool encode_block_header(const enum pec_stitch_type stitch_type,
	const int thread_index, const int stitch_count,
	const pes_encode_callback encode_cb, void * const arg)
{
	return encode_u16lsb(stitch_type, encode_cb, arg) &&
	       encode_u16lsb(thread_index + 1, encode_cb, arg) &&
	       encode_u16lsb(stitch_count, encode_cb, arg);
}

static bool encode_stitch(const float x, const float y,
	const pes_encode_callback encode_cb, void * const arg)
{
	return encode_i16lsb(pec_raw_coordinate(x), encode_cb, arg) &&
	       encode_i16lsb(pec_raw_coordinate(y), encode_cb, arg);
}

static int block_stitch_count(const struct pes_stitch * const stitch_list,
	const int stitch_count, const int stitch_index)
{
	int count = 0;

	while (stitch_index + count < stitch_count && (count == 0 ||
		!is_block(stitch_list, stitch_count, stitch_index + count)))
		count++;

	return count;
}

static bool encode_jump_stitch(
	const struct pes_stitch a, const struct pes_stitch b,
	const pes_encode_callback encode_cb, void * const arg)
{
	return encode_block_header(PEC_STITCH_JUMP,
		       b.thread_index, 2, encode_cb, arg) &&
	       encode_stitch(a.x, a.y, encode_cb, arg) &&
	       encode_stitch(b.x, b.y, encode_cb, arg);
}

static bool encode_stitch_list(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg)
{
	for (int i = 0; i < encoder->stitch_count; i++) {
		const struct pes_stitch * const stitch = &encoder->stitch_list[i];

		if (i == 0) {
			if (!encode_block_header(PEC_STITCH_NORMAL,
				stitch->thread_index,
				block_stitch_count(encoder->stitch_list,
					encoder->stitch_count, i),
				encode_cb, arg))
				return false;
		} else if (stitch->jump || stitch->thread_index !=
			encoder->stitch_list[i].thread_index) {
			/*
			 * A stitch jump can either be explicitly given or
			 * implicit on a thread index change.
			 */
			if (!encode_u16lsb(0x8003, encode_cb, arg)) /* FIXME: Unknown data */
				return false;

			if (!encode_jump_stitch(encoder->stitch_list[i - 1],
				*stitch, encode_cb, arg))
				return false;

			if (!encode_u16lsb(0x8003, encode_cb, arg)) /* FIXME: Unknown data */
				return false;

			const int stitch_count = block_stitch_count(
				encoder->stitch_list, encoder->stitch_count, i);

			if (!encode_block_header(PEC_STITCH_NORMAL,
				stitch->thread_index, stitch_count,
				encode_cb, arg))
				return false;
		}

		if (!encode_stitch(stitch->x, stitch->y, encode_cb, arg))
			return false;
	}

	return true;
}

static bool encode_thread_list14(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg)
{
	int change_count = 0;

	for (int i = 0; i < encoder->stitch_count; i++)
		if (i == 0 || encoder->stitch_list[i - 1].thread_index !=
			encoder->stitch_list[i].thread_index)
			change_count++;

	if (!encode_u16lsb(change_count, encode_cb, arg))
		return false;

	for (int i = 0, block_index = 0; i < encoder->stitch_count; i++) {
		if (i == 0 || encoder->stitch_list[i - 1].thread_index !=
			encoder->stitch_list[i].thread_index) {
			if (!encode_u16lsb(block_index, encode_cb, arg))
				return false;

			const int thread_index = encoder->stitch_list[i].thread_index;
			const struct pec_thread thread = encoder->thread_list[thread_index];
			const int palette_index = pec_palette_index_by_rgb(thread.rgb);

			if (!encode_u16lsb(palette_index, encode_cb, arg))
				return false;
		}

		/* Jump stitches are encoded as two blocks. */
		if (is_block(encoder->stitch_list, encoder->stitch_count, i))
			block_index += (i == 0 ? 1 : 2);
	}

	return encode_u16lsb(0, encode_cb, arg) &&
	       encode_u16lsb(0, encode_cb, arg);
}

static bool encode_csewseg14(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg)
{
	return encode_string("CSewSeg", encode_cb, arg) &&
	       encode_stitch_list(encoder, encode_cb, arg) &&
	       encode_thread_list14(encoder, encode_cb, arg);
}

static bool encode_sections14(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg)
{
	return encode_cembone(encoder, encode_cb, arg) &&
	       encode_csewseg14(encoder, encode_cb, arg);
}

static bool encode_pec(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg)
{
	return pec_encode(encoder->pec_encoder, encode_cb, arg);
}

static bool encode_pec_offset1(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg)
{
	int pec_offset = 22; /* #PES0001 header size. */

	return encode_sections14(encoder, encoded_size, &pec_offset) &&
	       encode_i32lsb(pec_offset, encode_cb, arg);
}

static bool append_stitch(struct pes_encoder * const encoder,
	const int thread_index, const float x, const float y, const bool jump)
{
	if (thread_index < 0 || encoder->thread_count <= thread_index)
		return false;

	if (encoder->stitch_capacity <= encoder->stitch_count) {
		const int capacity = encoder->stitch_capacity +
			(encoder->stitch_capacity == 0    ?   100 :
			 10000 < encoder->stitch_capacity ? 10000 :
			 encoder->stitch_capacity);

		if (capacity < INT_MAX/2) {
			struct pes_stitch * const stitch_list =
				realloc(encoder->stitch_list,
					(size_t)capacity * sizeof(*stitch_list));

			if (stitch_list != NULL) {
				encoder->stitch_list = stitch_list;
				encoder->stitch_capacity = capacity;
			}
		}
	}

	if (encoder->stitch_capacity <= encoder->stitch_count)
		return false;

	/* FIXME: Is there a 1000 stitch limit per block? Many PES files indicate that. */

	const bool thread_change = 0 < encoder->stitch_count && thread_index !=
		encoder->stitch_list[encoder->stitch_count - 1].thread_index;

	if (encoder->stitch_count == 0 || thread_change) {
		const int palette_index = pec_palette_index_by_rgb(
			encoder->thread_list[thread_index].rgb);
		if (!pec_append_thread(encoder->pec_encoder, palette_index))
			return false;
	}

	if (!(thread_change ? pec_append_jump_stitch :
		       jump ? pec_append_trim_stitch :
		              pec_append_stitch)
		(encoder->pec_encoder, x, y))
		return false;

	encoder->stitch_list[encoder->stitch_count] =
		(struct pes_stitch){
			.thread_index = thread_index,
			.x = x,
			.y = y,
			.jump = jump
		};
	update_bounds(&encoder->bounds, x, y);
	encoder->stitch_count++;

	/* Jump stitches are encoded as two blocks. */
	if (is_block(encoder->stitch_list, encoder->stitch_count,
		encoder->stitch_count - 1))
		encoder->block_count += (encoder->stitch_count == 1 ? 1 : 2);

	return true;
}

static size_t pes_encode_size(
	bool (*pes_encode)(const struct pes_encoder * const encoder,
		const pes_encode_callback encode_cb, void * const arg),
	const struct pes_encoder * const encoder)
{
	int size = 0;

	if (!pes_encode(encoder, encoded_size, &size))
		return 0;

	return (size_t)size;
}

struct pes_encoder *pes_encoder_init()
{
	struct pes_encoder * const encoder = calloc(1, sizeof(*encoder));

	if (encoder != NULL) {
		encoder->affine_transform.matrix[0][0] = 1.0f;
		encoder->affine_transform.matrix[1][1] = 1.0f;

		encoder->pec_encoder = pec_encoder_init();
		if (encoder->pec_encoder == NULL) {
			pes_encoder_free(encoder);
			return NULL;
		}
	}

	return encoder;
}

void pes_encoder_free(struct pes_encoder * const encoder)
{
	if (encoder != NULL) {
		pec_encoder_free(encoder->pec_encoder);
		free(encoder->stitch_list);
		free(encoder);
	}
}

bool pes_append_thread(struct pes_encoder * const encoder,
	const struct pec_thread thread)
{
	if (PES_MAX_THREADS <= encoder->thread_count)
		return false;

	encoder->thread_list[encoder->thread_count++] = thread;

	return true;
}

bool pes_append_stitch(struct pes_encoder * const encoder,
	const int thread_index, const float x, const float y)
{
	return append_stitch(encoder, thread_index, x, y, false);
}

bool pes_append_jump_stitch(struct pes_encoder * const encoder,
	const int thread_index, const float x, const float y)
{
	return append_stitch(encoder, thread_index, x, y, true);
}

void pes_encode_transform(struct pes_encoder * const encoder,
	const struct pes_transform affine_transform)
{
	memcpy(&encoder->affine_transform, &affine_transform,
		sizeof(encoder->affine_transform));
}

bool pes_encode1(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg)
{
	return encode_cb("#PES0001", 8, arg) &&
	       encode_pec_offset1(encoder, encode_cb, arg) &&
	       encode_u16lsb(0x0000, encode_cb, arg) && /* FIXME: Unknown data */
	       encode_u16lsb(0x0001, encode_cb, arg) && /* FIXME: Unknown data */
	       encode_u16lsb(0x0001, encode_cb, arg) && /* FIXME: Unknown data */
	       encode_u16lsb(0xFFFF, encode_cb, arg) && /* FIXME: Unknown data */
	       encode_u16lsb(0x0000, encode_cb, arg) && /* FIXME: Unknown data */
	       encode_sections14(encoder, encode_cb, arg) &&
	       encode_pec(encoder, encode_cb, arg);
}

bool pes_encode4(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg)
{
	return encode_cb("#PES0040", 8, arg) && /* FIXME: Proper header */
	       false;
}

bool pes_encode5(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg)
{
	return encode_cb("#PES0050", 8, arg) && /* FIXME: Proper header */
	       false;
}

bool pes_encode6(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg)
{
	return encode_cb("#PES0060", 8, arg) && /* FIXME: Proper header */
	       false;
}

size_t pes_encode1_size(const struct pes_encoder * const encoder)
{
	return pes_encode_size(pes_encode1, encoder);
}

size_t pes_encode4_size(const struct pes_encoder * const encoder)
{
	return pes_encode_size(pes_encode4, encoder);
}

size_t pes_encode5_size(const struct pes_encoder * const encoder)
{
	return pes_encode_size(pes_encode5, encoder);
}

size_t pes_encode6_size(const struct pes_encoder * const encoder)
{
	return pes_encode_size(pes_encode6, encoder);
}
