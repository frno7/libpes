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

#ifndef PESLIB_RUN_TESTS_H
#define PESLIB_RUN_TESTS_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define FATAL_EXIT(msg) do { fprintf(stderr, "%s:%d: %s\n", \
	__FILE__, __LINE__, (msg)); exit(EXIT_FAILURE); } while (false)

#define TEST_ASSERT(expr) do { if (!(expr)) \
	FATAL_EXIT("Assertion failed: " #expr); } while (false)

#define TEST_ENTRY(f) { f, #f }

struct test_entry {
	bool (*test)();
	const char * const name;
};

extern const struct test_entry test_suite_sax[];
extern const struct test_entry test_suite_svg_transcoder[];

#endif /* PESLIB_RUN_TESTS_H */
