/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Commodetto Poco renderer
	
		Renders 565 pixels
		rectangle
		bitmap - unscaled
		text
		etc
*/

// import Render from "Render"
// class Poco extends Render

export default class Poco @ "xs_poco_destructor" {
	constructor(pixelsOut, dictionary) {
		let pixelFormat;

		if ("rgb565le" == pixelsOut.pixelFormat)
			pixelFormat = 0;
		else
		if ("rgb565be" == pixelsOut.pixelFormat)
			pixelFormat = 1;
		else
			throw new Error("unsupported pixelFormat");

		this.pixelsOut = pixelsOut;

		this.build(pixelsOut.width, pixelsOut.height,
					pixelsOut.pixelsToBytes(pixelsOut.width), pixelFormat,
					(dictionary && dictionary.displayListLength) ? dictionary.displayListLength : 1024);
	}
	begin(x, y, width, height) @ "xs_poco_begin"
	end() @ "xs_poco_end"
	continue(...coodinates) {
		this.end(true);
		this.begin(...coodinates);
	}

	// clip and origin stacks
	clip(x, y, width, height) @ "xs_poco_clip"
	origin(x, y, width, height) @ "xs_poco_origin"

	// rendering calls
	makeColor(r, g, b) @ "xs_poco_makeColor"
	fillRectangle(color, x, y, width, height) @ "xs_poco_fillRectangle"
	blendRectangle(color, blend, x, y, width, height) @ "xs_poco_blendRectangle"
	drawPixel(color, x, y) @ "xs_poco_drawPixel"
	drawBitmap(bits, x, y, sx, sy, sw, sh) @ "xs_poco_drawBitmap"
	drawMonochrome(monochrome, fore, back, x, y, sx, sy, sw, sh) @ "xs_poco_drawMonochrome"
	drawGray(bits, color, x, y, sx, sy, sw, sh) @ "xs_poco_drawGray"
	drawMasked(bits, x, y, sx, sy, sw, sh, mask, mask_sx, mask_sy) @ "xs_poco_drawMasked"
	fillPattern(bits, x, y, w, h, sx, sx, sx, sh) @ "xs_poco_fillPattern"

	drawText(text, font, color, x, y) @ "xs_poco_drawText"

	// metrics
	getTextWidth(text, font) @ "xs_poco_getTextWidth"

	// internal implementation
	build(width, height, byteLength, pixelFormat, displayListLength) @ "xs_poco_build"
}

