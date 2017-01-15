# PES format description

The PES format is a [machine embroidery](https://en.wikipedia.org/wiki/Machine_embroidery) file format for [Brother Industries](https://en.wikipedia.org/wiki/Brother_Industries) and [Bernina International](https://en.wikipedia.org/wiki/Bernina_International) series of embroidery machines, among others. PES files contain sewing coordinates for stitches and corresponding thread colors.

The PEC format is embedded within the PES format, and so the same sewing coordinates are represented twice in two different formats.

## Table of contents

- [Unknown parts](#unknown-parts)
- [Basic types](#basic-types)
- [Sections](#sections)
  - [Header section](#header-section)
    - [Version 1 header section](#version-1-header-section)
    - [Version 4 header section](#version-4-header-section)
    - [Version 5 header section](#version-5-header-section)
    - [Version 6 header section](#version-6-header-section)
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

#### Version 4 header section

Type | Bytes | Value | Description
--- | ---: | --- | ---
`char` | 4 | `"#PES"` | Identification
`char` | 4 | `"0040"` | Version 4
`u32` | 4 | | Absolute PEC section byte offset
`u16` | 2 | `1` (typical) | Unknown
`u8` | 1 | `0x30` (typical) | Unknown
`u8` | 1 | `0x31` \| `0x32` \| ? | Unknown
`u8` | 1 | S1 | Length of following string
`char` | S1 | | Description string
| `u8` | 6 | `0` (typical) | Unknown
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
`u8` | 1 | S1 | Length of following string
`char` | S1 | | Description string
| `u8` | 6 | `0` (typical) | Unknown
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
`u8` | 1 | S1 | Length of following string
`char` | S1 | | Description string
| `u8` | 8 | `0` (typical) | Unknown
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

<table>
<tr><th>Index</th><th>Code</th><th>Name</th><th>Type</th><th>RGB color</th><th>Color</th>
<tr><td style="text-align: right"> 1</td><td>007</td><td>Prussian Blue    </td><td>A</td><td>#1a0a94</td><td style="background-color: #1a0a94"></td></tr>
<tr><td style="text-align: right"> 2</td><td>000</td><td>Blue             </td><td>A</td><td>#0f75ff</td><td style="background-color: #0f75ff"></td></tr>
<tr><td style="text-align: right"> 3</td><td>534</td><td>Teal Green       </td><td>A</td><td>#00934c</td><td style="background-color: #00934c"></td></tr>
<tr><td style="text-align: right"> 4</td><td>070</td><td>Corn Flower Blue </td><td>A</td><td>#babdfe</td><td style="background-color: #babdfe"></td></tr>
<tr><td style="text-align: right"> 5</td><td>800</td><td>Red              </td><td>A</td><td>#ec0000</td><td style="background-color: #ec0000"></td></tr>
<tr><td style="text-align: right"> 6</td><td>000</td><td>Reddish Brown    </td><td>A</td><td>#e4995a</td><td style="background-color: #e4995a"></td></tr>
<tr><td style="text-align: right"> 7</td><td>620</td><td>Magenta          </td><td>A</td><td>#cc48ab</td><td style="background-color: #cc48ab"></td></tr>
<tr><td style="text-align: right"> 8</td><td>810</td><td>Light Lilac      </td><td>A</td><td>#fdc4fa</td><td style="background-color: #fdc4fa"></td></tr>
<tr><td style="text-align: right"> 9</td><td>000</td><td>Lilac            </td><td>A</td><td>#dd84cd</td><td style="background-color: #dd84cd"></td></tr>
<tr><td style="text-align: right">10</td><td>502</td><td>Mint Green       </td><td>A</td><td>#6bd38a</td><td style="background-color: #6bd38a"></td></tr>
<tr><td style="text-align: right">11</td><td>214</td><td>Deep Gold        </td><td>A</td><td>#e4a945</td><td style="background-color: #e4a945"></td></tr>
<tr><td style="text-align: right">12</td><td>208</td><td>Orange           </td><td>A</td><td>#ffbd42</td><td style="background-color: #ffbd42"></td></tr>
<tr><td style="text-align: right">13</td><td>000</td><td>Yellow           </td><td>A</td><td>#ffe600</td><td style="background-color: #ffe600"></td></tr>
<tr><td style="text-align: right">14</td><td>513</td><td>Lime Green       </td><td>A</td><td>#6cd900</td><td style="background-color: #6cd900"></td></tr>
<tr><td style="text-align: right">15</td><td>328</td><td>Brass            </td><td>A</td><td>#c1a941</td><td style="background-color: #c1a941"></td></tr>
<tr><td style="text-align: right">16</td><td>005</td><td>Silver           </td><td>A</td><td>#b5ad97</td><td style="background-color: #b5ad97"></td></tr>
<tr><td style="text-align: right">17</td><td>000</td><td>Russet Brown     </td><td>A</td><td>#ba9c5f</td><td style="background-color: #ba9c5f"></td></tr>
<tr><td style="text-align: right">18</td><td>000</td><td>Cream Brown      </td><td>A</td><td>#faf59e</td><td style="background-color: #faf59e"></td></tr>
<tr><td style="text-align: right">19</td><td>704</td><td>Pewter           </td><td>A</td><td>#808080</td><td style="background-color: #808080"></td></tr>
<tr><td style="text-align: right">20</td><td>900</td><td>Black            </td><td>A</td><td>#000000</td><td style="background-color: #000000"></td></tr>
<tr><td style="text-align: right">21</td><td>000</td><td>Ultramarine      </td><td>A</td><td>#001cdf</td><td style="background-color: #001cdf"></td></tr>
<tr><td style="text-align: right">22</td><td>000</td><td>Royal Purple     </td><td>A</td><td>#df00b8</td><td style="background-color: #df00b8"></td></tr>
<tr><td style="text-align: right">23</td><td>707</td><td>Dark Gray        </td><td>A</td><td>#626262</td><td style="background-color: #626262"></td></tr>
<tr><td style="text-align: right">24</td><td>058</td><td>Dark Brown       </td><td>A</td><td>#69260d</td><td style="background-color: #69260d"></td></tr>
<tr><td style="text-align: right">25</td><td>086</td><td>Deep Rose        </td><td>A</td><td>#ff0060</td><td style="background-color: #ff0060"></td></tr>
<tr><td style="text-align: right">26</td><td>323</td><td>Light Brown      </td><td>A</td><td>#bf8200</td><td style="background-color: #bf8200"></td></tr>
<tr><td style="text-align: right">27</td><td>079</td><td>Salmon Pink      </td><td>A</td><td>#f39178</td><td style="background-color: #f39178"></td></tr>
<tr><td style="text-align: right">28</td><td>000</td><td>Vermilion        </td><td>A</td><td>#ff6805</td><td style="background-color: #ff6805"></td></tr>
<tr><td style="text-align: right">29</td><td>001</td><td>White            </td><td>A</td><td>#f0f0f0</td><td style="background-color: #f0f0f0"></td></tr>
<tr><td style="text-align: right">30</td><td>000</td><td>Violet           </td><td>A</td><td>#c832cd</td><td style="background-color: #c832cd"></td></tr>
<tr><td style="text-align: right">31</td><td>000</td><td>Seacrest         </td><td>A</td><td>#b0bf9b</td><td style="background-color: #b0bf9b"></td></tr>
<tr><td style="text-align: right">32</td><td>019</td><td>Sky Blue         </td><td>A</td><td>#65bfeb</td><td style="background-color: #65bfeb"></td></tr>
<tr><td style="text-align: right">33</td><td>000</td><td>Pumpkin          </td><td>A</td><td>#ffba04</td><td style="background-color: #ffba04"></td></tr>
<tr><td style="text-align: right">34</td><td>010</td><td>Cream Yellow     </td><td>A</td><td>#fff06c</td><td style="background-color: #fff06c"></td></tr>
<tr><td style="text-align: right">35</td><td>000</td><td>Khaki            </td><td>A</td><td>#feca15</td><td style="background-color: #feca15"></td></tr>
<tr><td style="text-align: right">36</td><td>000</td><td>Clay Brown       </td><td>A</td><td>#f38101</td><td style="background-color: #f38101"></td></tr>
<tr><td style="text-align: right">37</td><td>000</td><td>Leaf Green       </td><td>A</td><td>#37a923</td><td style="background-color: #37a923"></td></tr>
<tr><td style="text-align: right">38</td><td>405</td><td>Peacock Blue     </td><td>A</td><td>#23465f</td><td style="background-color: #23465f"></td></tr>
<tr><td style="text-align: right">39</td><td>000</td><td>Gray             </td><td>A</td><td>#a6a695</td><td style="background-color: #a6a695"></td></tr>
<tr><td style="text-align: right">40</td><td>000</td><td>Warm Gray        </td><td>A</td><td>#cebfa6</td><td style="background-color: #cebfa6"></td></tr>
<tr><td style="text-align: right">41</td><td>000</td><td>Dark Olive       </td><td>A</td><td>#96aa02</td><td style="background-color: #96aa02"></td></tr>
<tr><td style="text-align: right">42</td><td>307</td><td>Linen            </td><td>A</td><td>#ffe3c6</td><td style="background-color: #ffe3c6"></td></tr>
<tr><td style="text-align: right">43</td><td>000</td><td>Pink             </td><td>A</td><td>#ff99d7</td><td style="background-color: #ff99d7"></td></tr>
<tr><td style="text-align: right">44</td><td>000</td><td>Deep Green       </td><td>A</td><td>#007004</td><td style="background-color: #007004"></td></tr>
<tr><td style="text-align: right">45</td><td>000</td><td>Lavender         </td><td>A</td><td>#edccfb</td><td style="background-color: #edccfb"></td></tr>
<tr><td style="text-align: right">46</td><td>000</td><td>Wisteria Violet  </td><td>A</td><td>#c089d8</td><td style="background-color: #c089d8"></td></tr>
<tr><td style="text-align: right">47</td><td>843</td><td>Beige            </td><td>A</td><td>#e7d9b4</td><td style="background-color: #e7d9b4"></td></tr>
<tr><td style="text-align: right">48</td><td>000</td><td>Carmine          </td><td>A</td><td>#e90e86</td><td style="background-color: #e90e86"></td></tr>
<tr><td style="text-align: right">49</td><td>000</td><td>Amber Red        </td><td>A</td><td>#cf6829</td><td style="background-color: #cf6829"></td></tr>
<tr><td style="text-align: right">50</td><td>000</td><td>Olive Green      </td><td>A</td><td>#408615</td><td style="background-color: #408615"></td></tr>
<tr><td style="text-align: right">51</td><td>107</td><td>Dark Fuchsia     </td><td>A</td><td>#db1797</td><td style="background-color: #db1797"></td></tr>
<tr><td style="text-align: right">52</td><td>209</td><td>Tangerine        </td><td>A</td><td>#ffa704</td><td style="background-color: #ffa704"></td></tr>
<tr><td style="text-align: right">53</td><td>017</td><td>Light Blue       </td><td>A</td><td>#b9ffff</td><td style="background-color: #b9ffff"></td></tr>
<tr><td style="text-align: right">54</td><td>507</td><td>Emerald Green    </td><td>A</td><td>#228927</td><td style="background-color: #228927"></td></tr>
<tr><td style="text-align: right">55</td><td>614</td><td>Purple           </td><td>A</td><td>#b612cd</td><td style="background-color: #b612cd"></td></tr>
<tr><td style="text-align: right">56</td><td>515</td><td>Moss Green       </td><td>A</td><td>#00aa00</td><td style="background-color: #00aa00"></td></tr>
<tr><td style="text-align: right">57</td><td>124</td><td>Flesh Pink       </td><td>A</td><td>#fea9dc</td><td style="background-color: #fea9dc"></td></tr>
<tr><td style="text-align: right">58</td><td>000</td><td>Harvest Gold     </td><td>A</td><td>#fed510</td><td style="background-color: #fed510"></td></tr>
<tr><td style="text-align: right">59</td><td>000</td><td>Electric Blue    </td><td>A</td><td>#0097df</td><td style="background-color: #0097df"></td></tr>
<tr><td style="text-align: right">60</td><td>205</td><td>Lemon Yellow     </td><td>A</td><td>#ffff84</td><td style="background-color: #ffff84"></td></tr>
<tr><td style="text-align: right">61</td><td>027</td><td>Fresh Green      </td><td>A</td><td>#cfe774</td><td style="background-color: #cfe774"></td></tr>
<tr><td style="text-align: right">62</td><td>000</td><td>Applique Material</td><td>A</td><td>#ffc864</td><td style="background-color: #ffc864"></td></tr>
<tr><td style="text-align: right">63</td><td>000</td><td>Applique Position</td><td>A</td><td>#ffc8c8</td><td style="background-color: #ffc8c8"></td></tr>
<tr><td style="text-align: right">64</td><td>000</td><td>Applique         </td><td>A</td><td>#ffc8c8</td><td style="background-color: #ffc8c8"></td></tr>
</table>
