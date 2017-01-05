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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sax.h"

struct sax_state {
	int level;
};

static struct sax_token next_char(struct sax_token t)
{
	if (t.cursor[0] == '\n') {
		t.row++;
		t.column = 1;
		t.index++;
		t.length = 1;
	} else if (t.cursor[0] != '\0') {
		t.column++;
		t.index++;
		t.length = 1;
	}

	if (t.text[t.index] == '\0')
		t.length = 0;
	t.cursor = &t.text[t.index];

	return t;
}

static bool update_continuation(struct sax_token t, struct sax_token * const c,
	const bool valid)
{
	if (c != NULL)
		*c = t;

	return valid;
}

static bool parse_error(struct sax_token t, struct sax_token * const c,
	const char * const message, const sax_error_callback error_cb,
	void * const arg)
{
	if (error_cb != NULL)
		error_cb(t, message, arg);

	return update_continuation(t, c, false);
}

static bool valid_name_char(const char c)
{
	return c != '\0' && !isspace(c) && c != '=' && c != '/' && c != '>';
}

static struct sax_token parse_name(struct sax_token t, struct sax_token * const c)
{
	struct sax_token name = t;

	for (name.length = 0; valid_name_char(t.cursor[0]); t = next_char(t))
		name.length++;

	update_continuation(t, c, true);

	return name;
}

static struct sax_token parse_attribute_value(struct sax_token t,
	struct sax_token * const c, const char q)
{
	struct sax_token value = t;

	for (value.length = 0; t.cursor[0] != '\0' && t.cursor[0] != q; t = next_char(t))
		value.length++;

	update_continuation(t, c, true);

	return value;
}

static bool element_opening(const struct sax_token name,
	struct sax_state * const state,
	const sax_element_opening_callback element_opening_cb, void * const arg)
{
	state->level++;

	return element_opening_cb != NULL ? element_opening_cb(name, arg) : true;
}

static bool element_closing(const struct sax_token name,
	struct sax_state * const state,
	const sax_element_closing_callback element_closing_cb, void * const arg)
{
	const bool valid = element_closing_cb != NULL && 0 < state->level ?
		element_closing_cb(name, arg) : true;

	state->level--;

	return valid;
}

static bool parse_element_closing(struct sax_token t, struct sax_token * const c,
	struct sax_state * const state,
	const sax_element_closing_callback element_closing_cb,
	const sax_error_callback error_cb, void * const arg)
{
	const struct sax_token name = parse_name(t, &t);

	while (isspace(t.cursor[0]))
		t = next_char(t);
	if (t.cursor[0] != '>')
		return parse_error(t, c, "Expected '>'", error_cb, arg);
	update_continuation(next_char(t), c, true);

	return element_closing(name, state, element_closing_cb, arg);
}

static bool parse_attribute(struct sax_token t, struct sax_token * const c,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg)
{
	const struct sax_token name = parse_name(t, &t);

	if (t.cursor[0] != '=')
		return parse_error(t, c, "Expected '='", error_cb, arg);
	t = next_char(t);

	const char q = t.cursor[0];

	if (q != '\'' && q != '"')
		return parse_error(t, c, "Expected ' or \"", error_cb, arg);
	t = next_char(t);

	const struct sax_token value = parse_attribute_value(t, &t, q);

	if (t.cursor[0] != q)
		return parse_error(t, c, "Expected ' or \"", error_cb, arg);
	t = next_char(t);

	if (attribute_cb != NULL)
		if (!attribute_cb(name, value, arg))
			return update_continuation(t, c, false);

	return update_continuation(t, c, true);
}

static bool parse_attribute_list(struct sax_token t, struct sax_token * const c,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg)
{
	for (;;) {
		while (isspace(t.cursor[0]))
			t = next_char(t);

		if (t.cursor[0] == '/' || t.cursor[0] == '>')
			break;

		if (!parse_attribute(t, &t, attribute_cb, error_cb, arg))
			return update_continuation(t, c, false);
	}

	return update_continuation(t, c, true);
}

static bool parse_element_opening(struct sax_token t, struct sax_token * const c,
	struct sax_state * const state,
	const sax_element_opening_callback element_opening_cb,
	const sax_element_closing_callback element_closing_cb,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg)
{
	const struct sax_token name = parse_name(t, &t);

	if (!element_opening(name, state, element_opening_cb, arg))
		return update_continuation(t, c, false);

	if (!parse_attribute_list(t, &t, attribute_cb, error_cb, arg))
		return update_continuation(t, c, false);

	if (t.cursor[0] == '/') {
		t = next_char(t);
		if (!element_closing(name, state, element_closing_cb, arg))
			return update_continuation(t, c, false);
	}

	if (t.cursor[0] != '>')
		return parse_error(t, c, "Expected '>'", error_cb, arg);

	return update_continuation(next_char(t), c, true);
}

static bool parse_comment(struct sax_token t, struct sax_token * const c,
	const sax_error_callback error_cb, void * const arg)
{
	for (; t.cursor[0] != '\0'; t = next_char(t))
		if (t.cursor[0] == '-' && t.cursor[1] == '-' && t.cursor[2] == '>')
			return update_continuation(
				next_char(next_char(next_char(t))), c, true);

	return parse_error(t, c, "Unexpected end in comment", error_cb, arg);
}

static bool parse_declaration(struct sax_token t, struct sax_token * const c,
	const sax_error_callback error_cb, void * const arg)
{
	for (; t.cursor[0] != '\0'; t = next_char(t))
		if (t.cursor[0] == '>')
			return update_continuation(next_char(t), c, true);

	return parse_error(t, c, "Unexpected end in declaration", error_cb, arg);
}

static bool parse_processing(struct sax_token t, struct sax_token * const c,
	const sax_error_callback error_cb, void * const arg)
{
	for (; t.cursor[0] != '\0'; t = next_char(t))
		if (t.cursor[0] == '?' && t.cursor[1] == '>')
			return update_continuation(next_char(next_char(t)), c, true);

	return parse_error(t, c, "Unexpected end in processing instruction",
		error_cb, arg);
}

static bool parse_element(struct sax_token t, struct sax_token * const c,
	struct sax_state * const state,
	const sax_element_opening_callback element_opening_cb,
	const sax_element_closing_callback element_closing_cb,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg)
{
	if (t.cursor[0] == '/')
		return parse_element_closing(next_char(t), c, state,
			element_closing_cb, error_cb, arg);

	if (t.cursor[0] == '!') {
		t = next_char(t);

		if (t.cursor[0] == '-' && t.cursor[1] == '-')
			return parse_comment(next_char(next_char(t)), c,
				error_cb, arg);

		return parse_declaration(t, c, error_cb, arg);
	}

	if (t.cursor[0] == '?')
		return parse_processing(next_char(t), c, error_cb, arg);

	return parse_element_opening(t, c, state, element_opening_cb,
		element_closing_cb, attribute_cb, error_cb, arg);
}

static bool parse_children(struct sax_token t, struct sax_token * const c,
	struct sax_state * const state,
	const sax_element_opening_callback element_opening_cb,
	const sax_element_closing_callback element_closing_cb,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg)
{
	int element_count = 0;

	while (t.cursor[0] != '\0' && (element_count == 0 || 0 <= state->level))
		if (t.cursor[0] == '<') {
			t = next_char(t);
			element_count++;
			if (!parse_element(t, &t, state, element_opening_cb,
				element_closing_cb, attribute_cb, error_cb, arg))
				return update_continuation(t, c, false);
		} else if (isspace(t.cursor[0]))
			t = next_char(t);
		else
			return parse_error(t, c, "Unrecognized character",
				error_cb, arg);

	return update_continuation(t, c, true);
}

static bool parse_text(struct sax_token t, struct sax_token * const c,
	struct sax_state * const state,
	const sax_element_opening_callback element_opening_cb,
	const sax_element_closing_callback element_closing_cb,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg)
{
	if (!parse_children(t, &t, state, element_opening_cb,
		element_closing_cb, attribute_cb, error_cb, arg))
		return update_continuation(t, c, false);

	return update_continuation(t, c, true);
}

static bool element_closed(const struct sax_token element,
	void * const arg)
{
	bool * const closed = arg;

	*closed = true;

	return true;
}

bool sax_parse_text(const char * const text,
	const sax_element_opening_callback element_opening_cb,
	const sax_element_closing_callback element_closing_cb,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg)
{
	const struct sax_token t = { .row = 1, .column = 1, .index = 0,
		.length = 1, .text = text, .cursor = text };
	struct sax_state state = { };

	return parse_text(t, NULL, &state, element_opening_cb,
		element_closing_cb, attribute_cb, error_cb, arg);
}

bool sax_parse_attributes(struct sax_token element_token,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg)
{
	parse_name(element_token, &element_token);
	return parse_attribute_list(element_token, NULL,
		attribute_cb, error_cb, arg);
}

bool sax_parse_children(struct sax_token element_token,
	const sax_element_opening_callback element_opening_cb,
	const sax_element_closing_callback element_closing_cb,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg)
{
	struct sax_state state = { };
	bool closed = false;

	if (!parse_element(element_token, &element_token, &state,
		NULL, element_closed, NULL, error_cb, &closed))
		return false;
	if (closed)
		return true;
	state.level--;

	return parse_children(element_token, NULL, &state, element_opening_cb,
		element_closing_cb, attribute_cb, error_cb, arg);
}

bool sax_parse_siblings(struct sax_token element_token,
	const sax_element_opening_callback element_opening_cb,
	const sax_element_closing_callback element_closing_cb,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg)
{
	struct sax_state state = { };
	bool closed = false;

	if (!parse_element(element_token, &element_token, &state,
		NULL, element_closed, NULL, error_cb, &closed))
		return false;

	if (!closed) {
		state.level--;

		if (!parse_children(element_token, &element_token, &state,
			NULL, NULL, NULL, error_cb, arg))
			return false;

		state.level++;
	}

	return parse_text(element_token, NULL, &state, element_opening_cb,
		element_closing_cb, attribute_cb, error_cb, arg);
}

int sax_strcmp(const struct sax_token token, const char * const s)
{
	const size_t length = strlen(s);
	const int cmp = strncmp(token.cursor, s, token.length);

	return length <= token.length ? cmp : cmp == 0 ?  -1 : cmp;
}
