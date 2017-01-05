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

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "file.h"

bool read_file(FILE * const file, struct file_buffer * const buf)
{
	buf->size = 0;
	buf->data = NULL;

	for (size_t size = 0; !feof(file);) {
		size_t space = size - buf->size;

		if (space == 0) {
			space = 8192;
			size += space;

			uint8_t * const data = realloc(buf->data, size);

			if (data == NULL)
				goto err;
			buf->data = data;
		}

		const size_t ret = fread(&buf->data[buf->size], 1, space, file);

		if (ferror(file))
			goto err;

		buf->size += ret;
	}

	uint8_t * const data = realloc(buf->data, buf->size + 1);

	if (data == NULL)
		goto err;
	buf->data = data;
	buf->data[buf->size] = '\0';

	return true;

err:
	free(buf->data);
	buf->size = 0;
	buf->data = NULL;

	return false;
}

bool read_path(const char * const path, struct file_buffer *buf)
{
	FILE * const file = fopen(path, "rb");

	if (file == NULL)
		return false;

	if (!read_file(file, buf)) {
		const int saved_errno = errno;

		fclose(file);
		errno = saved_errno;

		return false;
	}

	return fclose(file) == 0;
}
