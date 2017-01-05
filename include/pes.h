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

#ifndef PESLIB_PES_H
#define PESLIB_PES_H

#include "stdbool.h"

#define PES_MAX_THREADS 256

/** Affine PES transformation matrix. */
struct pes_transform {
	float matrix[3][2]; /** Affine 3x2 matrix where
		[ a b ]                                          [ a c e ]
		[ c d ] corresponds to the transformation matrix [ b d f ].
		[ e f ]                                          [ 0 0 1 ] */
};

/**
 * Determines whether affine transform is the identity transform.
 *
 * @param affine_transform Affine transform to check.
 * @return True if transform is identity, else false.
 */
bool pes_is_identity_transform(const struct pes_transform affine_transform);

#endif /* PESLIB_PES_H */
