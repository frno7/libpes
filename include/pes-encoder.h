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

#ifndef PESLIB_PES_ENCODER_H
#define PESLIB_PES_ENCODER_H

#include <stdbool.h>
#include <stdlib.h>

#include "pec.h"
#include "pes.h"

struct pes_encoder; /* PES encoder object forward declaration. */

/**
 * Create a PES encoder object.
 *
 * @return Allocated PES encoder object or NULL. Must be freed using
 * `pes_encoder_free()`.
 */
struct pes_encoder *pes_encoder_init();

/**
 * Free allocated PES encoder object.
 *
 * @param encoder PES obect to free. Ignored if NULL.
 */
void pes_encoder_free(struct pes_encoder * const encoder);

/**
 * Append a thread to the PES encoder object. Appended threads are indexed
 * from zero.
 *
 * @param encoder PES encoder object.
 * @param thread PES thread.
 * @return True if thread was successfully appended, else false.
 */
bool pes_append_thread(struct pes_encoder * const encoder,
	const struct pec_thread thread);

/**
 * Append a stitch to the PES encoder object. Note that the given thread
 * index must have been appended using `pes_append_thread()` before this call.
 *
 * @param encoder PES encoder object.
 * @param thread_index Thread index starting from zero.
 * @param x X coordinate of stitch [millimeter].
 * @param y Y coordinate of stitch [millimeter].
 * @return True if stitch was successfully appended, else false.
 */
bool pes_append_stitch(struct pes_encoder * const encoder,
	const int thread_index, const float x, const float y);

/**
 * Append a jump stitch to the PES encoder object. Note that the given thread
 * index must have been appended using `pes_append_thread()` before this call.
 *
 * @param encoder PES encoder object.
 * @param x X coordinate of stitch [millimeter].
 * @param y Y coordinate of stitch [millimeter].
 * @param thread_index Thread index starting from zero.
 * @return True if stitch was successfully appended, else false.
 */
bool pes_append_jump_stitch(struct pes_encoder * const encoder,
	const int thread_index, const float x, const float y);

/**
 * Set affine transform for PES object.
 *
 * FIXME: How and when is this matrix applied?
 * FIXME: Is the translation part properly scaled to millimeters?
 *
 * @param encoder PES encoder object.
 * @param affine_transform Affine PES transform matrix.
 */
void pes_encode_transform(struct pes_encoder * const encoder,
	const struct pes_transform affine_transform);

/**
 * Callback with successively encoded PES data.
 *
 * @see pes_encode1
 *
 * @param data Encoded data.
 * @param size Size of encoded data.
 * @param arg Argument pointer supplied to `pes_encode1()`.
 * @return Turn true to continue processing or false to abort.
 */
typedef bool (*pes_encode_callback)(const void * const data,
	const size_t size, void * const arg);

/**
 * Encode PES version 1 sending data to the provided callback.
 *
 * Note that PES version 1 does not encode custom threads or hoop size.
 *
 * @param encoder PES encoder object.
 * @param encode_cb Callback to invoke for encoded data.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool pes_encode1(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg);

/**
 * Encode PES version 4 sending data to the provided callback.
 *
 * Note that PES version 4 does not encode custom threads.
 *
 * FIXME: PES version 4 encoder
 *
 * @param encoder PES encoder object.
 * @param encode_cb Callback to invoke for encoded data.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool pes_encode4(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg);

/**
 * Encode PES version 5 sending data to the provided callback.
 *
 * FIXME: PES version 5 encoder
 *
 * @param encoder PES encoder object.
 * @param encode_cb Callback to invoke for encoded data.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool pes_encode5(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg);

/**
 * Encode PES version 6 sending data to the provided callback.
 *
 * FIXME: PES version 6 encoder
 *
 * @param encoder PES encoder object.
 * @param encode_cb Callback to invoke for encoded data.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool pes_encode6(const struct pes_encoder * const encoder,
	const pes_encode_callback encode_cb, void * const arg);

/**
 * Return size of encoded PES version 1 data in bytes.
 *
 * @param encoder PES encoder object.
 * @return Size of encoded PES data in bytes, or zero on failure.
 */
size_t pes_encode1_size(const struct pes_encoder * const encoder);

/**
 * Return size of encoded PES version 4 data in bytes.
 *
 * @param encoder PES encoder object.
 * @return Size of encoded PES data in bytes, or zero on failure.
 */
size_t pes_encode4_size(const struct pes_encoder * const encoder);

/**
 * Return size of encoded PES version 5 data in bytes.
 *
 * @param encoder PES encoder object.
 * @return Size of encoded PES data in bytes, or zero on failure.
 */
size_t pes_encode5_size(const struct pes_encoder * const encoder);

/**
 * Return size of encoded PES version 6 data in bytes.
 *
 * @param encoder PES encoder object.
 * @return Size of encoded PES data in bytes, or zero on failure.
 */
size_t pes_encode6_size(const struct pes_encoder * const encoder);

#endif /* PESLIB_PES_ENCODER_H */
