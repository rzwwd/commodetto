/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Commodetto JPEG reader
		
		Pull model - returns block of pixels on each call to read
		Uses picojpeg decoder

	To do:


*/

import Bitmap from "Bitmap";

export default class JPEG @ "xs_JPEG_destructor" {
	constructor(buffer) @ "xs_JPEG_constructor"
	read() @ "xs_JPEG_read"

	initialize() {
		this.bitmap = new Bitmap(0, 0, Bitmap.Raw, this.pixels, 0);
		return this.bitmap;
	}

	static decompress(data) {
		let BufferOut = require("commodetto/BufferOut");
		let Poco = require("commodetto/Poco");
		let jpeg = new JPEG(data);

		let offscreen = new BufferOut({width: jpeg.width, height: jpeg.height, pixelFormat: "rgb565le"});
		let render = new Poco(offscreen);

		while (true) {
			let block = jpeg.read();
			if (!block)
				break;

			render.begin(block.x, block.y, block.width, block.height);
				render.drawBitmap(block, block.x, block.y);
			render.end();
		}

		return offscreen.bitmap;
	}
}
