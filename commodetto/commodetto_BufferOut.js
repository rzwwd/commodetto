/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Commodetto Buffer in Memory
		
		implements PixelOut to write pixels to an ArrayBuffer
*/

import PixelsOut from "PixelsOut";
import Bitmap from "Bitmap";

const header = 16;			// number of bytes preceeding pixels

export default class BufferOut extends PixelsOut {
	constructor(dictionary) {
		super(dictionary);

		let format = Bitmap.Raw;
		switch (dictionary.pixelFormat) {
			case "rgb565le":
			case "rgb565be":
				this.depth = 16;
				break;

			case "g4":
				this.depth = 4;
				format = Bitmap.Gray;
				break;

			case "m1":
				this.depth = 1;
				format = Bitmap.Monochrome;
				break;

			default:
				throw new Error("unsupported buffer pixel fornat");
				break;

		}

		// initialize array buffer for use as Bitmap (ArrayBuffer + width, height, and offset properties)
		this.buffer = new ArrayBuffer(this.pixelsToBytes(this.width * this.height) + header);
		this.bits = new Bitmap(this.width, this.height, format, this.buffer, header);

		this.words = new Uint32Array(this.buffer, 0, header);
	}
	begin(x, y, width, height) {
		let words = this.words;

		words[0] = this.pixelsToBytes((y * this.width) + x) + header;		// offset
		words[1] = this.pixelsToBytes(width);								// windowWidth
		words[2] = 0;														// windowOffset
		words[3] = this.pixelsToBytes(this.width - width);					// rowBump
	}
	send(pixels, offsetIn, count) @ "xs_BufferOut_send"
	end() {
	}
	continue() {
		// empty implementation overrides PixelOut.continue which throws
	}
	pixelsToBytes(count) {
		return ((count * this.depth) + 7) >> 3;
	}
	get bitmap() {
		return this.bits;
	}
}
