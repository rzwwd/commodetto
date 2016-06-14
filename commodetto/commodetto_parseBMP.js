/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Commodetto BMP parser

		trivial validation
		extracts width, height and offset to pixels
		converts pixels from 555 to 565
		converts pixels to 4 bit gray

	To do:

		pixel conversion should be native code
*/

import Bitmap from "Bitmap";

export default function parseBMP(buffer, format = Bitmap.Raw) {
	let bytes = new Uint8Array(buffer);

	if ((bytes[0] != "B".charCodeAt(0)) || (bytes[1] != "M".charCodeAt(0)))
		throw new Error("invalid BMP");

	let offset = bytes[10] | (bytes[11] << 8) | (bytes[12] << 16)| (bytes[13] << 24);
	let size = bytes[14] | (bytes[15] << 8);		// biSize
	let width = bytes[18] | (bytes[19] << 8) | (bytes[20] << 16)| (bytes[21] << 24);
	let height = bytes[22] | (bytes[23] << 8) | (bytes[24] << 16)| (bytes[25] << 24);
	if (bytes[25] & 0x80) {	// negative value... ouch. means BMP is upside down.
		height = ~height;
		height += 1;
	}

	let depth = bytes[28] | (bytes[29] << 8);
	let compression = bytes[30] | (bytes[31] << 8) | (bytes[32] << 16)| (bytes[33] << 24);			// biCompression

	if (4 == depth) {
		if (0 != compression)
			throw new Error("unsupported 4-bit compression");

		if (width & 7)
			throw new Error("4-bit gray bitmap width must be multiple of 8");

		for (let palette = size + 14, i = 0; i < 16; i++, palette += 4) {
			let gray = i | (i << 4);
			if ((gray != bytes[palette + 0]) || (gray != bytes[palette + 1]) || (gray != bytes[palette + 2]))
				throw new Error("not gray color palette");
		}

		return new Bitmap(width, height, Bitmap.Gray, buffer, offset);
	}

	if (1 == depth) {
		if (0 != compression)
			throw new Error("unsupported 1-bit compression");

		if (width & 15)
			throw new Error("1-bit BMP width must be multiple of 32");

		return new Bitmap(width, height, Bitmap.Monochrome, buffer, offset);
	}

	if (16 != depth)
		throw new Error("unsupported BMP depth - must be 16, 4, or 1");

	if ((0 != compression) && (3 != compression))
		throw new Error("unrecognized BMP compression");

	if (width & 1)
		throw new Error("width not multiple of 2");

	if (0 == compression) {
		// convert from 555 to 565
		//@@ this conversion is "in place" - will fail if asset is in read-only storage
		let words = new Uint16Array(buffer, offset);
		for (let i = 0, count = width * height; count; count--, i++) {
			let pixel = words[i];
			let r = pixel >> 10;
			let g = (pixel >> 5) & 0x01f
			let b = pixel & 0x1f;
			words[i] = (r << 11) | (g << 6) | ((g & 0x10) << 1) | b;
		}
	}
	else
	if (3 == compression) {
		// already 565!
	}
	if (Bitmap.Gray == format) {
		let words = new Uint16Array(buffer, offset);
		let bytes = new Uint8Array((width / 2) * height);
		for (let i = 0, count = (width * height) / 2; count; count--, i += 2)
			bytes[i >> 1] = ((gray(words[i]) >> 1) << 4) | (gray(words[i + 1]) >> 1);

		buffer = bytes.buffer;
		offset = 0;
	}

	return new Bitmap(width, height, format, buffer, offset);
}

// takes 565 RGB returns 5 bit gray value
function gray(color)
{
	let r = (color >> 11);
	let g = (color >> 6) & 0x1F;
	let b = (color >> 1) & 0x1F;

	return (r + r + r + b + g + g + g + g) >> 3;
}
