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

#ifndef PESLIB_PEC_ENCODER_H
#define PESLIB_PEC_ENCODER_H

#include <stdbool.h>
#include <stdint.h>

#include "pec.h"

struct pec_encoder; /* PEC encoder object forward declaration. */

/**
 * Create a PEC encoder object.
 *
 * @return Allocated PEC encoder object or NULL. Must be freed using
 * `pec_encoder_free()`.
 */
struct pec_encoder *pec_encoder_init();

/**
 * Free allocated PEC encoder object.
 *
 * @param encoder PEC obect to free. Ignored if NULL.
 */
void pec_encoder_free(struct pec_encoder * const encoder);

/**
 * Append a color to the PEC object. General RGB colors can be approximated to
 * a palette index using `pec_palette_index_by_rgb()`. Appending a color implies
 * that the next stitch is a stop stitch.
 *
 * @param encoder PEC encoder object.
 * @param palette_index Palette index for color.
 * @return True if color was successfully appended, else false.
 */
bool pec_append_thread(struct pec_encoder * const encoder,
	const int palette_index);

/**
 * Append a regular stitch to the PEC object. Note that at least one color
 * must have been appended using `pec_append_thread()` before this call.
 *
 * @param encoder PEC encoder object.
 * @param x X coordinate of stitch [millimeter].
 * @param y Y coordinate of stitch [millimeter].
 * @return True if stitch was successfully appended, else false.
 */
bool pec_append_stitch(struct pec_encoder * const encoder,
	const float x, const float y);

/**
 * Append a jump stitch to the PEC object. Note that at least one color
 * must have been appended using `pec_append_thread()` before this call.
 * Jump stitches are typically used after stop stitches.
 *
 * @param encoder PEC encoder object.
 * @param x X coordinate of stitch [millimeter].
 * @param y Y coordinate of stitch [millimeter].
 * @return True if stitch was successfully appended, else false.
 */
bool pec_append_jump_stitch(struct pec_encoder * const encoder,
	const float x, const float y);

/**
 * Append a trim stitch to the PEC object. Note that at least one color
 * must have been appended using `pec_append_thread()` before this call.
 * Trim stitches are used to move the needle without sewing between the
 * previous and this position.
 *
 * @param encoder PEC encoder object.
 * @param x X coordinate of stitch [millimeter].
 * @param y Y coordinate of stitch [millimeter].
 * @return True if stitch was successfully appended, else false.
 */
bool pec_append_trim_stitch(struct pec_encoder * const encoder,
	const float x, const float y);

/**
 * Callback with successively encoded PEC data.
 *
 * @see pec_encode
 *
 * @param data Encoded data.
 * @param size Size of encoded data.
 * @param arg Argument pointer supplied to `pec_encode()`.
 * @return Turn true to continue processing or false to abort.
 */
typedef bool (*pec_encode_callback)(const void * const data,
	const size_t size, void * const arg);

/**
 * Encode PEC sending data to the provided callback.
 *
 * @param encoder PEC encoder object.
 * @param encode_cb Callback to invoke for encoded data.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool pec_encode(const struct pec_encoder * const encoder,
	const pec_encode_callback encode_cb, void * const arg);

/**
 * Return size of encoded PEC data in bytes.
 *
 * @param encoder PEC encoder object.
 * @return Size of encoded PEC data in bytes, or zero on failure.
 */
size_t pec_encoded_size(const struct pec_encoder * const encoder);

/**
 * Convert physical value [millimeter] to raw PEC coordinate.
 *
 * @param c Physical PEC coordinate value [millimeter].
 * @return Raw PEC coordinate value.
 */
int pec_raw_coordinate(const float c);

#endif /* PESLIB_PEC_ENCODER_H */
