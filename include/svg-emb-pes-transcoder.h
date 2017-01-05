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

#ifndef PESLIB_SVG_EMB_PES_TRANSCODER_H
#define PESLIB_SVG_EMB_PES_TRANSCODER_H

#include "pes-encoder.h"
#include "sax.h"

/**
 * Transcode SVG embroidery to PES version 1 by sending data to the provided
 * callback.
 *
 * @param svg_emb_text SVG embroidery XML text.
 * @param encode_cb Callback to invoke for encoded data.
 * @param error_cb Invoked for SVG embroidery parsing errors. Ignored if NULL.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool svg_emb_pes1_transcode(const char * const svg_emb_text,
	const pes_encode_callback encode_cb,
	const sax_error_callback error_cb, void * const arg);

/**
 * Transcode SVG embroidery to PES version 4 by sending data to the provided
 * callback.
 *
 * @param svg_emb_text SVG embroidery XML text.
 * @param encode_cb Callback to invoke for encoded data.
 * @param error_cb Invoked for SVG embroidery parsing errors. Ignored if NULL.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool svg_emb_pes4_transcode(const char * const svg_emb_text,
	const pes_encode_callback encode_cb,
	const sax_error_callback error_cb, void * const arg);

/**
 * Transcode SVG embroidery to PES version 5 by sending data to the provided
 * callback.
 *
 * @param svg_emb_text SVG embroidery XML text.
 * @param encode_cb Callback to invoke for encoded data.
 * @param error_cb Invoked for SVG embroidery parsing errors. Ignored if NULL.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool svg_emb_pes5_transcode(const char * const svg_emb_text,
	const pes_encode_callback encode_cb,
	const sax_error_callback error_cb, void * const arg);

/**
 * Transcode SVG embroidery to PES version 6 by sending data to the provided
 * callback.
 *
 * @param svg_emb_text SVG embroidery XML text.
 * @param encode_cb Callback to invoke for encoded data.
 * @param error_cb Invoked for SVG embroidery parsing errors. Ignored if NULL.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool svg_emb_pes6_transcode(const char * const svg_emb_text,
	const pes_encode_callback encode_cb,
	const sax_error_callback error_cb, void * const arg);

#endif /* PESLIB_SVG_EMB_PES_TRANSCODER_H */
