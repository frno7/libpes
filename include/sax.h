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

#ifndef PESLIB_SAX_H
#define PESLIB_SAX_H

#include <stdlib.h>

/**
 * SAX tokens reference the original XML text and are not terminated by NUL
 * at the given length, but the text and cursor pointers are guaranteed to
 * eventually terminate with NUL. Thus it is valid to access characters
 * beyond the length up to and including the terminating NUL. This principle
 * simplies use of strtod and similar functions where the validness of the
 * length can be checked afterwards.
 */
struct sax_token {
	size_t row;         /** Row of token. */
	size_t column;      /** Column of token. */
	size_t index;       /** Index to token relative to beginning of text. */
	size_t length;      /** Length of token. */
	const char *text;   /** Pointer to beginning of text. */
	const char *cursor; /** Pointer to current position of token in text. */
};

/**
 * Callback for opening elements.
 *
 * @param element Element token.
 * @param arg Argument pointer supplied to `sax_parse_text()`.
 */
typedef bool (*sax_element_opening_callback)(const struct sax_token element,
	void * const arg);

/**
 * Callback for closing elements.
 *
 * @param element Element token.
 * @param arg Argument pointer supplied to `sax_parse_text()`.
 */
typedef bool (*sax_element_closing_callback)(const struct sax_token element,
	void * const arg);

/**
 * Callback for element attributes.
 *
 * @param attribute Attribute token.
 * @param value Value token.
 * @param arg Argument pointer supplied to `sax_parse_text()`.
 */
typedef bool (*sax_attribute_callback)(const struct sax_token attribute,
	const struct sax_token value, void * const arg);

/**
 * Callback for parsing errors.
 *
 * @param error Token where error occured.
 * @param message Error message.
 * @param arg Argument pointer supplied to `sax_parse_text()`.
 */
typedef void (*sax_error_callback)(struct sax_token error,
	const char * const message, void * const arg);

/**
 * This SAX (Simple API for XML) parser is a callback driven reentrant
 * streaming XML parser. It does not use any memory allocations.
 *
 * @param text NUL terminated XML to parse.
 * @param element_opening_cb Invoked when opening elements. Ignored if NULL.
 * @param element_closing_cb Invoked when closing elements. Ignored if NULL.
 * @param attribute_cb Invoked for element attributes. Ignored if NULL.
 * @param error_cb Invoked for parsing errors. Ignored if NULL.
 * @param arg Optional argument pointer supplied to callbacks. Can be NULL.
 * @return True on successful completion, else false.
 */
bool sax_parse_text(const char * const text,
	const sax_element_opening_callback element_opening_cb,
	const sax_element_closing_callback element_closing_cb,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg);

/**
 * Parse the attributes of the given element token.
 *
 * @param element Element token.
 * @param attribute_cb Invoked for element attributes. Ignored if NULL.
 * @param error_cb Invoked for parsing errors. Ignored if NULL.
 * @param arg Optional argument pointer supplied to callbacks. Can be NULL.
 * @return True on successful completion, else false.
 */
bool sax_parse_attributes(struct sax_token element,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg);

/**
 * Parse the children of the given element token.
 *
 * @param element Element token.
 * @param element_opening_cb Invoked when opening elements. Ignored if NULL.
 * @param element_closing_cb Invoked when closing elements. Ignored if NULL.
 * @param attribute_cb Invoked for element attributes. Ignored if NULL.
 * @param error_cb Invoked for parsing errors. Ignored if NULL.
 * @param arg Optional argument pointer supplied to callbacks. Can be NULL.
 * @return True on successful completion, else false.
 */
bool sax_parse_children(struct sax_token element_token,
	const sax_element_opening_callback element_opening_cb,
	const sax_element_closing_callback element_closing_cb,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg);

/**
 * Parse the siblings of the given element token.
 *
 * @param element Element token.
 * @param element_opening_cb Invoked when opening elements. Ignored if NULL.
 * @param element_closing_cb Invoked when closing elements. Ignored if NULL.
 * @param attribute_cb Invoked for element attributes. Ignored if NULL.
 * @param error_cb Invoked for parsing errors. Ignored if NULL.
 * @param arg Optional argument pointer supplied to callbacks. Can be NULL.
 * @return True on successful completion, else false.
 */
bool sax_parse_siblings(struct sax_token element_token,
	const sax_element_opening_callback element_opening_cb,
	const sax_element_closing_callback element_closing_cb,
	const sax_attribute_callback attribute_cb,
	const sax_error_callback error_cb, void * const arg);

/**
 * Compare token with string using strcmp.
 *
 * @param token Token to compare.
 * @param s String to compare.
 * @return The equivalent of strcmp of the token and the string.
 */
int sax_strcmp(const struct sax_token token, const char * const s);

#endif /* PESLIB_SAX_H */
