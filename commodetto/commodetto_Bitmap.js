/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


let Bitmap = class @ "xs_Bitmap_destructor" {
	constructor(width, height, format, buffer, offset) @ "xs_Bitmap"
	get width() @ "xs_bitmap_get_width"
	get height() @ "xs_bitmap_get_height"
	get format() @ "xs_bitmap_get_format"
}

Bitmap.Raw = 1;
Bitmap.RLE = 2;
Bitmap.Monochrome = 3;
Bitmap.Gray = 4;

export default Bitmap;
