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

#include <stddef.h>

#include "pec.h"

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

/*
 * The following 64 threads have been verified with a Brother Innovis 955
 * sewing and embroidery machine and its "EMBROIDERY" thread space.
 */
static const struct pec_thread palette_thread_list[] = {
	{  1,  "1", "000", "Prussian Blue",     "A", {  26,  10, 148 } },
	{  2,  "2", "000", "Blue",              "A", {  15, 117, 255 } },
	{  3,  "3", "000", "Teal Green",        "A", {   0, 147,  76 } },
	{  4,  "4", "000", "Corn Flower Blue",  "A", { 186, 189, 254 } },
	{  5,  "5", "000", "Red",               "A", { 236,   0,   0 } },
	{  6,  "6", "000", "Reddish Brown",     "A", { 228, 153,  90 } },
	{  7,  "7", "000", "Magenta",           "A", { 204,  72, 171 } },
	{  8,  "8", "000", "Light Lilac",       "A", { 253, 196, 250 } },
	{  9,  "9", "000", "Lilac",             "A", { 221, 132, 205 } },
	{ 10, "10", "000", "Mint Green",        "A", { 107, 211, 138 } },
	{ 11, "11", "000", "Deep Gold",         "A", { 228, 169,  69 } },
	{ 12, "12", "000", "Orange",            "A", { 255, 189,  66 } },
	{ 13, "13", "000", "Yellow",            "A", { 255, 230,   0 } },
	{ 14, "14", "000", "Lime Green",        "A", { 108, 217,   0 } },
	{ 15, "15", "000", "Brass",             "A", { 193, 169,  65 } },
	{ 16, "16", "000", "Silver",            "A", { 181, 173, 151 } },
	{ 17, "17", "000", "Russet Brown",      "A", { 186, 156,  95 } },
	{ 18, "18", "000", "Cream Brown",       "A", { 250, 245, 158 } },
	{ 19, "19", "000", "Pewter",            "A", { 128, 128, 128 } },
	{ 20, "20", "000", "Black",             "A", {   0,   0,   0 } },
	{ 21, "21", "000", "Ultramarine",       "A", {   0,  28, 223 } },
	{ 22, "22", "000", "Royal Purple",      "A", { 223,   0, 184 } },
	{ 23, "23", "000", "Dark Gray",         "A", {  98,  98,  98 } },
	{ 24, "24", "000", "Dark Brown",        "A", { 105,  38,  13 } },
	{ 25, "25", "000", "Deep Rose",         "A", { 255,   0,  96 } },
	{ 26, "26", "000", "Light Brown",       "A", { 191, 130,   0 } },
	{ 27, "27", "000", "Salmon Pink",       "A", { 243, 145, 120 } },
	{ 28, "28", "000", "Vermillion",        "A", { 255, 104,   5 } },
	{ 29, "29", "000", "White",             "A", { 240, 240, 240 } },
	{ 30, "30", "000", "Violet",            "A", { 200,  50, 205 } },
	{ 31, "31", "000", "Seacrest",          "A", { 176, 191, 155 } },
	{ 32, "32", "000", "Sky Blue",          "A", { 101, 191, 235 } },
	{ 33, "33", "000", "Pumpkin",           "A", { 255, 186,   4 } },
	{ 34, "34", "000", "Cream Yellow",      "A", { 255, 240, 108 } },
	{ 35, "35", "000", "Khaki",             "A", { 254, 202,  21 } },
	{ 36, "36", "000", "Clay Brown",        "A", { 243, 129,   1 } },
	{ 37, "37", "000", "Leaf Green",        "A", {  55, 169,  35 } },
	{ 38, "38", "000", "Peacock Blue",      "A", {  35,  70,  95 } },
	{ 39, "39", "000", "Gray",              "A", { 166, 166, 149 } },
	{ 40, "40", "000", "Warm Gray",         "A", { 206, 191, 166 } },
	{ 41, "41", "000", "Dark Olive",        "A", { 150, 170,   2 } },
	{ 42, "42", "000", "Linen",             "A", { 255, 227, 198 } },
	{ 43, "43", "000", "Pink",              "A", { 255, 153, 215 } },
	{ 44, "44", "000", "Deep Green",        "A", {   0, 112,   4 } },
	{ 45, "45", "000", "Lavender",          "A", { 237, 204, 251 } },
	{ 46, "46", "000", "Wisteria Violet",   "A", { 192, 137, 216 } },
	{ 47, "47", "000", "Beige",             "A", { 231, 217, 180 } },
	{ 48, "48", "000", "Carmine",           "A", { 233,  14, 134 } },
	{ 49, "49", "000", "Amber Red",         "A", { 207, 104,  41 } },
	{ 50, "50", "000", "Olive Green",       "A", {  64, 134,  21 } },
	{ 51, "51", "000", "Dark Fuschia",      "A", { 219,  23, 151 } },
	{ 52, "52", "000", "Tangerine",         "A", { 255, 167,   4 } },
	{ 53, "53", "000", "Light Blue",        "A", { 185, 255, 255 } },
	{ 54, "54", "000", "Emerald Green",     "A", {  34, 137,  39 } },
	{ 55, "55", "000", "Purple",            "A", { 182,  18, 205 } },
	{ 56, "56", "000", "Moss Green",        "A", {   0, 170,   0 } },
	{ 57, "57", "000", "Flesh Pink",        "A", { 254, 169, 220 } },
	{ 58, "58", "000", "Harvest Gold",      "A", { 254, 213,  16 } },
	{ 59, "59", "000", "Electric Blue",     "A", {   0, 151, 223 } },
	{ 60, "60", "000", "Lemon Yellow",      "A", { 255, 255, 132 } },
	{ 61, "61", "000", "Fresh Green",       "A", { 207, 231, 116 } },
	{ 62, "62", "000", "Applique Material", "A", { 255, 200, 100 } },
	{ 63, "63", "000", "Applique Position", "A", { 255, 200, 200 } },
	{ 64, "64", "000", "Applique",          "A", { 255, 200, 200 } }
};

#define PEC_PALETTE_COUNT COUNT_OF(palette_thread_list)

static int rgb_norm2(const struct pec_rgb u, const struct pec_rgb v)
{
	const int dr = v.r - u.r;
	const int dg = v.g - u.g;
	const int db = v.b - u.b;

	return dr*dr + dg*dg + db*db;
}

struct pec_thread pec_palette_thread(const int palette_index)
{
	return 1 <= palette_index && palette_index <= PEC_PALETTE_COUNT ?
	       palette_thread_list[palette_index - 1] : pec_undefined_thread();
}

int pec_palette_index_by_rgb(const struct pec_rgb rgb)
{
	int palette_index = 0;

	for (int i = 0; i < PEC_PALETTE_COUNT; i++)
		if (rgb_norm2(rgb, pec_palette_thread(i).rgb) <
		    rgb_norm2(rgb, pec_palette_thread(palette_index).rgb))
		    palette_index = i;

	return palette_index;
}

const struct pec_thread pec_undefined_thread()
{
	return (struct pec_thread)
		{ 0, "00", "000", "Undefined", "A", { 220,  220, 220 } };
}
