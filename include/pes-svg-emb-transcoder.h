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

#ifndef PESLIB_PES_SVG_EMB_TRANSCODER_H
#define PESLIB_PES_SVG_EMB_TRANSCODER_H

#include "svg-emb-encoder.h"

/**
 * Transcode PES to SVG embroidery by sending data to the provided callback.
 *
 * @param data Pointer to PES data.
 * @param size Size of PES data in bytes.
 * @param encode_cb Callback to invoke for encoded data.
 * @param arg Optional argument pointer supplied to callback. Can be NULL.
 * @return True on successful completion, else false.
 */
bool pes_svg_emb_transcode(const void * const data, const size_t size,
	const svg_emb_encode_callback encode_cb, void * const arg);

#endif /* PESLIB_PES_SVG_EMB_TRANSCODER_H */
