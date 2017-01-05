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

#ifndef PESLIB_PEC_H
#define PESLIB_PEC_H

#define PEC_MAX_THREADS 256

/** PEC stitch types. */
enum pec_stitch_type {
	PEC_STITCH_NORMAL = 0, /** Normal stitch. */
	PEC_STITCH_JUMP = 1,   /** Jump stitch. */
	PEC_STITCH_TRIM,       /** Trim stitch. */
	PEC_STITCH_STOP        /** Stop stitch. */
};

/** PEC RGB color. */
struct pec_rgb {
	int r; /** Red. */
	int g; /** Green. */
	int b; /** Blue. */
};

/** PEC thread. */
struct pec_thread {
	int index;          /** Thread index. */
	const char *id;     /** Thread id. */
	const char *code;   /** Thread code. */
	const char *name;   /** Name of thrad. */
	const char *type;   /** Type of thrad. */
	struct pec_rgb rgb; /** RGB color. */
};

/**
 * Return PEC thread in PEC palette.
 *
 * @param palette_index Palette index with the valid range of 1 to 40.
 * @return PEC palette thread for given index, or an undefined PEC thread.
 */
struct pec_thread pec_palette_thread(const int palette_index);

/**
 * Return PEC thread closest to given RGB color by RGB distance metric.
 *
 * @param rgb PEC RGB color.
 * @return PEC palette index.
 */
int pec_palette_index_by_rgb(const struct pec_rgb rgb);

/**
 * Return PEC thread representing an undefined thread.
 *
 * @return PEC thread.
 */
const struct pec_thread pec_undefined_thread();

#endif /* PESLIB_PEC_H */
