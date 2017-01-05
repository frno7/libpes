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

struct pec_string {
	int length;  /* Length is 0-31. */
	char s[32];  /* String terminated with NUL character. */
};

struct pec_decoder {
	int size;
	uint8_t *data;

	struct pec_string label;
};

static bool decode_u8(const struct pec_decoder * const decoder,
	const int offset, int * const value)
{
	if (offset < 0 || decoder->size < offset + 1)
		return false;

	*value = decoder->data[offset];

	return true;
}

static bool decode_u16lsb(const struct pec_decoder * const decoder,
	const int offset, int * const value)
{
	if (offset < 0 || decoder->size < offset + 2)
		return false;

	*value = ((int)decoder->data[offset + 0] << 0) |
	         ((int)decoder->data[offset + 1] << 8);

	return true;
}

static bool decode_thumbnail_offset(const struct pec_decoder * const decoder,
	int * const image_offset)
{
	if (!decode_u16lsb(decoder, 514, image_offset))
		return false;

	*image_offset += 512;

	return true;
}

static bool stitch_count(const int stitch_index,
	const float x, const float y, const enum pec_stitch_type stitch_type,
	void * const arg)
{
	int * const counter = (int *)arg;

	(*counter)++;

	return true;
}

static bool init_label(struct pec_decoder * const decoder)
{
	const int label_length = 19;

	if (decoder->size < label_length)
		return false;

	/* FIXME: Remove spaces and/or carrige return from label? */
	memcpy(&decoder->label.s[0], &decoder->data[0], label_length);
	decoder->label.length = (int)strlen(decoder->label.s);

	return true;
}

static bool decode_stitch_coordinate(const struct pec_decoder * const decoder,
	int * const stitch_offset, int * const c,
	enum pec_stitch_type * const stitch_type)
{
	int u;

	if (!decode_u8(decoder, (*stitch_offset)++, &u))
		return false;

	if ((u & 0x80) != 0) {
		int v;

		if ((u & 0x20) != 0)
			*stitch_type = PEC_STITCH_TRIM;
		if ((u & 0x10) != 0)
			*stitch_type = PEC_STITCH_JUMP;

		if (!decode_u8(decoder, (*stitch_offset)++, &v))
			return false;
		u = ((u & 0x0F) << 8) + v;

		if ((u & 0x800) != 0)
			u -= 0x1000;
	} else if (u >= 0x40)
		u -= 0x80;

	*c += u;

	return true;
}

struct pec_decoder *pec_decoder_init(const void * const data, const size_t size)
{
	if (size < 534 || INT_MAX/2 < size) /* PEC structure is at least 534 bytes. */
		return NULL;

	struct pec_decoder * const decoder = calloc(1, sizeof(*decoder) + size);

	if (decoder != NULL) {
		decoder->data = (uint8_t *)&decoder[1];
		decoder->size = (int)size;
		memcpy(decoder->data, data, size);

		if (!init_label(decoder)) {
			pec_decoder_free(decoder);
			return NULL;
		}
	}

	return decoder;
}

void pec_decoder_free(struct pec_decoder * const decoder)
{
	free(decoder);
}

const char *pec_label(const struct pec_decoder * const decoder)
{
	return decoder->label.s;
}

int pec_thread_count(const struct pec_decoder * const decoder)
{
	int raw;

	return decode_u8(decoder, 48, &raw) ? raw + 1 : 0;
}

const struct pec_thread pec_thread(const struct pec_decoder * const decoder,
	const int thread_index)
{
	struct pec_thread thread = pec_undefined_thread();
	const int palette_offset = 49 + thread_index;
	int palette_index;

	if (0 <= thread_index && thread_index < pec_thread_count(decoder) &&
	       decode_u8(decoder, palette_offset, &palette_index)) {
		thread = pec_palette_thread(palette_index);
		thread.index = thread_index;
	}

	return thread;
}

int pec_stitch_count(const struct pec_decoder * const decoder)
{
	int counter = 0;

	pec_stitch_foreach(decoder, stitch_count, &counter);

	return counter;
}

bool pec_stitch_foreach(const struct pec_decoder * const decoder,
	const pec_stitch_callback stitch_cb, void * const arg)
{
	int stitch_offset = 532;
	int x = 0, y = 0;

	for (int stitch_index = 0; ; stitch_index++) {
		enum pec_stitch_type stitch_type = PEC_STITCH_NORMAL;
		int cmd;

		if (!decode_u8(decoder, stitch_offset, &cmd))
			return false;

		if (cmd == 0xFF) {
			break;
		} else if (cmd == 0xFE) {
			if (!stitch_cb(stitch_index,
				pec_physical_coordinate(x),
				pec_physical_coordinate(y),
			       	PEC_STITCH_STOP, arg))
				return false;
			stitch_offset += 3; /* FIXME: Unknown data */
			continue;
		}

		if (!decode_stitch_coordinate(decoder,
			&stitch_offset, &x, &stitch_type) ||
		    !decode_stitch_coordinate(decoder,
			&stitch_offset, &y, &stitch_type))
			return false;

		if (!stitch_cb(stitch_index,
			pec_physical_coordinate(x),
			pec_physical_coordinate(y), stitch_type, arg))
			return false;
	}

	return true;
}

int pec_thumbnail_width(const struct pec_decoder * const decoder)
{
	int width;

	return decode_u8(decoder, 34, &width) ? 8 * width : 0;
}

int pec_thumbnail_height(const struct pec_decoder * const decoder)
{
	int height;

	return decode_u8(decoder, 35, &height) ? height : 0;
}

bool pec_thumbnail_pixel(const struct pec_decoder * const decoder,
	const int thumbnail_index, const int x, const int y)
{
	int thumbnail_offset, raw;

	if (!decode_thumbnail_offset(decoder, &thumbnail_offset))
		return false;

	const int w = pec_thumbnail_width(decoder);
       	const int h = pec_thumbnail_height(decoder);
	const int image_offset = thumbnail_index * w * h / 8;
	const int pixel_offset = (x + w * y) / 8;

	if (!decode_u8(decoder, thumbnail_offset +
		image_offset + pixel_offset, &raw))
		return false;

	return (raw & (1 << (x % 8))) != 0;
}

float pec_physical_coordinate(const int c)
{
	return 0.1f * (float)c;
}
