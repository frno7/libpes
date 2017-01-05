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

#include "file.h"
#include "pec-decoder.h"
#include "pes-decoder.h"

static void print_stitch_type(const enum pec_stitch_type stitch_type)
{
	const char * const type_name =
		stitch_type == PEC_STITCH_NORMAL ? "NORMAL" :
		stitch_type == PEC_STITCH_JUMP   ? "JUMP"   :
		stitch_type == PEC_STITCH_STOP   ? "STOP"   :
		stitch_type == PEC_STITCH_TRIM   ? "TRIM"   : NULL;

	if (type_name != NULL)
		printf(" %s", type_name);
	else
		printf(" %d", stitch_type);
}

static bool print_stitch_block(const struct pec_thread thread,
	const int stitch_count, const enum pec_stitch_type stitch_type,
	void * const arg)
{
	printf("CSewSeg block %d %d", thread.index, stitch_count);
	print_stitch_type(stitch_type);
	putchar('\n');

	return true;
}

static bool print_stitch(const int stitch_index,
	const float x, const float y, void * const arg)
{
	printf("CSewSeg stitch %4d %6.1f %6.1f\n", stitch_index, x, y);

	return true;
}

static void print_thread(const struct pec_thread thread)
{
	printf(" %d #%02x%02x%02x %s %4s %2s %s",
		thread.index, thread.rgb.r, thread.rgb.g, thread.rgb.b,
		thread.type, thread.code, thread.id, thread.name);
}

static bool print_pec_stitch(const int stitch_index,
	const float x, const float y,
	const enum pec_stitch_type stitch_type, void * const cb_arg)
{
	printf("PEC stitch %4d %6.1f %6.1f", stitch_index, x, y);
	print_stitch_type(stitch_type);
	putchar('\n');

	return true;
}

static void print_pec_thumbnail(const struct pec_decoder * const decoder,
	const int thumbnail_index)
{
	const int w = pec_thumbnail_width(decoder);
	const int h = pec_thumbnail_height(decoder);

	for (int y = 0; y < h; y++, putchar('\n')) {
		printf("  ");
		for (int x = 0; x < w; x++)
			putchar(pec_thumbnail_pixel(decoder,
				thumbnail_index, x, y) ? '#' : '.');
	}
}

static void print_pec_thumbnails(const struct pec_decoder * const decoder)
{
	const int thread_count = pec_thread_count(decoder);

	printf("PEC thumbnail index 0\n");
	print_pec_thumbnail(decoder, 0);

	for (int i = 0; i < thread_count; i++) {
		const struct pec_thread thread = pec_thread(decoder, i);

		printf("PEC thumbnail index %d %s\n", i + 1, thread.name);
		print_pec_thumbnail(decoder, i + 1);
	}
}

static void print_hoop_size(const struct pes_decoder * const decoder)
{
	const float hoop_width = pes_hoop_width(decoder);
	const float hoop_height = pes_hoop_height(decoder);

	if (hoop_width != 0.0f || hoop_height != 0.0f)
		printf("header hoop size %.0f %.0f\n", hoop_width, hoop_height);
}

static void print_threads(const struct pes_decoder * const decoder)
{
	const int thread_count = pes_thread_count(decoder);

	for (int i = 0; i < thread_count; i++) {
		const struct pec_thread thread = pes_thread(decoder, i);

		printf("thread");
		print_thread(thread);
		putchar('\n');
	}
}

static void print_transformation(const struct pes_decoder * const decoder)
{
	struct pes_transform affine_transform = pes_affine_transform(decoder);

	printf("CEmbOne transform %f %f %f %f %f %f\n",
		affine_transform.matrix[0][0], affine_transform.matrix[0][1],
		affine_transform.matrix[1][0], affine_transform.matrix[1][1],
		affine_transform.matrix[2][0], affine_transform.matrix[2][1]);
}

static void print_bounds(const struct pes_decoder * const decoder)
{
	float min_x, min_y, max_x, max_y;

	pes_bounds1(decoder, &min_x, &min_y, &max_x, &max_y);
	printf("CEmbOne bounds1 %.1f %.1f %.1f %.1f\n", min_x, min_y, max_x, max_y);
	pes_bounds2(decoder, &min_x, &min_y, &max_x, &max_y);
	printf("CEmbOne bounds2 %.1f %.1f %.1f %.1f\n", min_x, min_y, max_x, max_y);
}

static bool print_pes(const struct file_buffer * const buf,
	const struct pes_decoder * const decoder)
{
	bool valid = true;

	printf("header name %s\n", pes_name(decoder));
	print_hoop_size(decoder);
	print_threads(decoder);
	print_transformation(decoder);

	printf("CEmbOne translation %.1f %.1f\n",
		pes_translation_x(decoder),
		pes_translation_y(decoder));

	printf("CEmbOne size %.1f %.1f\n",
		pes_width(decoder),
		pes_height(decoder));

	print_bounds(decoder);

	printf("CSewSeg stitch_count %d\n", pes_stitch_count(decoder));
	if (!pes_stitch_foreach(decoder, print_stitch_block, print_stitch, NULL)) {
		fprintf(stderr, "%s: PES stitch iterator error\n", buf->name);
		valid = false;
	}

	return valid;
}

static bool print_pec(const struct file_buffer * const buf,
	const struct pec_decoder * const decoder)
{
	bool valid = true;

	printf("PEC label %s\n", pec_label(decoder));

	const int thread_count = pec_thread_count(decoder);

	for (int i = 0; i < thread_count; i++) {
		const struct pec_thread thread = pec_thread(decoder, i);

		printf("PEC thread");
		print_thread(thread);
		putchar('\n');
	}

	printf("PEC stitch_count %d\n", pec_stitch_count(decoder));
	if (!pec_stitch_foreach(decoder, print_pec_stitch, NULL)) {
		fprintf(stderr, "%s: PEC stitch iterator error\n", buf->name);
		valid = false;
	}

	printf("PEC thumbnail size %d %d\n",
		pec_thumbnail_width(decoder),
		pec_thumbnail_height(decoder));

	print_pec_thumbnails(decoder);

	return valid;
}

static bool print_info(const struct file_buffer * const buf)
{
	if (buf->size < 8) {
		fprintf(stderr, "%s: File too short\n", buf->name);
		return false;
	}

	printf("header id %c%c%c%c\n"
	       "header version %c%c%c%c\n",
		buf->data[0], buf->data[1], buf->data[2], buf->data[3],
		buf->data[4], buf->data[5], buf->data[6], buf->data[7]);

	struct pes_decoder * const decoder =
		pes_decoder_init(buf->data, buf->size);
	if (decoder == NULL) {
		fprintf(stderr, "%s: File format error\n", buf->name);
		return false;
	}

	bool valid = print_pes(buf, decoder);
	if (!print_pec(buf, pes_pec_decoder(decoder)))
		valid = false;

	pes_decoder_free(decoder);

	return valid;
}

static void print_help()
{
	printf("Usage: pes-info [PES file]...\n"
	       "\n"
	       "The pesinfo tool prints content of PES embroidery files. Without arguments the\n"
	       "PES file is read from standard input.\n"
	       "\n"
	       "Options:\n"
	       "\n"
	       "  --help  Print this help text and exit.\n");
}

int main(const int argc, const char **argv)
{
	bool valid = true;

	if (argc == 1) {
		struct file_buffer buf = { .name = "stdin" };

		if (!read_file(stdin, &buf)) {
			perror(buf.name);
			valid = false;
		} else {
			if (!print_info(&buf))
				valid = false;

			free(buf.data);
		}
	} else {
		if (strcmp(argv[1], "--help") == 0) {
			print_help();
			return EXIT_SUCCESS;
		}

		for (int arg = 1; arg < argc; arg++) {
			struct file_buffer buf = { .name = argv[arg] };

			if (!read_path(buf.name, &buf)) {
				perror(buf.name);
				valid = false;
				continue;
			}

			if (!print_info(&buf))
				valid = false;

			free(buf.data);
		}
	}

	return valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
