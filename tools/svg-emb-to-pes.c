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
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "svg-emb-pes-transcoder.h"

#include "file.h"

struct svg_pes_state {
	struct file_buffer svg;

	struct {
		const char *name;
		FILE *file;
	} pes;

	struct pes_encoder *encoder;
};

static bool valid_pes_extension(const char * const path)
{
	const size_t length = strlen(path);

	return length >= 4 && path[length - 4] == '.' &&
	       (path[length - 3] == 'p' || path[length - 3] == 'P') &&
	       (path[length - 2] == 'e' || path[length - 2] == 'E') &&
	       (path[length - 1] == 's' || path[length - 1] == 'S');
}

static bool write_pes(const void * const data,
	const size_t size, void * const arg)
{
	struct svg_pes_state * const state = arg;

	if (fwrite(data, size, 1, state->pes.file) != 1) {
		perror(state->pes.name);
		return false;
	}

	return true;
}

static void error_cb(struct sax_token error,
	const char * const message, void * const arg)
{
	struct svg_pes_state * const state = arg;

	fprintf(stderr, "%s:%zu:%zu: %s: %.*s\n",
		state->svg.name, error.row, error.column,
		message, (int)error.length, error.cursor);
}

static bool svg_emb_to_pes(const char * const svg_path,
	const char * const pes_path)
{
	struct svg_pes_state state = {
		.svg = { .name = svg_path },
		.pes = { .name = pes_path }
	};
	bool valid = true;

	if (state.pes.name != NULL && !valid_pes_extension(state.pes.name)) {
		fprintf(stderr, "%s: Invalid PES extension\n", state.pes.name);
		return false;
	}

	if (state.svg.name == NULL || strcmp(state.svg.name, "-") == 0) {
		state.svg.name = "stdin";

		if (!read_file(stdin, &state.svg)) {
			perror(state.svg.name);
			return false;
		}
	} else if (!read_path(state.svg.name, &state.svg)) {
		perror(state.svg.name);
		return false;
	}

	if (state.pes.name == NULL || strcmp(state.pes.name, "-") == 0) {
		state.pes.name = "stdin";

		state.pes.file = stdout;
	} else {
		state.pes.file = fopen(state.pes.name, "wb");

		if (state.pes.file == NULL) {
			perror(state.pes.name);
			valid = false;
		}
	}

	if (valid && !svg_emb_pes1_transcode((const char *)state.svg.data,
		write_pes, error_cb, &state)) {
		fprintf(stderr, "%s: SVG embroidery to PES transcoding failed\n",
			state.svg.name);
		valid = false;
	}

	if (state.pes.file != stdout && state.pes.file != NULL &&
		fclose(state.pes.file) != 0) {
		perror(state.pes.name);
		valid = false;
	}

	free(state.svg.data);

	return valid;
}

static void print_help()
{
	printf("Usage: svg-emb-to-pes [SVG embroidery file] [PES file]\n"
	       "\n"
	       "The svg-emb-to-pes tool converts an SVG embroidery file to a PES embroidery file.\n"
	       "With a single argument the SVG file is read from standard input.\n"
	       "\n"
	       "Options:\n"
	       "\n"
	       "  --help  Print this help text and exit.\n");
}

int main(const int argc, const char **argv)
{
	bool valid = true;

	if (argc == 1) {
		valid = svg_emb_to_pes(NULL, NULL);
	} else if (argc == 2 && strcmp(argv[1], "--help") == 0) {
		print_help();
	} else if (argc == 2) {
		valid = svg_emb_to_pes(NULL, argv[1]);
	} else if (argc == 3) {
		valid = svg_emb_to_pes(argv[1], argv[2]);
	} else {
		fprintf(stderr, "svg-emb-to-pes: Invalid number of arguments\n"
			"Try 'svg-emb-to-pes --help' for more information.\n");
		valid = false;
	}

	return valid ? EXIT_SUCCESS: EXIT_FAILURE;
}
