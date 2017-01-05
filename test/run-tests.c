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

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "run-tests.h"

static void print_help()
{
	printf("Usage: run-tests [OPTIONS]...\n"
	       "\n"
	       "Options:\n"
	       "\n"
	       "  --help  Print this help text and exit.\n"
	       "  -v      Test verbosely.\n");
}

int main(const int argc, const char **argv)
{
	static const struct {
		const struct test_entry * const entries;
		const char * const name;
	} test_suites[] = {
		{ test_suite_sax,            "SAX"            },
		{ test_suite_svg_transcoder, "SVG transcoder" },
		{ NULL, NULL }
	};

	bool verbose = false;
	bool valid = true;

	if (argc == 2) {
		if (strcmp(argv[1], "-v") == 0) {
			verbose = true;
		} else if (strcmp(argv[1], "--help") == 0) {
			print_help();
			return EXIT_SUCCESS;
		} else
			goto invalid_args;
	} else if (argc != 1)
		goto invalid_args;

	for (int i = 0; test_suites[i].entries != NULL; i++) {
		if (verbose)
			printf("Testing %s\n", test_suites[i].name);

		for (int k = 0; test_suites[i].entries[k].test != NULL; k++) {
			if (verbose)
				printf("  %s...", test_suites[i].entries[k].name);

			const bool successful = test_suites[i].entries[k].test();

			if (!successful)
				valid = false;
			if (verbose)
				printf(successful ? " OK\n" : " FAILED\n");
		}
	}

	return valid ? EXIT_SUCCESS : EXIT_FAILURE;

invalid_args:
	fprintf(stderr, "run-tests: Invalid arguments\n"
		"Try 'run-tests --help' for more information.\n");

	return EXIT_FAILURE;
}
