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

#ifndef PESLIB_SVG_EMB_DECODER_H
#define PESLIB_SVG_EMB_DECODER_H

#include "pes-decoder.h"
#include "sax.h"

struct svg_emb_decoder; /* SVG embroidery decoder object forward declaration. */

/**
 * Create an SVG embroidery decoder object.
 *
 * @param text Pointer to SVG XML text.
 * @param error_cb Invoked for parsing errors. Ignored if NULL.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return Allocated SVG embroidery decoder object or NULL. Must be freed using
 * `svg_emb_decoder_free()`.
 */
struct svg_emb_decoder *svg_emb_decoder_init(const char * const text,
	const sax_error_callback error_cb, void * const arg);

/**
 * Free allocated SVG embroidery decoder object.
 *
 * @param decoder SVG embroidery obect to free. Ignored if NULL.
 */
void svg_emb_decoder_free(struct svg_emb_decoder * const decoder);

/**
 * Return affine transform matrix.
 *
 * @param decoder SVG embroidery decoder object.
 * @return Affine transform matrix.
 */
struct pes_transform svg_emb_affine_transform(
	const struct svg_emb_decoder * const decoder);

/**
 * Return number of threads.
 *
 * @param decoder SVG embroidery decoder object.
 * @return Number of custom threads.
 */
int svg_emb_thread_count(const struct svg_emb_decoder * const decoder);

/**
 * Return thread for given index.
 *
 * @param decoder SVG embroidery decoder object.
 * @param thread_index Index of thread.
 * @return SVG embroidery thread for given index, or an undefined thread.
 * or NULL.
 */
const struct pec_thread svg_emb_thread(
	const struct svg_emb_decoder * const decoder, const int thread_index);

/**
 * Return number of SVG embroidery decoder object stitches.
 *
 * @param decoder SVG embroidery decoder object.
 * @return Number of stitches.
 */
int svg_emb_stitch_count(const struct svg_emb_decoder * const decoder);

/**
 * Callback for blocks during SVG embroidery stitch iteration. This callback
 * is always invoked before `svg_emb_stitch_callback`.
 *
 * @see svg_emb_stitch_foreach
 *
 * @param block_index Index of block.
 * @param thread Thread for following stitches.
 * @param stitch_count Number of stitches within the block.
 * @param arg Argument pointer supplied to `svg_emb_stitch_foreach()`.
 * @return Turn true to continue processing or false to abort.
 */
typedef bool (*svg_emb_block_callback)(const int block_index,
	const struct pec_thread thread, const int stitch_count,
	void * const arg);

/**
 * Callback for SVG embroidery stitch iteration.
 *
 * @see svg_emb_stitch_foreach
 *
 * @param stitch_index Index of stitch within the block.
 * @param x X coordinate [millimeter].
 * @param y Y coordinate [millimeter].
 * @param arg Argument pointer supplied to `svg_emb_stitch_foreach()`.
 * @return Turn true to continue processing or false to abort.
 */
typedef bool (*svg_emb_stitch_callback)(const int stitch_index,
	const float x, const float y, void * const arg);

/**
 * Iterate over all SVG stitches.
 *
 * @param decoder SVG decoder object.
 * @param block_cb Callback to invoke for all stitch blocks. Ignored if NULL.
 * @param stitch_cb Callback to invoke for all stitches. Ignored if NULL.
 * @param error_cb Invoked for parsing errors. Ignored if NULL.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool svg_emb_stitch_foreach(const struct svg_emb_decoder * const decoder,
	const svg_emb_block_callback block_cb,
	const svg_emb_stitch_callback stitch_cb,
	const sax_error_callback error_cb, void * const arg);

#endif /* PESLIB_SVG_EMB_DECODER_H */
