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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pec-encoder.h"

#define PEC_THUMBNAIL_WIDTH  48
#define PEC_THUMBNAIL_HEIGHT 38

struct pec_stitch {
	float x;
	float y;
	enum pec_stitch_type type;
};

struct pec_thumbnail {
	uint8_t image[PEC_THUMBNAIL_HEIGHT][PEC_THUMBNAIL_WIDTH / 8];
};

struct pec_bounds {
	float min_x;
	float min_y;
	float max_x;
	float max_y;
	bool valid;
};

struct pec_encoder {
	struct pec_bounds bounds;

	int stitch_count;
	int stitch_capacity;
	struct pec_stitch *stitch_list;

	int thread_count;
	int palette[PEC_MAX_THREADS];
};

static bool encoded_size(const void * const data, const size_t size,
	void * const arg)
{
	int * const s = arg;

	*s += size;

	return *s >= 0;
}

static bool encode_u8(const int value,
	const pec_encode_callback encode_cb, void * const arg)
{
	const uint8_t data[] = { value & 0xFF };

	return 0 <= value && value <= 0xFF &&
		encode_cb(data, sizeof(data), arg);
}

static bool encode_u16lsb(const int value,
	const pec_encode_callback encode_cb, void * const arg)
{
	const uint8_t data[] = {
		(value >>  0) & 0xFF,
		(value >>  8) & 0xFF
	};

	return 0 <= value && value <= 0xFFFF &&
		encode_cb(data, sizeof(data), arg);
}

static bool encode_threads(const struct pec_encoder * const encoder,
	const pec_encode_callback encode_cb, void * const arg)
{
	if (encoder->thread_count < 1 || PEC_MAX_THREADS < encoder->thread_count)
		return false;

	if (!encode_cb("            ", 12, arg)) /* FIXME: Unknown data */
		return false;

	if (!encode_u8(encoder->thread_count - 1, encode_cb, arg))
		return false;

	for (int i = 0; i < encoder->thread_count; i++)
		if (!encode_u8(encoder->palette[i], encode_cb, arg))
			return false;

	for (int i = encoder->thread_count; i < 463; i++)
		if (!encode_u8(0x20, encode_cb, arg))
			return false;

	return true;
}

static bool encode_label(const struct pec_encoder * const encoder,
	const pec_encode_callback encode_cb, void * const arg)
{
	const char label[32 + 1] = "LA:                \r            ";

	return encode_cb(label, 32, arg) &&
	       encode_u16lsb(0x00FF, encode_cb, arg); /* FIXME: Unknown data */
}

static bool encode_thumbnail_size(const struct pec_encoder * const encoder,
	const pec_encode_callback encode_cb, void * const arg)
{
	return encode_u8(PEC_THUMBNAIL_WIDTH / 8, encode_cb, arg) &&
	       encode_u8(PEC_THUMBNAIL_HEIGHT, encode_cb, arg);
}

static bool encode_size(const struct pec_encoder * const encoder,
	const pec_encode_callback encode_cb, void * const arg)
{
	const int width  = !encoder->bounds.valid ? 0 :
		pec_raw_coordinate(encoder->bounds.max_x) -
		pec_raw_coordinate(encoder->bounds.min_x);
	const int height = !encoder->bounds.valid ? 0 :
		pec_raw_coordinate(encoder->bounds.max_y) -
		pec_raw_coordinate(encoder->bounds.min_y);

	return encode_u16lsb( width, encode_cb, arg) &&
	       encode_u16lsb(height, encode_cb, arg) &&
	       encode_u16lsb(0x01E0, encode_cb, arg) && /* FIXME: Unknown data */
	       encode_u16lsb(0x01B0, encode_cb, arg) && /* FIXME: Unknown data */
	       encode_u16lsb(0x0000, encode_cb, arg) && /* FIXME: Unknown data */
	       encode_u16lsb(0x0000, encode_cb, arg);   /* FIXME: Unknown data */
}

static bool encode_stitch(const struct pec_encoder * const encoder,
	const enum pec_stitch_type type, const int d,
	const pec_encode_callback encode_cb, void * const arg)
{
	if (d < -0x800 || 0x7FF < d)
		return false;

	if (type == PEC_STITCH_NORMAL && -0x40 <= d && d <= 0x3F) {
		if (!encode_u8(d & 0x7F, encode_cb, arg))
			return false;
	} else if (!encode_u8(((d >> 8) & 0xF) | 0x80 |
			(type == PEC_STITCH_TRIM ? 0x20 :
			 type == PEC_STITCH_JUMP ? 0x10 : 0x00),
			encode_cb, arg) ||
		   !encode_u8(d & 0xFF, encode_cb, arg))
			return false;

	return true;
}

static bool encode_stitch_list(const struct pec_encoder * const encoder,
	const pec_encode_callback encode_cb, void * const arg)
{
	int x = pec_raw_coordinate(encoder->bounds.min_x);
	int y = pec_raw_coordinate(encoder->bounds.min_y);

	for (int i = 0, stop = 2; i < encoder->stitch_count; i++) {
		const enum pec_stitch_type type = encoder->stitch_list[i].type;
		const int nx = pec_raw_coordinate(encoder->stitch_list[i].x);
		const int ny = pec_raw_coordinate(encoder->stitch_list[i].y);

		/*
		 * FIXME: Move first (x,y) slightly if identical to (0,0)
		 * since the embroidery machine apparently ignores them.
		 * This needs a corresponding fix in the transcoder.
		 */

		if (type == PEC_STITCH_STOP) {
			if (!encode_u16lsb(0xB0FE, encode_cb, arg) ||
			    !encode_u8(stop, encode_cb, arg))
				return false;
			stop = 3 - stop; /* FIXME: Why alternate between 2 and 1? */
			continue;
		}

		if (!encode_stitch(encoder, type, nx - x, encode_cb, arg))
			return false;
		if (!encode_stitch(encoder, type, ny - y, encode_cb, arg))
			return false;

		x = nx;
		y = ny;
	}

	/* End of stitch list. */
	if (!encode_u8(0xFF, encode_cb, arg))
		return false;

	return true;
}

static bool encode_thumbnail_offset(const struct pec_encoder * const encoder,
	const pec_encode_callback encode_cb, void * const arg)
{
	int size = 20;

	if (!encode_stitch_list(encoder, encoded_size, &size))
		return 0;

	return size <= 0xFFFF &&
	       encode_u16lsb(0x0000, encode_cb, arg) &&
	       encode_u16lsb(  size, encode_cb, arg) &&
	       encode_u16lsb(0x3100, encode_cb, arg) &&
	       encode_u16lsb(0xF0FF, encode_cb, arg);
}

static void thumbnail_plot(struct pec_thumbnail * const thumbnail,
	const int c, const int r)
{
	if (0 <= c && c < PEC_THUMBNAIL_WIDTH &&
	    0 <= r && r < PEC_THUMBNAIL_HEIGHT)
		thumbnail->image[r][c / 8] |= 1 << (c %8);
}

static void thumbnail_framed_plot(struct pec_thumbnail * const thumbnail,
	const float x, const float y, const struct pec_bounds * const bounds)
{
	const int margin = 5;
	const float w = bounds->max_x - bounds->min_x;
	const float h = bounds->max_y - bounds->min_y;
	const float cx = 0.5f * (bounds->min_x + bounds->max_x);
	const float cy = 0.5f * (bounds->min_y + bounds->max_y);
	const float tx = 0.5f * (PEC_THUMBNAIL_WIDTH  - 2 * margin);
	const float ty = 0.5f * (PEC_THUMBNAIL_HEIGHT - 2 * margin);

	if (0.0f < w || 0.0f < h) {
		const float sw = 2.0f * tx / w;
		const float sh = 2.0f * ty / h;
		const float s = (sw < sh ? sw : sh);
		const int c = margin + (int)roundf(tx + (x - cx) * s);
		const int r = margin + (int)roundf(ty + (y - cy) * s);

		thumbnail_plot(thumbnail, c, r);
	}
}

static void thumbnail_framed_line(struct pec_thumbnail * const thumbnail,
	const struct pec_stitch * const a, const struct pec_stitch * const b,
	const struct pec_bounds * const bounds)
{
	if (a->type != PEC_STITCH_NORMAL ||
	    b->type != PEC_STITCH_NORMAL)
		return;

	for (int i = 0; i <= 100; i++) {
		const float t = i / 100.0f;

		thumbnail_framed_plot(thumbnail,
			(1.0f - t) * a->x + t * b->x,
			(1.0f - t) * a->y + t * b->y, bounds);
	}
}

static void thumbnail_frame(struct pec_thumbnail * const thumbnail)
{
	for (int c = 4; c < PEC_THUMBNAIL_WIDTH - 4; c++) {
		thumbnail_plot(thumbnail, c, 1);
		thumbnail_plot(thumbnail, c, PEC_THUMBNAIL_HEIGHT - 2);
	}

	for (int r = 4; r < PEC_THUMBNAIL_HEIGHT - 4; r++) {
		thumbnail_plot(thumbnail, 1, r);
		thumbnail_plot(thumbnail, PEC_THUMBNAIL_WIDTH - 2, r);
	}

	thumbnail_plot(thumbnail, 3, 2);
	thumbnail_plot(thumbnail, 2, 3);
	thumbnail_plot(thumbnail, PEC_THUMBNAIL_WIDTH - 4, 2);
	thumbnail_plot(thumbnail, PEC_THUMBNAIL_WIDTH - 3, 3);
	thumbnail_plot(thumbnail, 2, PEC_THUMBNAIL_HEIGHT - 4);
	thumbnail_plot(thumbnail, 3, PEC_THUMBNAIL_HEIGHT - 3);
	thumbnail_plot(thumbnail, PEC_THUMBNAIL_WIDTH - 3, PEC_THUMBNAIL_HEIGHT - 4);
	thumbnail_plot(thumbnail, PEC_THUMBNAIL_WIDTH - 4, PEC_THUMBNAIL_HEIGHT - 3);
}

static bool encode_thumbnail(const struct pec_encoder * const encoder,
	struct pec_thumbnail * const thumbnail,
	const pec_encode_callback encode_cb, void * const arg)
{
	thumbnail_frame(thumbnail);

	for (int r = 0; r < PEC_THUMBNAIL_HEIGHT; r++)
	for (int c = 0; c < PEC_THUMBNAIL_WIDTH / 8; c++)
		if (!encode_u8(thumbnail->image[r][c], encode_cb, arg))
			return false;

	return true;
}

static bool encode_thumbnail_list(const struct pec_encoder * const encoder,
	const pec_encode_callback encode_cb, void * const arg)
{
	struct pec_thumbnail thumbnail = { 0 };

	for (int k = 1; k < encoder->stitch_count; k++)
		thumbnail_framed_line(&thumbnail, &encoder->stitch_list[k - 1],
			&encoder->stitch_list[k], &encoder->bounds);

	if (!encode_thumbnail(encoder, &thumbnail, encode_cb, arg))
		return false;

	for (int i = 0, k = 1; i < encoder->thread_count; i++, k++) {
		memset(&thumbnail, 0, sizeof(thumbnail));

		for (; k < encoder->stitch_count &&
			encoder->stitch_list[k].type != PEC_STITCH_STOP; k++)
			thumbnail_framed_line(&thumbnail, &encoder->stitch_list[k - 1],
				&encoder->stitch_list[k], &encoder->bounds);

		if (!encode_thumbnail(encoder, &thumbnail, encode_cb, arg))
			return false;
	}

	return true;
}

static void update_bounds(struct pec_encoder * const encoder,
	const float x, const float y)
{
	if (!encoder->bounds.valid) {
		encoder->bounds.min_x = x;
		encoder->bounds.min_y = y;
		encoder->bounds.max_x = x;
		encoder->bounds.max_y = y;
		encoder->bounds.valid = true;
	} else {
		if (x < encoder->bounds.min_x)
			encoder->bounds.min_x = x;
		if (y < encoder->bounds.min_y)
			encoder->bounds.min_y = y;
		if (x > encoder->bounds.max_x)
			encoder->bounds.max_x = x;
		if (y > encoder->bounds.max_y)
			encoder->bounds.max_y = y;
	}
}

static bool append_stitch(struct pec_encoder * const encoder,
	const enum pec_stitch_type stitch_type, const float x, const float y)
{
	if (encoder->thread_count == 0)
		return false;

	if (encoder->stitch_capacity <= encoder->stitch_count) {
		const int capacity = encoder->stitch_capacity +
			(encoder->stitch_capacity == 0    ?   100 :
			 10000 < encoder->stitch_capacity ? 10000 :
			 encoder->stitch_capacity);

		if (capacity < INT_MAX/2) {
			struct pec_stitch * const stitch_list =
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

	encoder->stitch_list[encoder->stitch_count] =
		(struct pec_stitch){ .x = x, .y = y, .type = stitch_type };
	update_bounds(encoder, x, y);
	encoder->stitch_count++;

	return true;
}

struct pec_encoder *pec_encoder_init()
{
	return calloc(1, sizeof(struct pec_encoder));
}

void pec_encoder_free(struct pec_encoder * const encoder)
{
	if (encoder != NULL) {
		free(encoder->stitch_list);
		free(encoder);
	}
}

bool pec_encode(const struct pec_encoder * const encoder,
	const pec_encode_callback encode_cb, void * const arg)
{
	return encode_label(encoder, encode_cb, arg) &&
	       encode_thumbnail_size(encoder, encode_cb, arg) &&
	       encode_threads(encoder, encode_cb, arg) &&
	       encode_thumbnail_offset(encoder, encode_cb, arg) &&
	       encode_size(encoder, encode_cb, arg) &&
	       encode_stitch_list(encoder, encode_cb, arg) &&
	       encode_thumbnail_list(encoder, encode_cb, arg);
}

size_t pec_encoded_size(const struct pec_encoder * const encoder)
{
	int size = 0;

	if (!pec_encode(encoder, encoded_size, &size))
		return 0;

	return (size_t)size;
}

bool pec_append_stitch(struct pec_encoder * const encoder,
	const float x, const float y)
{
	return append_stitch(encoder, PEC_STITCH_NORMAL, x, y);
}

bool pec_append_jump_stitch(struct pec_encoder * const encoder,
	const float x, const float y)
{
	return append_stitch(encoder, PEC_STITCH_JUMP, x, y);
}

bool pec_append_trim_stitch(struct pec_encoder * const encoder,
	const float x, const float y)
{
	return append_stitch(encoder, PEC_STITCH_TRIM, x, y);
}

bool pec_append_thread(struct pec_encoder * const encoder,
	const int palette_index)
{
	if (PEC_MAX_THREADS <= encoder->thread_count)
		return false;

	encoder->palette[encoder->thread_count++] = palette_index;

	return encoder->stitch_count == 0 ? true :
		append_stitch(encoder, PEC_STITCH_STOP, 0.0f, 0.0f);
}

int pec_raw_coordinate(const float c)
{
	return (int)roundf(10.0f * c);
}
