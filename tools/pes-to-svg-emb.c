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
#include <unistd.h>

#include "pes-svg-emb-transcoder.h"

#include "file.h"

struct pes_svg_state {
	struct file_buffer pes;

	struct {
		const char *name;
		FILE *file;
	} svg;

	struct svg_emb_encoder *encoder;
};

static bool valid_svg_extension(const char * const path)
{
	const size_t length = strlen(path);

	return length >= 4 && path[length - 4] == '.' &&
	       (path[length - 3] == 's' || path[length - 3] == 'S') &&
	       (path[length - 2] == 'v' || path[length - 2] == 'V') &&
	       (path[length - 1] == 'g' || path[length - 1] == 'G');
}

static bool write_svg_emb(const void * const data,
	const size_t size, void * const arg)
{
	struct pes_svg_state * const state = arg;

	if (fwrite(data, size, 1, state->svg.file) != 1) {
		perror(state->svg.name);
		return false;
	}

	return true;
}

static bool pes_to_svg_emb(const char * const pes_path,
	const char * const svg_path)
{
	struct pes_svg_state state = {
		.pes = { .name = pes_path },
		.svg = { .name = svg_path }
	};
	bool valid = true;

	if (state.svg.name != NULL && !valid_svg_extension(state.svg.name)) {
		fprintf(stderr, "%s: Invalid SVG extension\n", state.svg.name);
		return false;
	}

	if (state.pes.name == NULL || strcmp(state.pes.name, "-") == 0) {
		state.pes.name = "stdin";

		if (!read_file(stdin, &state.pes)) {
			perror(state.pes.name);
			return false;
		}
	} else if (!read_path(state.pes.name, &state.pes)) {
		perror(state.pes.name);
		return false;
	}

	if (state.svg.name == NULL || strcmp(state.svg.name, "-") == 0) {
		state.svg.name = "stdin";

		state.svg.file = stdout;
	} else {
		state.svg.file = fopen(state.svg.name, "wb");

		if (state.svg.file == NULL) {
			perror(state.svg.name);
			valid = false;
		}
	}

	if (valid && !pes_svg_emb_transcode(state.pes.data, state.pes.size,
		write_svg_emb, &state)) {
		fprintf(stderr, "%s: PES to SVG embroidery transcoding failed\n",
			state.pes.name);
		valid = false;
	}

	if (state.svg.file != stdout && state.svg.file != NULL &&
		fclose(state.svg.file) != 0) {
		perror(state.svg.name);
		valid = false;
	}

	free(state.pes.data);

	return valid;
}

static void print_help()
{
	printf("Usage: pes-to-svg-emb [PES file] [SVG embroidery file]\n"
	       "\n"
	       "The pes-to-svg-emb tool converts a PES embroidery file to a primitive form of SVG printed\n"
	       "to standard output. Without arguments the PES file is read from standard input.\n"
	       "\n"
	       "Options:\n"
	       "\n"
	       "  --help  Print this help text and exit.\n");
}

int main(const int argc, const char **argv)
{
	bool valid = true;

	if (argc == 1) {
		valid = pes_to_svg_emb(NULL, NULL);
	} else if (argc == 2 && strcmp(argv[1], "--help") == 0) {
		print_help();
	} else if (argc == 2) {
		valid = pes_to_svg_emb(argv[1], NULL);
	} else if (argc == 3) {
		valid = pes_to_svg_emb(argv[1], argv[2]);
	} else {
		fprintf(stderr, "pes-to-svg-emb: Invalid number of arguments\n"
			"Try 'pes-to-svg-emb --help' for more information.\n");
		valid = false;
	}

	return valid ? EXIT_SUCCESS: EXIT_FAILURE;
}
