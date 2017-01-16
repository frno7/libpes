# PES format description

The PES format is a [machine embroidery](https://en.wikipedia.org/wiki/Machine_embroidery) file format for [Brother Industries](https://en.wikipedia.org/wiki/Brother_Industries) and [Bernina International](https://en.wikipedia.org/wiki/Bernina_International) series of embroidery machines, among others. PES files contain sewing coordinates for stitches and corresponding thread colors.

The PEC format is embedded within the PES format, and so the same sewing coordinates are represented twice in two different formats.

## Table of contents

- [Unknown parts](#unknown-parts)
- [Basic types](#basic-types)
- [Sections](#sections)
  - [Header section](#header-section)
    - [Version 1 header section](#version-1-header-section)
    - [Version 2 header section](#version-2-header-section)
    - [Version 4 header section](#version-4-header-section)
    - [Version 5 header section](#version-5-header-section)
    - [Version 6 header section](#version-6-header-section)
    - [Description strings subsection](#description-strings-subsection)
    - [Color list subsection](#color-list-subsection)
    - [Color subsection](#color-subsection)
  - [CEmbOne section](#cembone-section)
  - [CSewSeg section](#csewseg-section)
    - [CSewSeg coordinate list subsection](#csewseg-coordinate-list-subsection)
    - [CSewSeg coordinate subsection](#csewseg-coordinate-subsection)
    - [CSewSeg color list subsection](#csewseg-color-list-subsection)
    - [CSewSeg color subsection](#csewseg-color-subsection)
  - [PEC section](#pec-section)
    - [PEC stitch list subsection](#pec-stitch-list-subsection)
    - [PEC thumbnail image subsection](#pec-thumbnail-image-subsection)
    - [PEC palette](#pec-palette)

## Unknown parts

Several parts of the PES format remain unknown and are not properly described in this document. If you happen to understand these parts and is happy to contribute, please do!

Some concepts would also need to be explained in more detail. For example under what circumstances the [affine transformation](https://en.wikipedia.org/wiki/Affine_transformation) matrix in the CEmbOne section is applied.

## Basic types

All integer and floating point numbers are stored in [little-endian](https://en.wikipedia.org/wiki/Endianness#Little-endian) byte order unless otherwise noted.

Type | Bytes |Description
--- | ---: | ---
`char` | 1 | [ASCII](https://en.wikipedia.org/wiki/ASCII) text (not `NUL` terminated)
`u8` | 1 | 8 bit unsigned integer
`u16` | 2 | 16 bit unsigned integer
`s16` | 2 | 16 bit signed integer in stitch coordinate space (`1` represents 0.1 millimeters)
`u32` | 4 | 32 bit unsigned integer
`f32` | 4 | 32 bit [single-precision floating-point number](https://en.wikipedia.org/wiki/Single-precision_floating-point_format)

## Sections

Section sizes are often variable in length and therefore many parts cannot be located at a fixed byte offset. PES versions 1 and 4 and PEC have a fixed predefined palette of 65 threads. PES versions 5 and 6 replace this with configurable threads.

Section | Description
--- | ---
Header | Format id, version, name, colors, etc.
CEmbOne | Coordinate transformation matrix, etc.
CSewSeg | Stitch sewing coordinates, etc.
PEC | Embedded PEC format

### Header section

The first 12 bytes of the header section are common to all versions.

Type | Bytes | Value | Description
--- | ---: | --- | ---
`char` | 4 | `"#PES"` | Identification
`char` | 4 | `"0001"` \| `"0040"` \| `"0050"` \| `"0060"` | Version 1, 4, 5 or 6
`u32` | 4 | | Absolute PEC section byte offset

#### Version 1 header section

Type | Bytes | Value | Description
--- | ---: | --- | ---
`char` | 4 | `"#PES"` | Identification
`char` | 4 | `"0001"` | Version 1
`u32` | 4 | | Absolute PEC section byte offset
`u16` | 2 | `0` \| `1` \| ? | Unknown
`u16` | 2 | `1` (typical) | Unknown
`u16` | 2 | `1` (typical) | Unknown
`u16` | 2 | `0xFFFF` (typical) | Unknown
`u16` | 2 | `0` (typical) | Unknown

#### Version 2 header section

Type | Bytes | Value | Description
--- | ---: | --- | ---
`char` | 4 | `"#PES"` | Identification
`char` | 4 | `"0002"` \| `"0020"` \| ? | Version 2
`u32` | 4 | | Absolute PEC section byte offset
`u16` | 2 | | Hoop width [millimeter]
`u16` | 2 | | Hoop height [millimeter]
`u16` | 2 | | Unknown, sewing area?
`u16` | 2 | | Unknown, design page background color?
`u16` | 2 | | Unknown, design page foreground color?
`u16` | 2 | | Unknown, show grid?
`u16` | 2 | | Unknown, with axes?
`u16` | 2 | | Unknown, snap to grid?
`u16` | 2 | | Unknown, grid interval?
`u16` | 2 | | Unknown, P9 curves?
`u16` | 2 | | Unknown, optimize entry-exit points?
`u16` | 2 | `0xFFFF` (typical) | Unknown
`u16` | 2 | `0` (typical) | Unknown

#### Version 4 header section

Type | Bytes | Value | Description
--- | ---: | --- | ---
`char` | 4 | `"#PES"` | Identification
`char` | 4 | `"0040"` | Version 4
`u32` | 4 | | Absolute PEC section byte offset
`u16` | 2 | `1` (typical) | Unknown
`u8` | 1 | `0x30` (typical) | Unknown
`u8` | 1 | `0x31` \| `0x32` \| ? | Unknown
`description_strings` | | | Description strings
`u8` | 2 | `0` (typical) | Unknown
`u16` | 2 | | Hoop width [millimeter]
`u16` | 2 | | Hoop height [millimeter]
`u16` | 2 | `0` (typical) | Unknown
`u16` | 2 | `7` (typical) | Unknown
`u16` | 2 | `19` (typical) | Unknown
`u16` | 2 | `0` \| `1` \| ? | Unknown
`u16` | 2 | `1` (typical) | Unknown
`u16` | 2 | `0` \| `1` \| ? | Unknown
`u16` | 2 | `100` (typical) | Unknown
`u8` | 1 | `1` (typical) | Unknown
`u8` | 7 | `0` (typical) | Unknown
`u16` | 2 | `1` (typical) | Unknown
`u16` | 2 | `0xFFFF` (typical) | Unknown
`u16` | 2 | `0` (typical) | Unknown

#### Version 5 header section

Type | Bytes | Value | Description
--- | ---: | --- | ---
`char` | 4 | `"#PES"` | Identification
`char` | 4 | `"0050"` | Version 5
`u32` | 4 | | Absolute PEC section byte offset
`u16` | 2 | `1` (typical) | Unknown
`u8` | 1 | `0x30` (typical) | Unknown
`u8` | 1 | `0x31` \| `0x32` \| ? | Unknown
`description_strings` | | | Description strings
`u8` | 2 | `0` (typical) | Unknown
`u16` | 2 | | Hoop width [millimeter]
`u16` | 2 | | Hoop height [millimeter]
`u16` | 2 | `0` (typical) | Unknown
`u16` | 2 | `7` (typical) | Unknown
`u16` | 2 | `19` (typical) | Unknown
`u16` | 2 | `0` \| `1` \| ? | Unknown
`u16` | 2 | `1` (typical) | Unknown
`u16` | 2 | `0` \| `1` \| ? | Unknown
`u16` | 2 | `100` (typical) | Unknown
`u8` | 1 | `1` (typical) | Unknown
`u8` | 4 | `0` (typical) | Unknown
`f32` | 4 | `1.0f` (typical) | Unknown
`u8` | 8 | `0` (typical) | Unknown
`f32` | 4 | `1.0f` (typical) | Unknown
`u8` | 14 | `0` (typical) | Unknown
`color_list` | | | Color list subsection
`u16` | 2 | `1` (typical) | Unknown
`u16` | 2 | `0xFFFF` (typical) | Unknown
`u16` | 2 | `0` (typical) | Unknown

#### Version 6 header section

Type | Bytes | Value | Description
--- | ---: | --- | ---
`char` | 4 | `"#PES"` | PES Identification
`char` | 4 | `"0060"` | Version 6
`u32` | 4 | | Absolute PEC section byte offset
`u16` | 2 | `1` (typical) | Unknown
`u8` | 1 | `0x30` (typical) | Unknown
`u8` | 1 | `0x31` \| `0x32` \| ? | Unknown
`description_strings` | | | Description strings
| `u8` | 4 | `0` (typical) | Unknown
`u16` | 2 | | Hoop width [millimeter]
`u16` | 2 | | Hoop height [millimeter]
`u16` | 2 | `0` (typical) | Unknown
`u16` | 2 | `200` (typical) | Unknown
`u16` | 2 | `200` (typical) | Unknown
`u16` | 2 | `100` (typical) | Unknown
`u16` | 2 | `100` (typical) | Unknown
`u16` | 2 | `100` (typical) | Unknown
`u16` | 2 | `7` (typical) | Unknown
`u16` | 2 | `19` (typical) | Unknown
`u16` | 2 | `0` \| `1` \| ? | Unknown
`u16` | 2 | `1` (typical) | Unknown
`u16` | 2 | `0` (typical) | Unknown
`u16` | 2 | `15` \| `100` \| ? | Unknown
`u8` | 1 | `1` (typical) | Unknown
`u8` | 4 | `0` (typical) | Unknown
`f32` | 4 | `1.0f` (typical) | Unknown
`u8` | 8 | `0` (typical) | Unknown
`f32` | 4 | `1.0f` (typical) | Unknown
`u8` | 14 | `0` (typical) | Unknown
`color_list` | | | Color list subsection
`u16` | 2 | `1` (typical) | Unknown
`u16` | 2 | `0xFFFF` (typical) | Unknown
`u16` | 2 | `0` (typical) | Unknown

#### Description strings subsection

Type | Bytes | Value | Description
--- | ---: | --- | ---
`u8` | 1 | S1 | Length of following string
`char` | S1 | | Design string
`u8` | 1 | S2 | Length of following string
`char` | S2 | | Category string
`u8` | 1 | S3 | Length of following string
`char` | S3 | | Author string
`u8` | 1 | S4 | Length of following string
`char` | S4 | | Keywords string
`u8` | 1 | S5 | Length of following string
`char` | S5 | | Comments string

#### Color list subsection

Type | Bytes | Value | Description
--- | ---: | --- | ---
`u16` | 2 | | Number of following color subsections
`color` | | | Color subsections

#### Color subsection

Type | Bytes | Value | Description
--- | ---: | --- | ---
`u8` | 1 | S1 | Length of following string
`char` | S1 | `"336"` \| `"575"` \| etc. | Color code
`u8` | 1 | `0 ` – `255` | Red component of [RGB color model](https://en.wikipedia.org/wiki/RGB_color_model)
`u8` | 1 | `0 ` – `255` | Green component of [RGB color model](https://en.wikipedia.org/wiki/RGB_color_model)
`u8` | 1 | `0 ` – `255` | Blue component of [RGB color model](https://en.wikipedia.org/wiki/RGB_color_model)
`u8` | 1 | `0` (typical) | Unknown
`u8` | 1 | `0xA` \| `0xB` \| ? | Color type?
`u8` | 3 | `0` (typical) | Unknown
`u8` | 1 | S2 | Length of following string
`char` | S2 | `"17"` \| `"21"` \| etc. | PEC palette index?
`u8` | 1 | S3 | Length of following string
`char` | S3 | `"Isacord 40"` \| etc. | Thread type
`u8` | 1 | `0` (typical) | Unknown

### CEmbOne section

The CEmbOne section is always 73 bytes in size.

Type | Bytes | Value | Description
--- | ---: | --- | ---
`u16` | 2 | `7` | Length of following string
`char` | 7 | `"CEmbOne"` | CEmbOne identification
`s16` | 2 | | Minimum x coordinate in CSewSeg section
`s16` | 2 | | Minimum y coordinate in CSewSeg section
`s16` | 2 | | Maximum x coordinate in CSewSeg section
`s16` | 2 | | Maximum y coordinate in CSewSeg section
`s16` | 2 | | Minimum x coordinate in CSewSeg section (repeated?)
`s16` | 2 | | Minimum y coordinate in CSewSeg section (repeated?)
`s16` | 2 | | Maximum x coordinate in CSewSeg section (repeated?)
`s16` | 2 | | Maximum y coordinate in CSewSeg section (repeated?)
`f32` | 4 | `1.0f` (identity) | Row 1 column 1 of [affine transformation](https://en.wikipedia.org/wiki/Affine_transformation) matrix
`f32` | 4 | `0.0f` (identity) | Row 1 column 2 of [affine transformation](https://en.wikipedia.org/wiki/Affine_transformation) matrix
`f32` | 4 | `0.0f` (identity) | Row 2 column 1 of [affine transformation](https://en.wikipedia.org/wiki/Affine_transformation) matrix
`f32` | 4 | `1.0f` (identity) | Row 2 column 2 of [affine transformation](https://en.wikipedia.org/wiki/Affine_transformation) matrix
`f32` | 4 | `0.0f` (identity) | Row 3 column 1 of [affine transformation](https://en.wikipedia.org/wiki/Affine_transformation) matrix
`f32` | 4 | `0.0f` (identity) | Row 3 column 2 of [affine transformation](https://en.wikipedia.org/wiki/Affine_transformation) matrix
`u16` | 2 | `1` (typical) | Unknown
`s16` | 2 | | CSewSeg x coordinate translation?
`s16` | 2 | | CSewSeg x coordinate translation?
`s16` | 2 | | CSewSeg width
`s16` | 2 | | CSewSeg height
`u8` | 8 | `0` (typical) | Unknown
`u16` | 2 | | CSewSeg block count
`u16` | 2 | `0xFFFF` (typical) | Unknown
`u16` | 2 | `0` (typical) | Unknown

### CSewSeg section

Type | Bytes | Value | Description
--- | ---: | --- | ---
`u16` | 2 | `7` | Length of following string
`char` | 7 | `"CSewSeg"` | CSewSeg identification
`csewseg_stitch_list` | | | CSewSeg stitch list subsection
`csewseg_color_list` | | | CSewSeg color list subsection

#### CSewSeg stitch list subsection

The CSewSeg stitch list subsection is divided into stitch blocks. The last block does not end with the continuation code `0x8003` and so is 2 bytes shorter than the others. Some files seems to limit the number of stitches to a maximum 1000 per block, is this required?

Type | Bytes | Value | Description
--- | ---: | --- | ---
`u16` | 2 | `0` \| `1` \| ? | Stitch type where `0` means a normal stitch and `1` means a jump stitch
`u16` | 2 | | Thread index for block + 1
`u16` | 2 | N1 | Number of following coordinates
`csewseg_coordinates` | 4 × N1 | | CSewSeg coordinate subsection
`u16` | 2 | `0x8003` \| ? | Continuation code where `0x8003` means list continues with another following block, with the last block not having this field at all

#### CSewSeg coordinate subsection

Type | Bytes | Value | Description
--- | ---: | --- | ---
`s16` | 2 | | Stitch x coordinate
`s16` | 2 | | Stitch y coordinate

#### CSewSeg color list subsection

Type | Bytes | Value | Description
--- | ---: | --- | ---
`u16` | 2 | N1 | Number of following colors
`csewseg_color` | 4 × N1 | | CSewSeg color subsection
`u16` | 2 | `0` (typical) | Unknown
`u16` | 2 | `0` (typical) | Unknown

#### CSewSeg color subsection

Type | Bytes | Value | Description
--- | ---: | --- | ---
`u16` | 2 | | Block index where change of thread takes effect, starting from zero
`u16` | 2 | | In PES versions 1 and 4 this is a thread palette index and in versions 5 and 6 this is a thread index

### PEC section

The first part of the PEC section is 512 bytes.

Type | Bytes | Value | Description
--- | ---: | --- | ---
`char` | 19 | | Label string prefixed with `"LA:"` and padded with space (`0x20`)
`char` | 1 | `'\r'` | Carrige return character
`u8` | 11 | `0x20` (typical) | Unknown
`u8` | 1 | `0x20` \| `0xFF` \| ? | Unknown
`u16` | 2 | `0x00FF` (typical) | Unknown
`u8` | 1 | `6` (typical) | Thumbnail image width in bytes, with 8 bit pixels per byte, so `6` would mean 6×8 = 48 pixels per line
`u8` | 1 | `38` (typical) | Thumbnail image height in pixels
`u8` | 12 | `0x00` \| `0x20` \| `0x64` \| `0xFF` \| ? | Unknown
`u8` | 1 | N1 | Number of colors minus one
`u8` | 1 + N1 | | Palette index
`u8` | 462 - N1 | `0x20` (typical) | Unknown

Then follows the second part of the PEC section.

Type | Bytes | Value | Description
--- | ---: | --- | ---
`u16` | 2 | `0x0000` (typical) | Unknown
`u16` | 2 | | Offset to thumbnail image subsection relative to the PEC section offset plus 512 bytes
`u16` | 2 | `0x3100` (typical) | Unknown
`u16` | 2 | `0xF0FF` (typical) | Unknown
`s16` | 2 | | Width
`s16` | 2 | | Height
`u16` | 2 | `0x01E0` (typical) | Unknown
`u16` | 2 | `0x01B0` (typical) | Unknown
| | 4 | | Unknown
`pec_stitch_list` | | | PEC stitch list subsection

#### PEC stitch list subsection

The PEC stitch list come in dx and dy relative coordinate pairs. Depending on the bit patterns used, 2, 3 or 4 bytes are used for the coordinate pairs as explained below. Note that if the first coordinates are (0, 0) the embroidery machine apparently ignores them. The first nonzero coordinate pair is relative to the minimum bound coordinates.

Type | Bytes | Value | Description
--- | ---: | --- | ---
`u8` | 1 | | Stitch dx coordinate
`u8` | 1 | | Stitch dy coordinate

Some combined dx and dy values have special meaning. After a stop stitch, an alternating byte of 2 and 1 is encoded, starting with 2. The end of the stitch list is coded with `0xFF` for dx but no value for dy. To encode a stop stitch, dx is `0xFE`.

If the most significant bit is cleared but dx or dy is greater than `0x3F`, then their value is subtracted by `0x80` to yield a negative value, respectively.

If the most significant bit is set in dx, the following is computed. If bit 6 is set the stitch is a trim stitch, or if bit 5 is set then the stitch is a jump stitch. The 4 least significant bits in dx are masked and multiplied by 256 and dy is added to dx to yield a 12 bit dx. If dx is greater than `0x7FF` then the value is subtracted by `0x1000` to yield a negative value. A following `u8` byte is used for a new dy value.

If the most significant bit is set in dy, the following is computed. If bit 6 is set the stitch is a trim stitch, or if bit 5 is set then the stitch is a jump stitch. The 4 least significant bits in dy are masked and multiplied by 256 and the following `u8` byte is added to dy to yield a 12 bit dy. If dy is greater than `0x7FF` then the value is subtracted by `0x1000` to yield a negative value.

#### PEC thumbnail image subsection

There is always one main thumbnail image plus one for each color. The size of each thumbnail is the thumbnail width multiplied with the thumbnail height number of bytes (width and height are given in the PEC section). The pixels are oriented from the top left corner, with one pixel per bit starting with the most significant bit in the first byte.

### PEC thread palette

The following 64 threads have been verified with a Brother Innovis 955 sewing and embroidery machine and its “EMBROIDERY” thread space.

Index | Code | Name | Type | RGB color
---: | --- | --- | --- | ---
 1 | 007 | Prussian Blue     | A | #1a0a94
 2 | 000 | Blue              | A | #0f75ff
 3 | 534 | Teal Green        | A | #00934c
 4 | 070 | Corn Flower Blue  | A | #babdfe
 5 | 800 | Red               | A | #ec0000
 6 | 000 | Reddish Brown     | A | #e4995a
 7 | 620 | Magenta           | A | #cc48ab
 8 | 810 | Light Lilac       | A | #fdc4fa
 9 | 000 | Lilac             | A | #dd84cd
10 | 502 | Mint Green        | A | #6bd38a
11 | 214 | Deep Gold         | A | #e4a945
12 | 208 | Orange            | A | #ffbd42
13 | 000 | Yellow            | A | #ffe600
14 | 513 | Lime Green        | A | #6cd900
15 | 328 | Brass             | A | #c1a941
16 | 005 | Silver            | A | #b5ad97
17 | 000 | Russet Brown      | A | #ba9c5f
18 | 000 | Cream Brown       | A | #faf59e
19 | 704 | Pewter            | A | #808080
20 | 900 | Black             | A | #000000
21 | 000 | Ultramarine       | A | #001cdf
22 | 000 | Royal Purple      | A | #df00b8
23 | 707 | Dark Gray         | A | #626262
24 | 058 | Dark Brown        | A | #69260d
25 | 086 | Deep Rose         | A | #ff0060
26 | 323 | Light Brown       | A | #bf8200
27 | 079 | Salmon Pink       | A | #f39178
28 | 000 | Vermilion         | A | #ff6805
29 | 001 | White             | A | #f0f0f0
30 | 000 | Violet            | A | #c832cd
31 | 000 | Seacrest          | A | #b0bf9b
32 | 019 | Sky Blue          | A | #65bfeb
33 | 000 | Pumpkin           | A | #ffba04
34 | 010 | Cream Yellow      | A | #fff06c
35 | 000 | Khaki             | A | #feca15
36 | 000 | Clay Brown        | A | #f38101
37 | 000 | Leaf Green        | A | #37a923
38 | 405 | Peacock Blue      | A | #23465f
39 | 000 | Gray              | A | #a6a695
40 | 000 | Warm Gray         | A | #cebfa6
41 | 000 | Dark Olive        | A | #96aa02
42 | 307 | Linen             | A | #ffe3c6
43 | 000 | Pink              | A | #ff99d7
44 | 000 | Deep Green        | A | #007004
45 | 000 | Lavender          | A | #edccfb
46 | 000 | Wisteria Violet   | A | #c089d8
47 | 843 | Beige             | A | #e7d9b4
48 | 000 | Carmine           | A | #e90e86
49 | 000 | Amber Red         | A | #cf6829
50 | 000 | Olive Green       | A | #408615
51 | 107 | Dark Fuchsia      | A | #db1797
52 | 209 | Tangerine         | A | #ffa704
53 | 017 | Light Blue        | A | #b9ffff
54 | 507 | Emerald Green     | A | #228927
55 | 614 | Purple            | A | #b612cd
56 | 515 | Moss Green        | A | #00aa00
57 | 124 | Flesh Pink        | A | #fea9dc
58 | 000 | Harvest Gold      | A | #fed510
59 | 000 | Electric Blue     | A | #0097df
60 | 205 | Lemon Yellow      | A | #ffff84
61 | 027 | Fresh Green       | A | #cfe774
62 | 000 | Applique Material | A | #ffc864
63 | 000 | Applique Position | A | #ffc8c8
64 | 000 | Applique          | A | #ffc8c8
