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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "run-tests.h"

#include "svg-emb-pes-transcoder.h"
#include "pes-svg-emb-transcoder.h"

struct buffer {
	size_t size;
	size_t capacity;
	uint8_t *data;
};

static bool encoded_size(const void * const data,
	const size_t size, void * const arg)
{
	size_t * const s = arg;

	*s += size;

	return true;
}

static bool encode_buffer(const void * const data,
	const size_t size, void * const arg)
{
	struct buffer * const buf = arg;

	if (buf->capacity < buf->size + size)
		return false;

	memcpy(&buf->data[buf->size], data, size);
	buf->size += size;

	return true;
}

static bool test_svg_transcoder()
{
	static const char * const xml =
		"<?xml version=\"1.0\"?>\n"
		"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"
		"  \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
		"<svg width=\"20.9mm\" height=\"35.5mm\" version=\"1.1\"\n"
		"     viewBox=\"28.5 7.4 20.9 35.5\" xmlns=\"http://www.w3.org/2000/svg\">\n"
		"  <path stroke=\"#feca15\" fill=\"none\" stroke-width=\"0.2\"\n"
		"        d=\"M  30.5  23.7 L  30.5  23.7 L  30.4  24.1 L  30.3  24.5\n"
		"           L  30.3  24.9 L  29.5  25.3 L  28.9  24.3 L  29.5  23.2\n"
		"           L  30.8  23.8 L  29.6  24.9 L  28.5  24.9 L  29.0  23.8\" />\n"
		"  <path stroke=\"#feca15\" fill=\"none\" stroke-width=\"0.2\"\n"
		"        d=\"M  40.6  26.9 L  45.9  10.5\" />\n"
		"  <path stroke=\"#feca15\" fill=\"none\" stroke-width=\"0.2\"\n"
		"        d=\"M  45.9  10.5 L  45.9  10.5 L  45.6  10.7 L  45.2  10.9\n"
		"           L  44.9  11.1 L  44.3  11.3 L  43.3  11.9 L  41.6  12.7\n"
		"           L  43.3  11.2 L  45.4   9.9 L  47.7   8.4 L  49.4   7.4\" />\n"
		"  <path stroke=\"#96aa02\" fill=\"none\" stroke-width=\"0.2\"\n"
		"        d=\"M  42.3   8.3 L  29.8  42.9\" />\n"
		"  <path stroke=\"#96aa02\" fill=\"none\" stroke-width=\"0.2\"\n"
		"        d=\"M  29.8  42.9 L  29.8  42.9 L  29.9  42.4 L  29.9  42.1\n"
		"           L  30.0  41.6 L  29.4  41.2 L  28.8  39.8 L  28.8  37.9\" />\n"
		"  <path stroke=\"#96aa02\" fill=\"none\" stroke-width=\"0.2\"\n"
		"        d=\"M  39.4  36.8 L  40.6  27.3\" />\n"
		"</svg>\n";

	struct {
		bool (* const transcode)(const char * const svg_emb_text,
			const pes_encode_callback encode_cb,
			const sax_error_callback error_cb, void * const arg);
		const char * const version;
	} versions[] = {
		{ svg_emb_pes1_transcode, "#PES0001" },
      /* FIXME: { svg_emb_pes4_transcode, "#PES0040" }, */
      /* FIXME: { svg_emb_pes5_transcode, "#PES0050" }, */
      /* FIXME: { svg_emb_pes6_transcode, "#PES0060" }, */
		{ NULL, NULL }
	};

	/* FIXME: Test PEC coordinate decoder, including 12 bits */

	for (int i = 0; versions[i].transcode != NULL; i++) {
		struct buffer pes = { };

		TEST_ASSERT(versions[i].transcode(xml, encoded_size, NULL, &pes.capacity));
		pes.data = malloc(pes.capacity);
		TEST_ASSERT(pes.data != NULL);
		TEST_ASSERT(versions[i].transcode(xml, encode_buffer, NULL, &pes));
		TEST_ASSERT(pes.size == pes.capacity);

		TEST_ASSERT(pes.size >= 8);
		TEST_ASSERT(strncmp((const char *)&pes.data[0], versions[i].version, 8) == 0);

		struct buffer svg = { .capacity = 1 };

		TEST_ASSERT(pes_svg_emb_transcode(pes.data, pes.size, encoded_size, &svg.capacity));
		svg.data = malloc(svg.capacity);
		TEST_ASSERT(svg.data != NULL);
		TEST_ASSERT(pes_svg_emb_transcode(pes.data, pes.size, encode_buffer, &svg));
		TEST_ASSERT(svg.size + 1 == svg.capacity);
		svg.data[svg.size] = '\0';

		TEST_ASSERT(strcmp(xml, (const char *)svg.data) == 0);

		free(svg.data);
		free(pes.data);
	}

	return true;
}

const struct test_entry test_suite_svg_transcoder[] = {
	TEST_ENTRY(test_svg_transcoder),
	TEST_ENTRY(NULL)
};
