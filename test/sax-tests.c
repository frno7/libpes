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

#include "sax.h"

enum sax_type {
	SAX_ELEMENT_OPENING,
	SAX_ELEMENT_CLOSING,
	SAX_ATTRIBUTE
};

struct sax_invocation {
	const enum sax_type type;
	const int row;
	const int column;
	const char * const a;
	const char * const b;
};

struct sax_reference {
	int index;
	const struct sax_invocation *list;
	int specials;
};

static int sign(const int n)
{
	return n < 0 ? -1 :
	       n > 0 ?  1 : 0;
}

static bool check_reference(const enum sax_type type,
	const struct sax_token * const a,
	const struct sax_token * const b, void * const arg)
{
	struct sax_reference * const ref = arg;
	const struct sax_invocation * const inv = &ref->list[ref->index];

	TEST_ASSERT(inv->type == type);
	TEST_ASSERT(inv->row == a->row);
	TEST_ASSERT(inv->column == a->column);
	TEST_ASSERT(inv->a != NULL);
	TEST_ASSERT(strlen(inv->a) == a->length);
	TEST_ASSERT(strncmp(inv->a, a->cursor, a->length) == 0);

	if (b != NULL) {
		TEST_ASSERT(inv->b != NULL);
		TEST_ASSERT(strlen(inv->b) == b->length);
		TEST_ASSERT(strncmp(inv->b, b->cursor, b->length) == 0);
	}

	ref->index++;

	return true;
}

static void error_cb(struct sax_token error, const char * const message,
	void * const arg)
{
	char buf[100];

	snprintf(buf, sizeof(buf), "%zd:%zd: %s",
		error.row, error.column, message);
	FATAL_EXIT(buf);
}

static bool attribute_cb(const struct sax_token attribute,
	const struct sax_token value, void * const arg)
{
	return check_reference(SAX_ATTRIBUTE, &attribute, &value, arg);
}

static bool element_closing_cb(const struct sax_token element, void * const arg)
{
	return check_reference(SAX_ELEMENT_CLOSING, &element, NULL, arg);
}

static bool sub_element_opening_cb(const struct sax_token element, void * const arg)
{
	return check_reference(SAX_ELEMENT_OPENING, &element, NULL, arg);
}

static bool test_parse_attributes(const struct sax_token element,
	struct sax_reference * const ref)
{
	static const struct sax_invocation list[] = {
		{ SAX_ATTRIBUTE,  4,  6, "width", "84.1mm" },
		{ SAX_ATTRIBUTE,  4, 21, "height", "51.1mm" },
		{ SAX_ATTRIBUTE,  4, 37, "version", "1.1" },
		{ SAX_ATTRIBUTE,  5,  6, "viewBox", "0 0 84.1 51.1" },
		{ SAX_ATTRIBUTE,  5, 30, "xmlns", "http://www.w3.org/2000/svg" },
		{ 0, 0, 0, NULL }
	};

	/* Note: static gives global lifetime. */
	static struct sax_reference subref = { 0, list };

	if (subref.specials != 0) /* Ensure tested once only. */
		return true;
	subref.specials++;

	TEST_ASSERT(sax_parse_attributes(element,
		attribute_cb, error_cb, &subref));
	TEST_ASSERT(subref.index == 5);
	TEST_ASSERT(subref.list[subref.index].a == NULL);
	ref->specials++;

	return true;
}

static bool test_parse_no_attributes(const struct sax_token element,
	struct sax_reference * const ref)
{
	static const struct sax_invocation list[] = {
		{ 0, 0, 0, NULL }
	};

	/* Note: static gives global lifetime. */
	static struct sax_reference subref = { 0, list };

	if (subref.specials != 0) /* Ensure tested once only. */
		return true;
	subref.specials++;

	TEST_ASSERT(sax_parse_attributes(element,
		attribute_cb, error_cb, &subref));
	TEST_ASSERT(subref.index == 0);
	TEST_ASSERT(subref.list[subref.index].a == NULL);
	ref->specials++;

	return true;
}

static bool test_parse_siblings(const struct sax_token element,
	struct sax_reference * const ref)
{
	static const struct sax_invocation list[] = {
		{ SAX_ELEMENT_OPENING, 11,  4, "path" },
		{ SAX_ATTRIBUTE, 11,  9, "stroke", "#fffc11" },
		{ SAX_ATTRIBUTE, 11, 26, "fill", "none" },
		{ SAX_ATTRIBUTE, 11, 38, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 11, 57, "stroke-dasharray", "1 1" },
		{ SAX_ATTRIBUTE, 12,  9, "d", "M  40.6  26.9 L  45.9  10.5" },
		{ SAX_ELEMENT_CLOSING, 11,  4, "path" },
		{ SAX_ELEMENT_OPENING, 13,  4, "path" },
		{ SAX_ATTRIBUTE, 13,  9, "stroke", "#fffc11" },
		{ SAX_ATTRIBUTE, 13, 26, "fill", "none" },
		{ SAX_ATTRIBUTE, 13, 38, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 14,  9, "d", "M  45.9  10.5 L  45.9  10.5 L  45.6  10.7 L  45.2  10.9\n"
				   "           L  44.9  11.1 L  44.3  11.3 L  43.3  11.9 L  41.6  12.7\n"
				   "           L  43.3  11.2 L  45.4   9.9 L  47.7   8.4 L  49.4   7.4" },
		{ SAX_ELEMENT_CLOSING, 13,  4, "path" },
		{ SAX_ELEMENT_OPENING, 17,  4, "g" },
		{ SAX_ELEMENT_OPENING, 18,  6, "path" },
		{ SAX_ATTRIBUTE, 18, 11, "stroke", "#b5dc10" },
		{ SAX_ATTRIBUTE, 18, 28, "fill", "none" },
		{ SAX_ATTRIBUTE, 18, 40, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 18, 59, "stroke-dasharray", "1 1" },
		{ SAX_ATTRIBUTE, 19, 11, "d", "M  42.3   8.3 L  29.8  42.9" },
		{ SAX_ELEMENT_CLOSING, 18,  6, "path" },
		{ SAX_ELEMENT_OPENING, 20,  6, "path" },
		{ SAX_ATTRIBUTE, 20, 11, "stroke", "#b5dc10" },
		{ SAX_ATTRIBUTE, 20, 28, "fill", "none" },
		{ SAX_ATTRIBUTE, 20, 40, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 21, 11, "d", "M  29.8  42.9 L  29.8  42.9 L  29.9  42.4 L  29.9  42.1\n"
				 "             L  30.0  41.6 L  29.4  41.2 L  28.8  39.8 L  28.8  37.9" },
		{ SAX_ELEMENT_CLOSING, 20,  6, "path" },
		{ SAX_ELEMENT_CLOSING, 23,  5, "g" },
		{ SAX_ELEMENT_OPENING, 24,  4, "path" },
		{ SAX_ATTRIBUTE, 24,  9, "stroke", "#b5dc10" },
		{ SAX_ATTRIBUTE, 24, 26, "fill", "none" },
		{ SAX_ATTRIBUTE, 24, 38, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 24, 57, "stroke-dasharray", "1 1" },
		{ SAX_ATTRIBUTE, 25,  9, "d", "M  39.4  36.8 L  40.6  27.3" },
		{ SAX_ELEMENT_CLOSING, 24,  4, "path" },
		{ 0, 0, 0, NULL }
	};

	/* Note: static gives global lifetime. */
	static struct sax_reference subref = { 0, list };

	if (subref.specials != 0) /* Ensure tested once only. */
		return true;
	subref.specials++;

	TEST_ASSERT(sax_parse_siblings(element, sub_element_opening_cb,
		element_closing_cb, attribute_cb, error_cb, &subref));
	TEST_ASSERT(subref.index == 35);
	TEST_ASSERT(subref.list[subref.index].a == NULL);
	ref->specials++;

	return true;
}

static bool test_parse_siblings_with_children(const struct sax_token element,
	struct sax_reference * const ref)
{
	static const struct sax_invocation list[] = {
		{ SAX_ELEMENT_OPENING, 24,  4, "path" },
		{ SAX_ATTRIBUTE, 24,  9, "stroke", "#b5dc10" },
		{ SAX_ATTRIBUTE, 24, 26, "fill", "none" },
		{ SAX_ATTRIBUTE, 24, 38, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 24, 57, "stroke-dasharray", "1 1" },
		{ SAX_ATTRIBUTE, 25,  9, "d", "M  39.4  36.8 L  40.6  27.3" },
		{ SAX_ELEMENT_CLOSING, 24,  4, "path" },
		{ 0, 0, 0, NULL }
	};

	/* Note: static gives global lifetime. */
	static struct sax_reference subref = { 0, list };

	if (subref.specials != 0) /* Ensure tested once only. */
		return true;
	subref.specials++;

	TEST_ASSERT(sax_parse_siblings(element, sub_element_opening_cb,
		element_closing_cb, attribute_cb, error_cb, &subref));
	TEST_ASSERT(subref.index == 7);
	TEST_ASSERT(subref.list[subref.index].a == NULL);
	ref->specials++;

	return true;
}

static bool test_parse_no_siblings(const struct sax_token element,
	struct sax_reference * const ref)
{
	static const struct sax_invocation list[] = {
		{ 0, 0, 0, NULL }
	};

	/* Note: static gives global lifetime. */
	static struct sax_reference subref = { 0, list };

	if (subref.specials != 0) /* Ensure tested once only. */
		return true;
	subref.specials++;

	TEST_ASSERT(sax_parse_siblings(element, sub_element_opening_cb,
		element_closing_cb, attribute_cb, error_cb, &subref));
	TEST_ASSERT(subref.index == 0);
	TEST_ASSERT(subref.list[subref.index].a == NULL);
	ref->specials++;

	return true;
}

static bool test_parse_no_children(const struct sax_token element,
	struct sax_reference * const ref)
{
	static const struct sax_invocation list[] = {
		{ 0, 0, 0, NULL }
	};

	/* Note: static gives global lifetime. */
	static struct sax_reference subref = { 0, list };

	if (subref.specials != 0) /* Ensure tested once only. */
		return true;
	subref.specials++;

	TEST_ASSERT(sax_parse_children(element, sub_element_opening_cb,
		element_closing_cb, attribute_cb, error_cb, &subref));
	TEST_ASSERT(subref.index == 0);
	TEST_ASSERT(subref.list[subref.index].a == NULL);
	ref->specials++;

	return true;
}

static bool test_parse_children(const struct sax_token element,
	struct sax_reference * const ref)
{
	static const struct sax_invocation list[] = {
		{ SAX_ELEMENT_OPENING, 18,  6, "path" },
		{ SAX_ATTRIBUTE, 18, 11, "stroke", "#b5dc10" },
		{ SAX_ATTRIBUTE, 18, 28, "fill", "none" },
		{ SAX_ATTRIBUTE, 18, 40, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 18, 59, "stroke-dasharray", "1 1" },
		{ SAX_ATTRIBUTE, 19, 11, "d", "M  42.3   8.3 L  29.8  42.9" },
		{ SAX_ELEMENT_CLOSING, 18,  6, "path" },
		{ SAX_ELEMENT_OPENING, 20,  6, "path" },
		{ SAX_ATTRIBUTE, 20, 11, "stroke", "#b5dc10" },
		{ SAX_ATTRIBUTE, 20, 28, "fill", "none" },
		{ SAX_ATTRIBUTE, 20, 40, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 21, 11, "d", "M  29.8  42.9 L  29.8  42.9 L  29.9  42.4 L  29.9  42.1\n"
				 "             L  30.0  41.6 L  29.4  41.2 L  28.8  39.8 L  28.8  37.9" },
		{ SAX_ELEMENT_CLOSING, 20,  6, "path" },
		{ 0, 0, 0, NULL }
	};

	/* Note: static gives global lifetime. */
	static struct sax_reference subref = { 0, list };

	if (subref.specials != 0) /* Ensure tested once only. */
		return true;
	subref.specials++;

	TEST_ASSERT(sax_parse_children(element, sub_element_opening_cb,
		element_closing_cb, attribute_cb, error_cb, &subref));
	TEST_ASSERT(subref.index == 13);
	TEST_ASSERT(subref.list[subref.index].a == NULL);
	ref->specials++;

	return true;
}

static bool element_opening_cb(const struct sax_token element, void * const arg)
{
	struct sax_reference * const ref = arg;

	/* Test sax_parse_attributes on the "svg" element. */
	if (strcmp(ref->list[ref->index].a, "svg") == 0)
		if (!test_parse_attributes(element, ref))
			return false;

	/* Test sax_parse_attributes on the "g" element. */
	if (strcmp(ref->list[ref->index].a, "g") == 0)
		if (!test_parse_no_attributes(element, ref))
			return false;

	/* Test sax_parse_siblings on the first "path" element. */
	if (strcmp(ref->list[ref->index].a, "path") == 0)
		if (!test_parse_siblings(element, ref))
			return false;

	/* Test sax_parse_siblings on the first "svg" element. */
	if (strcmp(ref->list[ref->index].a, "svg") == 0)
		if (!test_parse_no_siblings(element, ref))
			return false;

	/* Test sax_parse_siblings on the "g" element. */
	if (strcmp(ref->list[ref->index].a, "g") == 0)
		if (!test_parse_siblings_with_children(element, ref))
			return false;

	/*
	 * Test sax_parse_children on the first "path" element, which does
	 * not have children.
	 */
	if (strcmp(ref->list[ref->index].a, "path") == 0)
		if (!test_parse_no_children(element, ref))
			return false;

	/* Test sax_parse_children on the "g" element. */
	if (strcmp(ref->list[ref->index].a, "g") == 0)
		if (!test_parse_children(element, ref))
			return false;

	return check_reference(SAX_ELEMENT_OPENING, &element, NULL, arg);
}

static bool test_sax_parser()
{
	static const char * const xml =
		"<?xml version=\"1.0\"?>\n"
		"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"
		"  \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
		"<svg width='84.1mm' height='51.1mm' version=\"1.1\"\n"
		"     viewBox=\"0 0 84.1 51.1\" xmlns=\"http://www.w3.org/2000/svg\">\n"
		"  <!-- SVG paths -->\n"
		"  <path stroke=\"#fffc11\" fill=\"none\" stroke-width=\"0.2\"\n"
		"        d=\"M  30.5  23.7 L  30.5  23.7 L  30.4  24.1 L  30.3  24.5\n"
		"           L  30.3  24.9 L  29.5  25.3 L  28.9  24.3 L  29.5  23.2\n"
		"           L  30.8  23.8 L  29.6  24.9 L  28.5  24.9 L  29.0  23.8\" />\n"
		"  <path stroke=\"#fffc11\" fill=\"none\" stroke-width=\"0.2\" stroke-dasharray=\"1 1\"\n"
		"        d=\"M  40.6  26.9 L  45.9  10.5\" />\n"
		"  <path stroke=\"#fffc11\" fill=\"none\" stroke-width=\"0.2\"\n"
		"        d=\"M  45.9  10.5 L  45.9  10.5 L  45.6  10.7 L  45.2  10.9\n"
		"           L  44.9  11.1 L  44.3  11.3 L  43.3  11.9 L  41.6  12.7\n"
		"           L  43.3  11.2 L  45.4   9.9 L  47.7   8.4 L  49.4   7.4\" />\n"
		"  <g>\n"
		"    <path stroke=\"#b5dc10\" fill=\"none\" stroke-width=\"0.2\" stroke-dasharray=\"1 1\"\n"
		"          d=\"M  42.3   8.3 L  29.8  42.9\" />\n"
		"    <path stroke=\"#b5dc10\" fill=\"none\" stroke-width=\"0.2\"\n"
		"          d=\"M  29.8  42.9 L  29.8  42.9 L  29.9  42.4 L  29.9  42.1\n"
		"             L  30.0  41.6 L  29.4  41.2 L  28.8  39.8 L  28.8  37.9\" />\n"
		"  </g>\n"
		"  <path stroke=\"#b5dc10\" fill=\"none\" stroke-width=\"0.2\" stroke-dasharray=\"1 1\"\n"
		"        d=\"M  39.4  36.8 L  40.6  27.3\" />\n"
		"</svg>\n";

	static const struct sax_invocation list[] = {
		{ SAX_ELEMENT_OPENING,  4,  2, "svg" },
		{ SAX_ATTRIBUTE,  4,  6, "width", "84.1mm" },
		{ SAX_ATTRIBUTE,  4, 21, "height", "51.1mm" },
		{ SAX_ATTRIBUTE,  4, 37, "version", "1.1" },
		{ SAX_ATTRIBUTE,  5,  6, "viewBox", "0 0 84.1 51.1" },
		{ SAX_ATTRIBUTE,  5, 30, "xmlns", "http://www.w3.org/2000/svg" },
		{ SAX_ELEMENT_OPENING,  7,  4, "path" },
		{ SAX_ATTRIBUTE,  7,  9, "stroke", "#fffc11" },
		{ SAX_ATTRIBUTE,  7, 26, "fill", "none" },
		{ SAX_ATTRIBUTE,  7, 38, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE,  8,  9, "d", "M  30.5  23.7 L  30.5  23.7 L  30.4  24.1 L  30.3  24.5\n"
		                   "           L  30.3  24.9 L  29.5  25.3 L  28.9  24.3 L  29.5  23.2\n"
			           "           L  30.8  23.8 L  29.6  24.9 L  28.5  24.9 L  29.0  23.8" },
		{ SAX_ELEMENT_CLOSING,  7,  4, "path" },
		{ SAX_ELEMENT_OPENING, 11,  4, "path" },
		{ SAX_ATTRIBUTE, 11,  9, "stroke", "#fffc11" },
		{ SAX_ATTRIBUTE, 11, 26, "fill", "none" },
		{ SAX_ATTRIBUTE, 11, 38, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 11, 57, "stroke-dasharray", "1 1" },
		{ SAX_ATTRIBUTE, 12,  9, "d", "M  40.6  26.9 L  45.9  10.5" },
		{ SAX_ELEMENT_CLOSING, 11,  4, "path" },
		{ SAX_ELEMENT_OPENING, 13,  4, "path" },
		{ SAX_ATTRIBUTE, 13,  9, "stroke", "#fffc11" },
		{ SAX_ATTRIBUTE, 13, 26, "fill", "none" },
		{ SAX_ATTRIBUTE, 13, 38, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 14,  9, "d", "M  45.9  10.5 L  45.9  10.5 L  45.6  10.7 L  45.2  10.9\n"
			           "           L  44.9  11.1 L  44.3  11.3 L  43.3  11.9 L  41.6  12.7\n"
			           "           L  43.3  11.2 L  45.4   9.9 L  47.7   8.4 L  49.4   7.4" },
		{ SAX_ELEMENT_CLOSING, 13,  4, "path" },
		{ SAX_ELEMENT_OPENING, 17,  4, "g" },
		{ SAX_ELEMENT_OPENING, 18,  6, "path" },
		{ SAX_ATTRIBUTE, 18, 11, "stroke", "#b5dc10" },
		{ SAX_ATTRIBUTE, 18, 28, "fill", "none" },
		{ SAX_ATTRIBUTE, 18, 40, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 18, 59, "stroke-dasharray", "1 1" },
		{ SAX_ATTRIBUTE, 19, 11, "d", "M  42.3   8.3 L  29.8  42.9" },
		{ SAX_ELEMENT_CLOSING, 18,  6, "path" },
		{ SAX_ELEMENT_OPENING, 20,  6, "path" },
		{ SAX_ATTRIBUTE, 20, 11, "stroke", "#b5dc10" },
		{ SAX_ATTRIBUTE, 20, 28, "fill", "none" },
		{ SAX_ATTRIBUTE, 20, 40, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 21, 11, "d", "M  29.8  42.9 L  29.8  42.9 L  29.9  42.4 L  29.9  42.1\n"
			         "             L  30.0  41.6 L  29.4  41.2 L  28.8  39.8 L  28.8  37.9" },
		{ SAX_ELEMENT_CLOSING, 20,  6, "path" },
		{ SAX_ELEMENT_CLOSING, 23,  5, "g" },
		{ SAX_ELEMENT_OPENING, 24,  4, "path" },
		{ SAX_ATTRIBUTE, 24,  9, "stroke", "#b5dc10" },
		{ SAX_ATTRIBUTE, 24, 26, "fill", "none" },
		{ SAX_ATTRIBUTE, 24, 38, "stroke-width", "0.2" },
		{ SAX_ATTRIBUTE, 24, 57, "stroke-dasharray", "1 1" },
		{ SAX_ATTRIBUTE, 25,  9, "d", "M  39.4  36.8 L  40.6  27.3" },
		{ SAX_ELEMENT_CLOSING, 24,  4, "path" },
		{ SAX_ELEMENT_CLOSING, 26,  3, "svg" },
		{ 0, 0, 0, NULL }
	};

	struct sax_reference ref = { 0, list };

	TEST_ASSERT(sax_parse_text(xml, element_opening_cb, element_closing_cb,
		attribute_cb, error_cb, &ref));
	TEST_ASSERT(ref.index == 48);
	TEST_ASSERT(ref.list[ref.index].a == NULL);
	TEST_ASSERT(ref.specials == 7);

	TEST_ASSERT(sax_parse_text(xml, NULL, NULL, NULL, NULL, NULL));

	return true;
}

static bool test_sax_strcmp()
{
	static const char *s[] = { "", "0", "1", "2", "a", "b", "c",
		"01", "10", "00", "11", "09", "90", "99",
		"ab", "ba", "ac", "ca", "bc", "cb",
		"000", "123", "321", "789", "987", "090", "909",
		"abc", "cba", "cab", "bac",
		NULL
	};

	for (int i = 0; s[i] != NULL; i++)
	for (int j = 0; s[j] != NULL; j++) {
		const int cmp = strcmp(s[i], s[j]);

		for (int k = 0; s[k] != NULL; k++) {
			char buf[100];
			const struct sax_token token = {
				.row = 1,
				.column = 1,
				.index = 0,
				.length = strlen(s[i]),
				.text = buf,
				.cursor = buf,
			};

			snprintf(buf, sizeof(buf), "%s%s", s[i], s[k]);
			TEST_ASSERT(sign(sax_strcmp(token, s[j])) == sign(cmp));
		}
	}

	return true;
}

const struct test_entry test_suite_sax[] = {
	TEST_ENTRY(test_sax_parser),
	TEST_ENTRY(test_sax_strcmp),
	TEST_ENTRY(NULL)
};
