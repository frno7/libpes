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

#include "pes-decoder.h"
#include "pes-svg-emb-transcoder.h"
#include "svg-emb-encoder.h"

struct transcoder_state {
	struct svg_emb_encoder *encoder;

	enum pec_stitch_type stitch_type;
	struct pec_thread thread;
	bool jump;

	void * const arg;
};

static bool block_cb(const struct pec_thread thread,
	const int stitch_count, const enum pec_stitch_type stitch_type,
	void * const arg)
{
	struct transcoder_state * const state = arg;

	state->thread = thread;
	state->stitch_type = stitch_type;
	if (stitch_type != PEC_STITCH_NORMAL)
		state->jump = true;

	return true;
}

static bool stitch_cb(const int stitch_index,
	const float x, const float y, void * const arg)
{
	struct transcoder_state * const state = arg;

	if (state->stitch_type != PEC_STITCH_NORMAL)
		return true;

	if (!(state->jump ? svg_emb_append_jump_stitch : svg_emb_append_stitch)
		(state->encoder, state->thread.index, x, y))
		return false;

	state->jump = false;

	return true;
}

static bool transcode_threads(struct pes_decoder * const decoder,
	struct transcoder_state * const state)
{
	const int thread_count = pes_thread_count(decoder);

	for (int i = 0; i < thread_count; i++)
		if (!svg_emb_append_thread(state->encoder,
			pes_thread(decoder, i)))
			return false;

	return true;
}

static bool transcode_transform(struct pes_decoder * const decoder,
	struct transcoder_state * const state)
{
	svg_emb_encode_transform(state->encoder, pes_affine_transform(decoder));

	return true;
}

static bool transcode_stitches(struct pes_decoder * const decoder,
	struct transcoder_state * const state)
{
	return pes_stitch_foreach(decoder, block_cb, stitch_cb, state);
}

bool pes_svg_emb_transcode(const void * const data, const size_t size,
	const svg_emb_encode_callback encode_cb, void * const arg)
{
	struct transcoder_state state = {
		.encoder = svg_emb_encoder_init(),
		.arg = arg
	};

	struct pes_decoder * const decoder = pes_decoder_init(data, size);

	if (decoder == NULL || state.encoder == NULL ||
	    !transcode_threads(decoder, &state) ||
	    !transcode_transform(decoder, &state) ||
	    !transcode_stitches(decoder, &state) ||
	    !svg_emb_encode(state.encoder, encode_cb, arg)) {
		svg_emb_encoder_free(state.encoder);
		pes_decoder_free(decoder);
		return false;
	}

	svg_emb_encoder_free(state.encoder);
	pes_decoder_free(decoder);

	return true;
}
