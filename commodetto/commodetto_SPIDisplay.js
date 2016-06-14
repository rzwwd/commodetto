/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Commodetto SPIDisplay
		
		implements PixelOut for a display connected over SPI
*/

/*

import PixelsOut from "PixelsOut";

export default class SPIDisplay extends PixelsOut {
	constructor(dictionary) {
		if ((320 != dictionary.width) ||
			(240 != dictionary.height) ||
			("rgb565le" != dictionary.pixelFormat))
			throw new Error("invalid display settings");

		super(dictionary);
	}
	begin(x, y, width, height) {
	}
	send(pixels, offset = 0, count = pixels.byteLength - offset) {
	}
	end() {
	}
	continue() {
		// empty implementation overrides PixelOut.continue which throws
	}
	pixelsToBytes(count) {
		return count << 1;
	}
}

*/
