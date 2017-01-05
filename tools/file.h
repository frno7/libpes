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

#ifndef PESLIB_FILE_H
#define PESLIB_FILE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct file_buffer {
	size_t size;
	uint8_t *data;
	const char *name;
};

bool read_file(FILE * const file, struct file_buffer * const buf);
bool read_path(const char * const path, struct file_buffer *buf);

#endif /* PESLIB_FILE_H */
