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

#ifndef PESLIB_SVG_EMB_ENCODER_H
#define PESLIB_SVG_EMB_ENCODER_H

#include <stdbool.h>
#include <stdlib.h>

#include "pes.h"
#include "pec.h"

struct svg_emb_encoder; /* SVG embroidery encoder object forward declaration. */

/**
 * Create an SVG embroidery encoder object.
 *
 * @return Allocated SVG embroidery encoder object or NULL. Must be freed
 * using * `svg_emb_encoder_free()`.
 */
struct svg_emb_encoder *svg_emb_encoder_init();

/**
 * Free allocated SVG embroidery encoder object.
 *
 * @param encoder SVG embroidery obect to free. Ignored if NULL.
 */
void svg_emb_encoder_free(struct svg_emb_encoder * const encoder);

/**
 * Append a thread to the SVG embroidery encoder object. Appended threads are
 * indexed from zero.
 *
 * @param encoder SVG embroidery encoder object.
 * @param thread PEC thread.
 * @return True if thread was successfully appended, else false.
 */
bool svg_emb_append_thread(struct svg_emb_encoder * const encoder,
	const struct pec_thread thread);

/**
 * Append a stitch to the SVG embroidery encoder object. Note that the
 * given thread index must have been appended using `svg_emb_append_thread()`
 * before this call.
 *
 * @param encoder SVG embroidery encoder object.
 * @param thread_index Thread index starting from zero.
 * @param x X coordinate of stitch [millimeter].
 * @param y Y coordinate of stitch [millimeter].
 * @return True if stitch was successfully appended, else false.
 */
bool svg_emb_append_stitch(struct svg_emb_encoder * const encoder,
	const int thread_index, const float x, const float y);

/**
 * Set affine transform for SVG embroidery encoder object.
 *
 * @param encoder SVG embroidery encoder object.
 * @param affine_transform Affine transform matrix.
 */
void svg_emb_encode_transform(struct svg_emb_encoder * const encoder,
	const struct pes_transform affine_transform);

/**
 * Append a jump stitch to the SVG embroidery encoder object. Note that the
 * given thread index must have been appended using `svg_emb_append_thread()`
 * before this call.
 *
 * @param encoder SVG embroidery encoder object.
 * @param x X coordinate of stitch [millimeter].
 * @param y Y coordinate of stitch [millimeter].
 * @param thread_index Thread index starting from zero.
 * @return True if stitch was successfully appended, else false.
 */
bool svg_emb_append_jump_stitch(struct svg_emb_encoder * const encoder,
	const int thread_index, const float x, const float y);

/**
 * Callback with successively encoded SVG embroidery data.
 *
 * @see svg_emb_encode
 *
 * @param data Encoded data.
 * @param size Size of encoded data.
 * @param arg Argument pointer supplied to `svg_emb_encode()`.
 * @return Turn true to continue processing or false to abort.
 */
typedef bool (*svg_emb_encode_callback)(const void * const data,
	const size_t size, void * const arg);

/**
 * Encode SVG embroidery sending data to the provided callback.
 *
 * @param encoder SVG embroidery encoder object.
 * @param encode_cb Callback to invoke for encoded data.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool svg_emb_encode(const struct svg_emb_encoder * const encoder,
	const svg_emb_encode_callback encode_cb, void * const arg);

/**
 * Return size of encoded SVG embroidery data in bytes.
 *
 * @param encoder SVG embroidery encoder object.
 * @return Size of encoded SVG embroidery data in bytes, or zero on failure.
 */
size_t svg_emb_encode_size(const struct svg_emb_encoder * const encoder);

#endif /* PESLIB_SVG_EMB_ENCODER_H */
