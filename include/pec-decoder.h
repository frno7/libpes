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

#ifndef PESLIB_PEC_DECODER_H
#define PESLIB_PEC_DECODER_H

#include <stdbool.h>

#include "pec.h"

struct pec_decoder; /* PEC decoder object forward declaration. */

/**
 * Create a PEC decoder object.
 *
 * @param data Pointer to PEC data.
 * @param size Size of PEC data in bytes.
 * @return Allocated PEC decoder object or NULL. Must be freed using
 * `pec_decoder_free()`.
 */
struct pec_decoder *pec_decoder_init(const void * const data, const size_t size);

/**
 * Free allocated PEC decoder object.
 *
 * @param decoder PEC obect to free. Ignored if NULL.
 */
void pec_decoder_free(struct pec_decoder * const decoder);

/**
 * Return label string of PEC decoder object.
 *
 * @param decoder PEC decoder object.
 * @return Pointer to label of PEC decoder object, valid for PEC decoder
 * object lifetime. Never NULL.
 */
const char *pec_label(const struct pec_decoder * const decoder);

/**
 * Return number of PEC threads.
 *
 * @param decoder PEC decoder object.
 * @return Number of threads.
 */
int pec_thread_count(const struct pec_decoder * const decoder);

/**
 * Return PEC thread for given thread index.
 *
 * @param decoder PEC decoder object.
 * @param thread_index Index of thread.
 * @return PEC thread, valid for PEC decoder object lifetime, or NULL.
 */
const struct pec_thread pec_thread(const struct pec_decoder * const decoder,
	const int thread_index);

/**
 * Return number of PEC object stitches.
 *
 * @param decoder PEC decoder object.
 * @return Number of stitches.
 */
int pec_stitch_count(const struct pec_decoder * const decoder);

/**
 * Callback for PEC stitch iteration.
 *
 * @see pec_stitch_foreach
 *
 * @param stitch_index Index of stitch.
 * @param x X coordinate [millimeter].
 * @param y Y coordinate [millimeter].
 * @param stitch_type Stitch type, PEC_STITCH_STOP indicates change of threads.
 * @param arg Argument pointer supplied to `pec_stitch_foreach()`.
 * @return Turn true to continue processing or false to abort.
 */
typedef bool (*pec_stitch_callback)(const int stitch_index,
	const float x, const float y, const enum pec_stitch_type stitch_type,
	void * const arg);

/**
 * Iterate over all PEC stitches.
 *
 * @param decoder PEC decoder object.
 * @param stitch_cb Callback to invoke for all stitches.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool pec_stitch_foreach(const struct pec_decoder * const decoder,
	const pec_stitch_callback stitch_cb, void * const arg);

/**
 * Return pixel width of PEC thumbnails.
 *
 * @param decoder PEC decoder object.
 * @return Width of PEC thumbnails [pixel].
 */
int pec_thumbnail_width(const struct pec_decoder * const decoder);

/**
 * Return pixel height of PEC thumbnails.
 *
 * @param decoder PEC decoder object.
 * @return Height of PEC thumbnails [pixel].
 */
int pec_thumbnail_height(const struct pec_decoder * const decoder);

/**
 * Return pixel value of PEC thumbnail. Coordinate origin is top-left corner
 * at (0,0).
 *
 * @param decoder PEC decoder object.
 * @param thumbnail_index Index of thumbnail. 0 is main thumbnail and 1..
 * represent all PEC threads.
 * @param x X coordinate of pixel from left.
 * @param y Y coordinate of pixel from top.
 * @return True if pixel is set else false.
 */
bool pec_thumbnail_pixel(const struct pec_decoder * const decoder,
	const int thumbnail_index, const int x, const int y);

/**
 * Convert raw PEC coordinate value to physical value [millimeter].
 *
 * @param c Raw PEC coordinate value.
 * @return Physical PEC coordinate value [millimeter].
 */
float pec_physical_coordinate(const int c);

#endif /* PESLIB_PEC_DECODER_H */
