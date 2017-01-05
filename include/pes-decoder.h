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

#ifndef PESLIB_PES_DECODER_H
#define PESLIB_PES_DECODER_H

#include <stdbool.h>
#include <stdlib.h>

#include "pec.h"
#include "pes.h"

struct pes_decoder; /* PES decoder object forward declaration. */

/**
 * Create a PES decoder object.
 *
 * @param data Pointer to PES data.
 * @param size Size of PES data in bytes.
 * @return Allocated PES decoder object or NULL. Must be freed using
 * `pes_decoder_free()`.
 */
struct pes_decoder *pes_decoder_init(const void * const data, const size_t size);

/**
 * Free allocated PES decoder object.
 *
 * @param decoder PES obect to free. Ignored if NULL.
 */
void pes_decoder_free(struct pes_decoder * const decoder);

/**
 * Return version string of PES decoder object.
 *
 * @param decoder PES decoder object.
 * @return Pointer to version of PES decoder object, valid for PES decoder
 * object lifetime. Never NULL.
 */
const char *pes_version(const struct pes_decoder * const decoder);

/**
 * Return name string of PES decoder object.
 *
 * @param decoder PES decoder object.
 * @return Pointer to name of PES decoder object, valid for PES decoder
 * object lifetime. Never NULL.
 */
const char *pes_name(const struct pes_decoder * const decoder);

/**
 * Return number of PES threads.
 *
 * @param decoder PES decoder object.
 * @return Number of threads.
 */
int pes_thread_count(const struct pes_decoder * const decoder);

/**
 * Return PES thread for given thread index.
 *
 * @param decoder PES decoder object.
 * @param thread_index Index of thread.
 * @return PES thread for given index, or an undefined thread.
 */
const struct pec_thread pes_thread(const struct pes_decoder * const decoder,
	const int thread_index);

/**
 * Return number of PES object stitches.
 *
 * @param decoder PES decoder object.
 * @return Number of stitches.
 */
int pes_stitch_count(const struct pes_decoder * const decoder);

/**
 * Return affine PES transform matrix.
 *
 * FIXME: How and when is this matrix applied?
 * FIXME: Is the translation part properly scaled to millimeters?
 *
 * @param decoder PES decoder object.
 * @return Affine PES transform matrix.
 */
struct pes_transform pes_affine_transform(
	const struct pes_decoder * const decoder);

/**
 * Return PES object bounds of type 1.
 *
 * FIXME: What kind of bound is type 1?
 *
 * @param decoder PES decoder object.
 * @param min_x Minimum x coordinate [millimeter]. Ignored if NULL.
 * @param min_y Minimum y coordinate [millimeter]. Ignored if NULL.
 * @param max_x Maximum x coordinate [millimeter]. Ignored if NULL.
 * @param max_y Maximum y coordinate [millimeter]. Ignored if NULL.
 */
void pes_bounds1(const struct pes_decoder * const decoder,
	float * const min_x, float * const min_y,
	float * const max_x, float * const max_y);

/**
 * Return PES object bounds of type 2.
 *
 * FIXME: What kind of bound is type 2?
 *
 * @param decoder PES decoder object.
 * @param min_x Minimum x coordinate [millimeter]. Ignored if NULL.
 * @param min_y Minimum y coordinate [millimeter]. Ignored if NULL.
 * @param max_x Maximum x coordinate [millimeter]. Ignored if NULL.
 * @param max_y Maximum y coordinate [millimeter]. Ignored if NULL.
 */
void pes_bounds2(const struct pes_decoder * const decoder,
	float * const min_x, float * const min_y,
	float * const max_x, float * const max_y);

/**
 * Return x coordinate translation.
 *
 * FIXME: How and when is this x translation applied?
 *
 * @param decoder PES decoder object.
 * @return X coordinate translation [millimeter].
 */
float pes_translation_x(const struct pes_decoder * const decoder);

/**
 * Return y coordinate translation.
 *
 * FIXME: How and when is this y translation applied?
 *
 * @param decoder PES decoder object.
 * @return Y coordinate translation [millimeter].
 */
float pes_translation_y(const struct pes_decoder * const decoder);

/**
 * Return width of PES object.
 *
 * @param decoder PES decoder object.
 * @return Width of object [millimeter].
 */
float pes_width(const struct pes_decoder * const decoder);

/**
 * Return height of PES object.
 *
 * @param decoder PES decoder object.
 * @return Height of object [millimeter].
 */
float pes_height(const struct pes_decoder * const decoder);

/**
 * Return hoop width for PES object.
 *
 * @param decoder PES decoder object.
 * @return Hoop width [millimeter] or zero if undefined.
 */
float pes_hoop_width(const struct pes_decoder * const decoder);

/**
 * Return hoop height for PES object.
 *
 * @param decoder PES decoder object.
 * @return Hoop height [millimeter] or zero if undefined.
 */
float pes_hoop_height(const struct pes_decoder * const decoder);

/**
 * Callback for blocks during PES stitch iteration. This callback is always
 * invoked before `pes_stitch_callback`.
 *
 * @see pes_stitch_foreach
 *
 * @param thread Thread for following stitches.
 * @param stitch_count Number of stitches within the block.
 * @param stitch_type Type of stitch.
 * @param arg Argument pointer supplied to `pes_stitch_foreach()`.
 * @return Turn true to continue processing or false to abort.
 */
typedef bool (*pes_block_callback)(const struct pec_thread thread,
	const int stitch_count, const enum pec_stitch_type stitch_type,
	void * const arg);

/**
 * Callback for PES stitch iteration.
 *
 * @see pes_stitch_foreach
 *
 * @param stitch_index Index of stitch within the block.
 * @param x X coordinate [millimeter].
 * @param y Y coordinate [millimeter].
 * @param arg Argument pointer supplied to `pes_stitch_foreach()`.
 * @return Turn true to continue processing or false to abort.
 */
typedef bool (*pes_stitch_callback)(const int stitch_index,
	const float x, const float y, void * const arg);

/**
 * Iterate over all PES stitches.
 *
 * @param decoder PES decoder object.
 * @param block_cb Callback to invoke for all stitch blocks. Ignored if NULL.
 * @param stitch_cb Callback to invoke for all stitches. Ignored if NULL.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool pes_stitch_foreach(const struct pes_decoder * const decoder,
	const pes_block_callback block_cb, const pes_stitch_callback stitch_cb,
	void * const arg);

/**
 * Return PEC decoder object for PES object. Note that this particular PEC
 * decoder object must not be freed using `pec_decoder_free()`.
 *
 * @param decoder PES decoder object.
 * @return PEC decoder object associated with the given PES decoder object,
 * valid for PES decoder object lifetime.
 */
struct pec_decoder *pes_pec_decoder(const struct pes_decoder * const decoder);

#endif /* PESLIB_PES_DECODER_H */
