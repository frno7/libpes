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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pec-decoder.h"
#include "pes-decoder.h"
#include "pes.h"

struct pes_string {
	int length;
	char s[256];
};

struct pes_thread {
	struct pec_thread thread;

	/* These pes_strings are needed for storage of thread strings. */
	struct pes_string id, code, name, type;
};

struct pes_change {
	int block_index;  /* Block index of thread change. */
	int thread_index; /* Thread index to change for. */
};

struct pes_decoder {
	int size;
	uint8_t *data;

	char version[5];

	int hoop_width;
	int hoop_height;

	int pec_offset;
	int cembone_offset;
	int csewseg_offset;

	struct pes_string name;

	int thread_count;
	struct pes_thread *thread_list;

	int change_count;
	struct pes_change *change_list;

	struct pec_decoder *pec;
};

typedef bool (*pes_thread_callback)(const int change_index,
	const int block_index, const int index, void * const arg);

static bool decode_u8(const struct pes_decoder * const decoder,
	const int offset, int * const value)
{
	if (offset < 0 || decoder->size < offset + 1)
		return false;

	*value = decoder->data[offset];

	return true;
}

static bool decode_u16lsb(const struct pes_decoder * const decoder,
	const int offset, int * const value)
{
	if (offset < 0 || decoder->size < offset + 2)
		return false;

	*value = ((int)decoder->data[offset + 0] << 0) |
	         ((int)decoder->data[offset + 1] << 8);

	return true;
}

static bool decode_i16lsb(const struct pes_decoder * const decoder,
	const int offset, int * const value)
{
	if (offset < 0 || decoder->size < offset + 2)
		return false;

	*value = (int16_t)((decoder->data[offset + 0] << 0) |
	                   (decoder->data[offset + 1] << 8));

	return true;
}

static bool decode_i32lsb(const struct pes_decoder * const decoder,
	const int offset, int * const value)
{
	if (offset < 0 || decoder->size < offset + 4)
		return false;

	*value = ((int)decoder->data[offset + 0] <<  0) |
	         ((int)decoder->data[offset + 1] <<  8) |
	         ((int)decoder->data[offset + 2] << 16) |
	         ((int)decoder->data[offset + 3] << 24);

	return true;
}

static bool decode_f32lsb(const struct pes_decoder * const decoder,
	const int offset, float * const value)
{
	uint32_t raw;
	int i;

	if (!decode_i32lsb(decoder, offset, &i))
		return false;
	raw = i;
	if (sizeof(raw) != sizeof(*value))
		return false;
	memcpy(value, &raw, sizeof(*value));

	return true;
}

static bool decode_string(const struct pes_decoder * const decoder,
	const int offset, struct pes_string * const small_string)
{
	if (!decode_u8(decoder, offset, &small_string->length))
		return false;
	if (decoder->size < offset + 1 + small_string->length)
		return false;

	memcpy(&small_string->s[0], &decoder->data[offset + 1],
		small_string->length);
	small_string->s[small_string->length] = '\0';

	return true;
}

static bool decode_hoop_size(const struct pes_decoder * const decoder,
	const int offset, int * const hoop_width, int * const hoop_height)
{
	return decode_u16lsb(decoder, offset + 0, hoop_width) &&
	       decode_u16lsb(decoder, offset + 2, hoop_height);
}

static bool string_equal(const struct pes_decoder * const decoder,
	const int offset, const char * const s)
{
	int length;

	return decode_u16lsb(decoder, offset, &length) &&
	       (size_t)length == strlen(s) &&
	       offset + 2 + length <= decoder->size &&
	       strncmp((const char *)&decoder->data[offset + 2], s, length) == 0;
}

static bool thread_foreach(const struct pes_decoder * const decoder,
	int offset, const pes_thread_callback thread_cb, void * const arg)
{
	int thread_count;

	if (!decode_u16lsb(decoder, offset, &thread_count))
		return false;
	offset += 2;

	for (int i = 0; i < thread_count; i++) {
		int block_index, index;

		if (!decode_u16lsb(decoder, offset + 0, &block_index) ||
		    !decode_u16lsb(decoder, offset + 2, &index))
			return false;
		offset += 4;

		if (!thread_cb(i, block_index, index, arg))
			return false;
	}

	return offset <= decoder->pec_offset;
}

static bool stitch_foreach(const struct pes_decoder * const decoder,
	const pes_block_callback block_cb, const pes_stitch_callback stitch_cb,
	const pes_thread_callback thread_cb, void * const arg)
{
	struct pec_thread thread = pec_undefined_thread();
	int offset = decoder->csewseg_offset + 9;
	int change_index = 0;

	for (int block_index = 0; offset < decoder->pec_offset; block_index++) {
		int stitch_type, block_id, stitch_count;

		if (!decode_u16lsb(decoder, offset + 0, &stitch_type) ||
		    !decode_u16lsb(decoder, offset + 2, &block_id) ||
		    !decode_u16lsb(decoder, offset + 4, &stitch_count))
			return false;
		offset += 6;

		/* Update thread by block index if necessary. */
		if (change_index < decoder->change_count && block_index ==
			decoder->change_list[change_index].block_index)
			thread = decoder->thread_list[decoder->
				change_list[change_index++].thread_index].thread;

		if (block_cb != NULL)
			if (!block_cb(thread, stitch_count, stitch_type, arg))
				return false;

		for (int i = 0; i < stitch_count; i++) {
			int x, y;

			if (!decode_i16lsb(decoder, offset + 0, &x) ||
			    !decode_i16lsb(decoder, offset + 2, &y))
				return false;
			offset += 4;

			if (stitch_cb != NULL) {
				if (!stitch_cb(i,
					pec_physical_coordinate(x),
				       	pec_physical_coordinate(y), arg))
					return false;
			}
		}

		int code;
		if (!decode_u16lsb(decoder, offset, &code))
			return false;
		if (code != 0x8003)
			break;
		offset += 2;
	}

	return thread_cb == NULL ||
		thread_foreach(decoder, offset, thread_cb, arg);
}

static bool stitch_count(const int stitch_index,
	const float x, const float y, void * const arg)
{
	int * const counter = (int *)arg;

	(*counter)++;

	return true;
}

static bool change_count_cb(const int change_index, const int block_index,
	const int palette_index, void * const arg)
{
	int * const change_count = arg;

	(*change_count)++;

	return true;
}

static bool change_thread_cb(const int change_index, const int block_index,
	const int thread_index, void * const arg)
{
	struct pes_decoder * const decoder = arg;
	struct pes_change * const change = &decoder->change_list[change_index];

	change->block_index = block_index;
	change->thread_index = thread_index;

	return change->thread_index < decoder->thread_count;
}

static bool palette_thread_cb(const int change_index, const int block_index,
	const int palette_index, void * const arg)
{
	struct pes_decoder * const decoder = arg;
	struct pes_thread * const thread = &decoder->thread_list[change_index];

	thread->thread = pec_palette_thread(palette_index);
	thread->thread.index = change_index;

	return change_thread_cb(change_index, block_index, change_index, arg);
}

static bool init_palette(struct pes_decoder * const decoder)
{
	int change_count = 0;

	if (!stitch_foreach(decoder, NULL, NULL, change_count_cb, &change_count))
		return false;

	decoder->thread_list =
		calloc(1, (size_t)change_count * sizeof(*decoder->thread_list));
	if (decoder->thread_list == NULL)
		return false;
	decoder->thread_count = change_count;

	decoder->change_list =
		calloc(1, (size_t)change_count * sizeof(*decoder->change_list));
	if (decoder->change_list == NULL)
		return false;
	decoder->change_count = change_count;

	if (!stitch_foreach(decoder, NULL, NULL, palette_thread_cb, decoder))
		return false;

	return true;
}

static bool init_change(struct pes_decoder * const decoder)
{
	int change_count = 0;

	if (!stitch_foreach(decoder, NULL, NULL, change_count_cb, &change_count))
		return false;

	decoder->change_list =
		calloc(1, (size_t)change_count * sizeof(*decoder->change_list));
	if (decoder->change_list == NULL)
		return false;

	if (!stitch_foreach(decoder, NULL, NULL, change_thread_cb, decoder))
		return false;

	decoder->change_count = change_count;

	return true;
}

static bool init_threads(struct pes_decoder * const decoder, int * const offset)
{
	if (!decode_u16lsb(decoder, *offset, &decoder->thread_count))
		return false;
	*offset += 2;

	decoder->thread_list = calloc(1, (size_t)decoder->thread_count *
		sizeof(*decoder->thread_list));
	if (decoder->thread_list == NULL)
		return false;

	for (int i = 0; i < decoder->thread_count; i++) {
		static const char * const thread_type[] =
			{ "A", "B", "C", "D", "E", "F" };
		struct pes_thread * const thread = &decoder->thread_list[i];

		if (!decode_string(decoder, *offset, &thread->code))
			return false;
		*offset += 1 + thread->code.length;

		if (!decode_u8(decoder, *offset + 0, &thread->thread.rgb.r) ||
		    !decode_u8(decoder, *offset + 1, &thread->thread.rgb.g) ||
		    !decode_u8(decoder, *offset + 2, &thread->thread.rgb.b))
			return false;
		*offset += 3;

		*offset += 1; /* FIXME: Unknown data */

		int type;
		if (!decode_u8(decoder, *offset, &type))
			return false;
		*offset += 1;

		*offset += 3; /* FIXME: Unknown data */

		if (!decode_string(decoder, *offset, &thread->id))
			return false;
		*offset += 1 + thread->id.length;

		if (!decode_string(decoder, *offset, &thread->name))
			return false;
		*offset += 1 + thread->name.length;

		*offset += 1; /* FIXME: Unknown data */

		thread->thread.index = i;
		thread->thread.id = thread->id.s;
		thread->thread.code = thread->code.s;
		thread->thread.name = thread->name.s;
		thread->thread.type = (0xA <= type && type <= 0xF ?
			thread_type[type - 0xA] : "-");
	}

	return true;
}

static bool init_version1(struct pes_decoder * const decoder)
{
	int offset = 12;

	offset += 10; /* FIXME: Unknown data */

	decoder->cembone_offset = offset;

	return true;
}

static bool init_version4(struct pes_decoder * const decoder)
{
	int offset = 12;

	offset += 4; /* FIXME: Unknown data */

	if (!decode_string(decoder, offset, &decoder->name))
		return false;
	offset += 1 + decoder->name.length;

	offset += 6; /* FIXME: Unknown data */

	if (!decode_hoop_size(decoder, offset,
		&decoder->hoop_width, &decoder->hoop_height))
		return false;
	offset += 4;

	offset += 28; /* FIXME: Unknown data */

	decoder->cembone_offset = offset;

	return true;
}

static bool init_version5(struct pes_decoder * const decoder)
{
	int offset = 12;

	offset += 4; /* FIXME: Unknown data */

	if (!decode_string(decoder, offset, &decoder->name))
		return false;
	offset += 1 + decoder->name.length;

	offset += 6; /* FIXME: Unknown data */

	if (!decode_hoop_size(decoder, offset,
		&decoder->hoop_width, &decoder->hoop_height))
		return false;
	offset += 4;

	offset += 49; /* FIXME: Unknown data */

	if (!init_threads(decoder, &offset))
		return false;

	offset += 6; /* FIXME: Unknown data */

	decoder->cembone_offset = offset;

	return true;
}

static bool init_version6(struct pes_decoder * const decoder)
{
	int offset = 12;

	offset += 4; /* FIXME: Unknown data */

	if (!decode_string(decoder, offset, &decoder->name))
		return false;
	offset += 1 + decoder->name.length;

	offset += 8; /* FIXME: Unknown data */

	if (!decode_hoop_size(decoder, offset,
		&decoder->hoop_width, &decoder->hoop_height))
		return false;
	offset += 4;

	offset += 59; /* FIXME: Unknown data */

	if (!init_threads(decoder, &offset))
		return false;

	offset += 6; /* FIXME: Unknown data */

	decoder->cembone_offset = offset;

	return true;
}

static bool init_pec(struct pes_decoder * const decoder)
{
	if (!decode_i32lsb(decoder, 8, &decoder->pec_offset) ||
	    decoder->size < decoder->pec_offset)
		return false;

	decoder->pec = pec_decoder_init(&decoder->data[decoder->pec_offset],
		decoder->size - decoder->pec_offset);

	return decoder->pec != NULL;
}

struct pes_decoder *pes_decoder_init(const void * const data, const size_t size)
{
	if (INT_MAX/2 < size)
		return NULL;

	struct pes_decoder * const decoder = calloc(1, sizeof(*decoder) + size);

	if (decoder != NULL) {
		decoder->data = (uint8_t *)&decoder[1];
		decoder->size = (int)size;
		memcpy(decoder->data, data, size);

		if (!init_pec(decoder)) {
			pes_decoder_free(decoder);
			return NULL;
		}

		if (strncmp(data, "#PES0001", 8) == 0 ? !init_version1(decoder) :
		    strncmp(data, "#PES0040", 8) == 0 ? !init_version4(decoder) :
		    strncmp(data, "#PES0050", 8) == 0 ? !init_version5(decoder) :
		    strncmp(data, "#PES0060", 8) == 0 ? !init_version6(decoder) :
		    true) {
			pes_decoder_free(decoder);
			return NULL;
		}

		memcpy(&decoder->version[0], &decoder->data[4], 4);

		if (!string_equal(decoder, decoder->cembone_offset, "CEmbOne")) {
			pes_decoder_free(decoder);
			return NULL;
		}

		decoder->csewseg_offset = decoder->cembone_offset + 73;
		if (!string_equal(decoder, decoder->csewseg_offset, "CSewSeg")) {
			pes_decoder_free(decoder);
			return false;
		}

		if (decoder->thread_list == NULL) {
			if (!init_palette(decoder))
				return false;
		} else if (!init_change(decoder))
			return false;
	}

	return decoder;
}

void pes_decoder_free(struct pes_decoder * const decoder)
{
	if (decoder != NULL) {
		pec_decoder_free(decoder->pec);
		free(decoder->thread_list);
		free(decoder->change_list);
		free(decoder);
	}
}

const char *pes_version(const struct pes_decoder * const decoder)
{
	return decoder->version;
}

const char *pes_name(const struct pes_decoder * const decoder)
{
	return decoder->name.s;
}

int pes_thread_count(const struct pes_decoder * const decoder)
{
	return decoder->thread_count;
}

const struct pec_thread pes_thread(const struct pes_decoder * const decoder,
	const int thread_index)
{
	return 0 <= thread_index && thread_index < decoder->thread_count ?
		decoder->thread_list[thread_index].thread :
		pec_undefined_thread();
}

int pes_stitch_count(const struct pes_decoder * const decoder)
{
	int counter = 0;

	if (!stitch_foreach(decoder, NULL, stitch_count, NULL, &counter))
		return 0;

	return counter;
}

void pes_bounds1(const struct pes_decoder * const decoder,
	float * const min_x, float * const min_y,
	float * const max_x, float * const max_y)
{
	int x_min = 0, y_min = 0, x_max = 0, y_max = 0;

	decode_i16lsb(decoder, decoder->cembone_offset +  9, &x_min);
	decode_i16lsb(decoder, decoder->cembone_offset + 11, &y_min);
	decode_i16lsb(decoder, decoder->cembone_offset + 13, &x_max);
	decode_i16lsb(decoder, decoder->cembone_offset + 15, &y_max);

	if (min_x != NULL)
		*min_x = pec_physical_coordinate(x_min);
	if (min_y != NULL)
		*min_y = pec_physical_coordinate(y_min);
	if (max_x != NULL)
		*max_x = pec_physical_coordinate(x_max);
	if (max_y != NULL)
		*max_y = pec_physical_coordinate(y_max);
}

void pes_bounds2(const struct pes_decoder * const decoder,
	float * const min_x, float * const min_y,
	float * const max_x, float * const max_y)
{
	int x_min = 0, y_min = 0, x_max = 0, y_max = 0;

	decode_i16lsb(decoder, decoder->cembone_offset + 17, &x_min);
	decode_i16lsb(decoder, decoder->cembone_offset + 19, &y_min);
	decode_i16lsb(decoder, decoder->cembone_offset + 21, &x_max);
	decode_i16lsb(decoder, decoder->cembone_offset + 23, &y_max);

	if (min_x != NULL)
		*min_x = pec_physical_coordinate(x_min);
	if (min_x != NULL)
		*min_y = pec_physical_coordinate(y_min);
	if (max_y != NULL)
		*max_x = pec_physical_coordinate(x_max);
	if (max_y != NULL)
		*max_y = pec_physical_coordinate(y_max);
}

struct pes_transform pes_affine_transform(
	const struct pes_decoder * const decoder)
{
	const float physical_translation_scale = pec_physical_coordinate(1);
	struct pes_transform affine_transform = { };

	decode_f32lsb(decoder, decoder->cembone_offset + 25, &affine_transform.matrix[0][0]);
	decode_f32lsb(decoder, decoder->cembone_offset + 29, &affine_transform.matrix[0][1]);
	decode_f32lsb(decoder, decoder->cembone_offset + 33, &affine_transform.matrix[1][0]);
	decode_f32lsb(decoder, decoder->cembone_offset + 37, &affine_transform.matrix[1][1]);
	decode_f32lsb(decoder, decoder->cembone_offset + 41, &affine_transform.matrix[2][0]);
	decode_f32lsb(decoder, decoder->cembone_offset + 45, &affine_transform.matrix[2][1]);

	affine_transform.matrix[2][0] *= physical_translation_scale;
	affine_transform.matrix[2][1] *= physical_translation_scale;

	return affine_transform;
}

float pes_translation_x(const struct pes_decoder * const decoder)
{
	int x;

	return decode_i16lsb(decoder, decoder->cembone_offset + 51, &x) ?
		pec_physical_coordinate(x) : 0.0f;
}

float pes_translation_y(const struct pes_decoder * const decoder)
{
	int y;

	return decode_i16lsb(decoder, decoder->cembone_offset + 53, &y) ?
		pec_physical_coordinate(y) : 0.0f;
}

float pes_width(const struct pes_decoder * const decoder)
{
	int width;

	return decode_u16lsb(decoder, decoder->cembone_offset + 55, &width) ?
		pec_physical_coordinate(width) : 0.0f;
}

float pes_height(const struct pes_decoder * const decoder)
{
	int height;

	return decode_u16lsb(decoder, decoder->cembone_offset + 57, &height) ?
		pec_physical_coordinate(height) : 0.0f;
}

float pes_hoop_width(const struct pes_decoder * const decoder)
{
	return (float)decoder->hoop_width;
}

float pes_hoop_height(const struct pes_decoder * const decoder)
{
	return (float)decoder->hoop_height;
}

bool pes_stitch_foreach(const struct pes_decoder * const decoder,
	const pes_block_callback block_cb, const pes_stitch_callback stitch_cb,
	void * const arg)
{
	return stitch_foreach(decoder, block_cb, stitch_cb, NULL, arg);
}

struct pec_decoder *pes_pec_decoder(const struct pes_decoder * const decoder)
{
	return decoder->pec;
}
