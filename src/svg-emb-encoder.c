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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pes.h"
#include "svg-emb-encoder.h"

#define SVG_EMB_MAX_THREADS 256

struct svg_emb_stitch {
	int thread_index;
	float x;
	float y;
	bool jump;
};

struct svg_emb_bounds {
	float min_x;
	float min_y;
	float max_x;
	float max_y;
	bool valid;
};

struct svg_emb_encoder {
	struct svg_emb_bounds bounds;
	struct pes_transform affine_transform;

	int thread_count;
	struct pec_thread thread_list[SVG_EMB_MAX_THREADS];

	int stitch_count;
	int stitch_capacity;
	struct svg_emb_stitch *stitch_list;
};

static bool encoded_size(const void * const data, const size_t size,
	void * const arg)
{
	int * const s = arg;

	*s += size;

	return *s >= 0;
}

static void update_bounds(struct svg_emb_bounds * const bounds,
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

static bool encode_stitch_footer(const svg_emb_encode_callback encode_cb,
	void * const arg)
{
	static const char * const stitch_end = "\" />\n";

	return encode_cb(stitch_end, strlen(stitch_end), arg);
}

static bool encode_stitch_header(const struct pec_thread * const thread,
	const svg_emb_encode_callback encode_cb, void * const arg)
{
	char header[1024];

	snprintf(header, sizeof(header),
		"  <path stroke=\"#%02x%02x%02x\" fill=\"none\" "
			"stroke-width=\"0.2\"\n"
		"        d=\"",
		thread->rgb.r, thread->rgb.g, thread->rgb.b);

	return encode_cb(header, strlen(header), arg);
}

static bool encode_stitch(const int stitch_index, const float x, const float y,
	const svg_emb_encode_callback encode_cb, void * const arg)
{
	char header[1024];

	snprintf(header, sizeof(header), "%s%c %5.1f %5.1f",
		stitch_index % 4 != 0 ? " " :
		stitch_index != 0 ? "\n           " : "",
		stitch_index == 0 ? 'M' : 'L', x, y);

	return encode_cb(header, strlen(header), arg);
}

static bool encode_stitch_list(const struct svg_emb_encoder * const encoder,
	const svg_emb_encode_callback encode_cb, void * const arg)
{
	for (int i = 0, stitch_index = 0, thread_index = -1;
	     i < encoder->stitch_count; i++, stitch_index++) {
		const struct svg_emb_stitch * const stitch =
			&encoder->stitch_list[i];

		/*
		 * A stitch jump can either be explicitly given or implicit
		 * on a thread index change. The first stitch is never a jump.
		 */
		const bool jump = (0 < i && (stitch->jump ||
			thread_index != stitch->thread_index));

		if (jump && !encode_stitch_footer(encode_cb, arg))
			return false;

		if (i == 0 || jump) {
			stitch_index = 0;

			if (!encode_stitch_header(&encoder->
				thread_list[stitch->thread_index],
				encode_cb, arg))
				return false;
		}

		if (!encode_stitch(stitch_index,
			stitch->x, stitch->y, encode_cb, arg))
			return false;

		thread_index = stitch->thread_index;
	}

	return encoder->stitch_count == 0 ||
		encode_stitch_footer(encode_cb, arg);
}

static bool append_stitch(struct svg_emb_encoder * const encoder,
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
			struct svg_emb_stitch * const stitch_list =
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
		(struct svg_emb_stitch){
			.thread_index = thread_index,
			.x = x,
			.y = y,
			.jump = jump
		};
	update_bounds(&encoder->bounds, x, y);
	encoder->stitch_count++;

	return true;
}

static bool encode_header(const struct svg_emb_encoder * const encoder,
	const svg_emb_encode_callback encode_cb, void * const arg)
{
	const float w = encoder->bounds.max_x - encoder->bounds.min_x;
	const float h = encoder->bounds.max_y - encoder->bounds.min_y;
	char header[1024];

	snprintf(header, sizeof(header),
		"<?xml version=\"1.0\"?>\n"
		"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"
		"  \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
		"<svg width=\"%.1fmm\" height=\"%.1fmm\" version=\"1.1\"\n"
		"     viewBox=\"%.1f %.1f %.1f %.1f\" "
		"xmlns=\"http://www.w3.org/2000/svg\">\n",
		w, h,
		/*
		 * FIXME: Bounds cannot be stored and must be computed since
		 * the affine transform affects them. Also apply rotational
		 * part for a general matrix multiplication of all coordinates
		 * to compute the bounds. Try WLD01.pes.
		 */
		encoder->bounds.min_x + encoder->affine_transform.matrix[2][0],
		encoder->bounds.min_y + encoder->affine_transform.matrix[2][1],
		w, h);

	return encode_cb(header, strlen(header), arg);
}

static bool encode_footer(const struct svg_emb_encoder * const encoder,
	const svg_emb_encode_callback encode_cb, void * const arg)
{
	static const char * const footer = "</svg>\n";

	return encode_cb(footer, strlen(footer), arg);
}

static bool encode_transform_header(const struct svg_emb_encoder * const encoder,
	const svg_emb_encode_callback encode_cb, void * const arg)
{
	char header[256];

	if (pes_is_identity_transform(encoder->affine_transform))
		return true;

	snprintf(header, sizeof(header), /* FIXME: Increase identation within <g> */
		"  <g transform=\"matrix(%.7f %.7f %.7f %.7f %.7f %.7f)\">\n",
		encoder->affine_transform.matrix[0][0],
		encoder->affine_transform.matrix[0][1],
		encoder->affine_transform.matrix[1][0],
		encoder->affine_transform.matrix[1][1],
		encoder->affine_transform.matrix[2][0],
		encoder->affine_transform.matrix[2][1]);

	return encode_cb(header, strlen(header), arg);
}

static bool encode_transform_footer(const struct svg_emb_encoder * const encoder,
	const svg_emb_encode_callback encode_cb, void * const arg)
{
	static const char * const footer = "  </g>\n";

	if (pes_is_identity_transform(encoder->affine_transform))
		return true;

	return encode_cb(footer, strlen(footer), arg);
}

struct svg_emb_encoder *svg_emb_encoder_init()
{
	struct svg_emb_encoder * const encoder =
		calloc(1, sizeof(struct svg_emb_encoder));

	if (encoder != NULL) {
		encoder->affine_transform.matrix[0][0] = 1.0f;
		encoder->affine_transform.matrix[1][1] = 1.0f;
	}

	return encoder;
}

void svg_emb_encoder_free(struct svg_emb_encoder * const encoder)
{
	if (encoder != NULL) {
		free(encoder->stitch_list);
		free(encoder);
	}
}

bool svg_emb_append_thread(struct svg_emb_encoder * const encoder,
	const struct pec_thread thread)
{
	if (SVG_EMB_MAX_THREADS <= encoder->thread_count)
		return false;

	encoder->thread_list[encoder->thread_count++] = thread;

	return true;
}

bool svg_emb_append_stitch(struct svg_emb_encoder * const encoder,
	const int thread_index, const float x, const float y)
{
	return append_stitch(encoder, thread_index, x, y, false);
}

bool svg_emb_append_jump_stitch(struct svg_emb_encoder * const encoder,
	const int thread_index, const float x, const float y)
{
	return append_stitch(encoder, thread_index, x, y, true);
}

void svg_emb_encode_transform(struct svg_emb_encoder * const encoder,
	const struct pes_transform affine_transform)
{
	memcpy(&encoder->affine_transform, &affine_transform,
		sizeof(encoder->affine_transform));
}

bool svg_emb_encode(const struct svg_emb_encoder * const encoder,
	const svg_emb_encode_callback encode_cb, void * const arg)
{
	return encode_header(encoder, encode_cb, arg) &&
	       encode_transform_header(encoder, encode_cb, arg) &&
	       encode_stitch_list(encoder, encode_cb, arg) &&
	       encode_transform_footer(encoder, encode_cb, arg) &&
	       encode_footer(encoder, encode_cb, arg);
}

size_t svg_emb_encode_size(const struct svg_emb_encoder * const encoder)
{
	int size = 0;

	if (!svg_emb_encode(encoder, encoded_size, &size))
		return 0;

	return (size_t)size;
}
