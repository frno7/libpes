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

#include "pes-encoder.h"
#include "svg-emb-decoder.h"
#include "svg-emb-pes-transcoder.h"

struct transcoder_state {
	struct pes_encoder *encoder;

	struct pec_thread thread;
	bool jump;

	const sax_error_callback error_cb;
	void * const arg;
};

static bool block_cb(const int block_index, const struct pec_thread thread,
	const int stitch_count, void * const arg)
{
	struct transcoder_state * const state = arg;

	state->thread = thread;
	state->jump = (block_index != 0);

	return true;
}

static bool stitch_cb(const int stitch_index,
	const float x, const float y, void * const arg)
{
	struct transcoder_state * const state = arg;

	if (!(state->jump ? pes_append_jump_stitch : pes_append_stitch)
		(state->encoder, state->thread.index, x, y))
		return false;

	state->jump = false;

	return true;
}

static void internal_error_cb(struct sax_token error,
	const char * const message, void * const arg)
{
	struct transcoder_state * const state = arg;

	if (state->error_cb != NULL)
		state->error_cb(error, message, state->arg);
}

static bool transcode_threads(struct svg_emb_decoder * const decoder,
	struct transcoder_state * const state)
{
	const int thread_count = svg_emb_thread_count(decoder);

	for (int i = 0; i < thread_count; i++)
		if (!pes_append_thread(state->encoder,
			svg_emb_thread(decoder, i)))
			return false;

	return true;
}

static bool transcode_transform(struct svg_emb_decoder * const decoder,
	struct transcoder_state * const state)
{
	pes_encode_transform(state->encoder, svg_emb_affine_transform(decoder));

	return true;
}

static bool transcode_stitches(struct svg_emb_decoder * const decoder,
	struct transcoder_state * const state)
{
	return svg_emb_stitch_foreach(decoder, block_cb, stitch_cb,
		internal_error_cb, state);
}

static bool transcode(
	bool (*pes_encode)(const struct pes_encoder * const encoder,
		const pes_encode_callback encode_cb, void * const arg),
	const char * const svg_emb_text,
	const pes_encode_callback encode_cb,
	const sax_error_callback error_cb, void * const arg)
{
	struct transcoder_state state = {
		.encoder = pes_encoder_init(),
		.error_cb = error_cb,
		.arg = arg
	};

	struct svg_emb_decoder * const decoder =
		svg_emb_decoder_init(svg_emb_text, internal_error_cb, &state);

	if (decoder == NULL || state.encoder == NULL ||
	    !transcode_threads(decoder, &state) ||
	    !transcode_transform(decoder, &state) ||
	    !transcode_stitches(decoder, &state) ||
	    !pes_encode(state.encoder, encode_cb, arg)) {
		pes_encoder_free(state.encoder);
		svg_emb_decoder_free(decoder);
		return false;
	}

	pes_encoder_free(state.encoder);
	svg_emb_decoder_free(decoder);

	return true;
}

bool svg_emb_pes1_transcode(const char * const svg_emb_text,
	const pes_encode_callback encode_cb,
	const sax_error_callback error_cb, void * const arg)
{
	return transcode(pes_encode1, svg_emb_text, encode_cb, error_cb, arg);
}

bool svg_emb_pes4_transcode(const char * const svg_emb_text,
	const pes_encode_callback encode_cb,
	const sax_error_callback error_cb, void * const arg)
{
	return transcode(pes_encode4, svg_emb_text, encode_cb, error_cb, arg);
}

bool svg_emb_pes5_transcode(const char * const svg_emb_text,
	const pes_encode_callback encode_cb,
	const sax_error_callback error_cb, void * const arg)
{
	return transcode(pes_encode5, svg_emb_text, encode_cb, error_cb, arg);
}

bool svg_emb_pes6_transcode(const char * const svg_emb_text,
	const pes_encode_callback encode_cb,
	const sax_error_callback error_cb, void * const arg)
{
	return transcode(pes_encode6, svg_emb_text, encode_cb, error_cb, arg);
}
