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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "svg-emb-decoder.h"
#include "pes.h"
#include "sax.h"

struct svg_emb_decoder {
	const char *text;

	struct pes_transform affine_transform;

	int thread_count;
	struct pec_thread thread_list[PES_MAX_THREADS];
};

struct svg_emb_thread_state {
	struct svg_emb_decoder *decoder;

	bool path_element;

	sax_error_callback error_cb;
	void *arg;
};

struct svg_emb_stitch_state {
	const struct svg_emb_decoder *decoder;

	bool path_element;

	int block_index;
	int thread_index;
	int stitch_count;
	int stitch_index;

	svg_emb_block_callback block_cb;
	svg_emb_stitch_callback stitch_cb;
	sax_error_callback error_cb;
	void *arg;
};

typedef bool (*path_d_callback)(const float x, const float y, void * const arg);

static void thread_error_cb(struct sax_token error,
	const char * const message, void * const arg)
{
	struct svg_emb_thread_state * const state = arg;

	if (state->error_cb != NULL)
		state->error_cb(error, message, state->arg);
}

static void stitch_error_cb(struct sax_token error,
	const char * const message, void * const arg)
{
	struct svg_emb_stitch_state * const state = arg;

	if (state->error_cb != NULL)
		state->error_cb(error, message, state->arg);
}

static bool ishex(const char c)
{
	return ('0' <= c && c <= '9') ||
	       ('a' <= c && c <= 'f') ||
	       ('A' <= c && c <= 'F');
}

static int hex(const char c)
{
	return '0' <= c && c <= '9' ? c - '0' :
	       'a' <= c && c <= 'f' ? c - 'a' + 0xA :
	       'A' <= c && c <= 'F' ? c - 'A' + 0xA : 0;
}

static bool parse_rgb(const struct sax_token color, struct pec_rgb * const rgb)
{
	if (color.length == 7 && color.cursor[0] == '#' &&
	    ishex(color.cursor[1]) && ishex(color.cursor[2]) &&
	    ishex(color.cursor[3]) && ishex(color.cursor[4]) &&
	    ishex(color.cursor[5]) && ishex(color.cursor[6])) {
		rgb->r = (hex(color.cursor[1]) << 4) | hex(color.cursor[2]);
		rgb->g = (hex(color.cursor[3]) << 4) | hex(color.cursor[4]);
		rgb->b = (hex(color.cursor[5]) << 4) | hex(color.cursor[6]);
		return true;
	}

	return false;
}

static int find_thread_index(const struct svg_emb_decoder * const decoder,
	const struct pec_rgb rgb)
{
	for (int i = 0; i < decoder->thread_count; i++)
		if (decoder->thread_list[i].rgb.r == rgb.r &&
		    decoder->thread_list[i].rgb.g == rgb.g &&
		    decoder->thread_list[i].rgb.b == rgb.b)
			return i;

	return -1;
}

static bool parse_color(const struct sax_token color,
	struct svg_emb_thread_state * const state)
{
	struct pec_rgb rgb = { 0 };

	if (!parse_rgb(color, &rgb)) {
		thread_error_cb(color,
			"Invalid color not in #RRGGBB hex format", state);
		return false;
	}

	if (find_thread_index(state->decoder, rgb) == -1) {
		/* FIXME: Can the thread assignment be improved? */

		const int palette_index = pec_palette_index_by_rgb(rgb);
		const struct pec_thread p = pec_palette_thread(palette_index);
		struct pec_thread * const c = &state->decoder->
			thread_list[state->decoder->thread_count];

		*c = p;
		c->rgb = rgb;
		c->index = state->decoder->thread_count++;
	}

	return true;
}

static bool g_cb(const struct sax_token attribute,
	const struct sax_token value, void * const arg)
{
	struct svg_emb_thread_state * const state = arg;

	if (sax_strcmp(attribute, "transform") == 0 &&
		sscanf(value.cursor, "matrix(%f %f %f %f %f %f)",
			&state->decoder->affine_transform.matrix[0][0],
			&state->decoder->affine_transform.matrix[0][1],
			&state->decoder->affine_transform.matrix[1][0],
			&state->decoder->affine_transform.matrix[1][1],
			&state->decoder->affine_transform.matrix[2][0],
			&state->decoder->affine_transform.matrix[2][1]) != 6)
		return false;

	return true;
}

static bool thread_element_opening_cb(const struct sax_token element,
	void * const arg)
{
	struct svg_emb_thread_state * const state = arg;

	state->path_element = (sax_strcmp(element, "path") == 0);

	if (sax_strcmp(element, "g") == 0)
		if (!sax_parse_attributes(element, g_cb, stitch_error_cb, state))
			return false;

	return true;
}

static bool thread_attribute_cb(const struct sax_token attribute,
	const struct sax_token value, void * const arg)
{
	struct svg_emb_thread_state * const state = arg;

	if (state->path_element && sax_strcmp(attribute, "stroke") == 0)
		if (!parse_color(value, state))
			return false;

	return true;
}

static bool stitch_count(const float x, const float y, void * const arg)
{
	int * const counter = (int *)arg;

	(*counter)++;

	return true;
}

static bool internal_stitch_cb(const float x, const float y, void * const arg)
{
	struct svg_emb_stitch_state * const state = arg;

	state->stitch_index++;

	return state->stitch_cb != NULL ?
		state->stitch_cb(state->stitch_index - 1, x, y, state->arg) :
		true;
}

static bool parse_coordinate(const char ** const s, float * const f)
{
	const char * const p = *s;
	char *e;

	*f = strtof(*s, &e);
	*s = e;

	return *s != p;
}

static bool parse_d(const char *d, const size_t length,
	const path_d_callback d_cb, void * const arg)
{
	const char * const e = &d[length]; /* Used to check end of token. */

	while (d < e && isspace(*d))
		d++;

	while (d < e && *d != '\0') {
		if (*d != 'M' && *d != 'L')
			return false;
		d++;

		float x, y;

		if (!parse_coordinate(&d, &x) ||
		    !parse_coordinate(&d, &y))
			return false;

		if (e < d || !d_cb(x, y, arg))
			return false;

		while (d < e && isspace(*d))
			d++;
	}

	return d == e; /* All characters have been parsed successfully. */
}

static bool path_cb(const struct sax_token attribute,
	const struct sax_token value, void * const arg)
{
	struct svg_emb_stitch_state * const state = arg;

	if (sax_strcmp(attribute, "d") == 0) {
		if (!parse_d(value.cursor, value.length,
			stitch_count, &state->stitch_count)) {
			stitch_error_cb(value,
				"Malformed \"d\" attribute", state);
			return false;
		}
	} else if (sax_strcmp(attribute, "stroke") == 0) {
		struct pec_rgb rgb = { 0 };

		if (!parse_rgb(value, &rgb)) {
			stitch_error_cb(value,
				"Invalid color not in #RRGGBB hex format", state);
			return false;
		}

		state->thread_index = find_thread_index(state->decoder, rgb);
	}

	return true;
}

static bool stitch_element_opening_cb(const struct sax_token element,
	void * const arg)
{
	struct svg_emb_stitch_state * const state = arg;

	if (sax_strcmp(element, "path") == 0) {
		state->path_element = true;

		state->thread_index = -1;
		state->stitch_count = 0;
		state->stitch_index = 0;

		if (!sax_parse_attributes(element, path_cb, stitch_error_cb, state))
			return false;

		if (state->thread_index == -1) {
			stitch_error_cb(element,
				"Missing \"stroke\" attribute", state);
			return false;
		}

		if (state->block_cb != NULL)
			if (!state->block_cb(state->block_index++,
				svg_emb_thread(state->decoder, state->thread_index),
				state->stitch_count, state->arg))
				return false;
	}

	return true;
}

static bool stitch_attribute_cb(const struct sax_token attribute,
	const struct sax_token value, void * const arg)
{
	struct svg_emb_stitch_state * const state = arg;

	if (state->path_element)
		if (sax_strcmp(attribute, "d") == 0) {
			if (!parse_d(value.cursor, value.length,
				internal_stitch_cb, state))
				return false;
		}

	return true;
}

static bool init_threads(struct svg_emb_decoder * const decoder,
	const sax_error_callback error_cb, void * const arg)
{
	struct svg_emb_thread_state state = {
		.decoder = decoder,
		.error_cb = error_cb,
		.arg = arg
	};

	return sax_parse_text(decoder->text, thread_element_opening_cb,
		NULL, thread_attribute_cb, thread_error_cb, &state);
}

struct svg_emb_decoder *svg_emb_decoder_init(const char * const text,
	const sax_error_callback error_cb, void * const arg)
{
	const size_t length = strlen(text);
	struct svg_emb_decoder * const decoder =
		calloc(1, sizeof(*decoder) + length + 1);

	if (decoder != NULL) {
		decoder->affine_transform.matrix[0][0] = 1.0f;
		decoder->affine_transform.matrix[1][1] = 1.0f;

		decoder->text = (const char *)&decoder[1];
		memcpy(&decoder[1], text, length + 1);

		if (!init_threads(decoder, error_cb, arg)) {
			svg_emb_decoder_free(decoder);
			return false;
		}
	}

	return decoder;
}

void svg_emb_decoder_free(struct svg_emb_decoder * const decoder)
{
	free(decoder);
}

struct pes_transform svg_emb_affine_transform(
	const struct svg_emb_decoder * const decoder)
{
	return decoder->affine_transform;
}

int svg_emb_thread_count(const struct svg_emb_decoder * const decoder)
{
	return decoder->thread_count;
}

const struct pec_thread svg_emb_thread(
	const struct svg_emb_decoder * const decoder, const int thread_index)
{
	return 0 <= thread_index && thread_index < decoder->thread_count ?
		decoder->thread_list[thread_index] : pec_undefined_thread();
}

bool svg_emb_stitch_foreach(const struct svg_emb_decoder * const decoder,
	const svg_emb_block_callback block_cb,
	const svg_emb_stitch_callback stitch_cb,
	const sax_error_callback error_cb, void * const arg)
{
	struct svg_emb_stitch_state state = {
		.decoder = decoder,
		.block_cb = block_cb,
		.stitch_cb = stitch_cb,
		.error_cb = error_cb,
		.arg = arg
	};

	return sax_parse_text(decoder->text, stitch_element_opening_cb,
		NULL, stitch_attribute_cb, stitch_error_cb, &state);
}
