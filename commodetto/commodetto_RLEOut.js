/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Commodetto RLE in Memory
		
		implements PixelOut to write a run-length encoded bitmap to ArrayBuffer

		series of runs. three types of runs
			1. quote (copy pixels)
			2. repeat (repeat single pixel)
			3. skip (don't render)

		run header is one byte. high two bits determine run type.
		remaining bits are run length.
			0-  > literal 1 to 128
			10  > repeat 2 to 65
			11  > skip 1 to 64
*/

import PixelsOut from "PixelsOut";
import Bitmap from "Bitmap";

export default class RLEOut extends PixelsOut {
	constructor(dictionary) {
		if (("rgb565le" != dictionary.pixelFormat))
			throw new Error("invalid display settings");

		super(dictionary);

		this.key = dictionary.key;
	}
	begin(x, y, width, height) {
		this.buffers = [];
		this.output = undefined;
		this.windowWidth = width;
		this.windowHeight = height;
		this.scratch = new Uint8Array(this.pixelsToBytes(width) + width);
	}
	send(pixels, offsetIn = 0, count = pixels.byteLength - offsetIn) {
		let depth = this.pixelsToBytes(1) * 8;		//@@ base on SOURCE bitmap, not DEST
		let white = 0xffff;		//@@ input parameter
		let width = this.windowWidth;
		let mask = new Uint8Array(this.pixelsToBytes(width))
		pixels = new Uint16Array(pixels);
		let lines = Math.floor(pixels.length / width);
		let bytes = this.scratch;
		let key = this.key;
		if (undefined === key)
			mask.fill(1);
		for (let line = 0, offset = 0; line < lines; line++, offset += width) {
			if (undefined !== key) {
				// build mask
				for (let x = 0; x < width; x++) {
					let pixel = pixels[offset + x];
					mask[x] = (white != pixel);
				}
			}

			let position = 0;
			for (let x = 0; x < width; ) {
				let maskRunLength;

				if (undefined !== key) {
					maskRunLength = 1;
					while ((maskRunLength < 64) && ((x + maskRunLength) < width) && (mask[x] == mask[x + maskRunLength]))		//@@ 64 is conservative... depends on run type
						maskRunLength += 1;
				}
				else
					maskRunLength = width;

				if (mask[x]) {
					// visible pixels
					while (maskRunLength) {
						// is the start of this run a repeat?
						if (maskRunLength >= 3) {
							let repeatLength;
							for (repeatLength = 0; repeatLength < maskRunLength; repeatLength++) {
								if (pixels[offset + x] != pixels[offset + x + repeatLength])
									break;
							}
							if (repeatLength >= 3) {
								if (repeatLength > 64)
									repeatLength = 64;								// maximum repeat length
								bytes[position++] = 0x80 | (repeatLength - 1);		// repeat op-code
								bytes[position++] = pixels[offset + x] & 0xff;		// repeat value
								if (16 == depth)
									bytes[position++] = pixels[offset + x] >> 8;	// repeat value
								maskRunLength -= repeatLength;
								x += repeatLength;
								continue;
							}
						}

						// literal. find next repeat, if any, to see for how long the literal lasts.
						let repeatStart = maskRunLength;
						for (let j = 0; j < maskRunLength - 2; j++) {
							if ((pixels[offset + x + j] != pixels[offset + x + j + 1]) || (pixels[offset + x + j] != pixels[offset + x + j + 2]))
								continue;

							repeatStart = j;
							break;
						}

						if (repeatStart > 64)
							repeatStart = 64;										// maximum literal length

						bytes[position++] = repeatStart - 1;						// literal op-code
						for (let j = 0; j < repeatStart; j++) {
							bytes[position++] = pixels[offset + x + j] & 0xff;		// literal data byte
							if (16 == depth)
								bytes[position++] = pixels[offset + x + j] >> 8;	// literal data byte
						}
						maskRunLength -= repeatStart;
						x += repeatStart;
					}
				}
				else {
					// skip pixels
					if (maskRunLength > 64)
						maskRunLength = 64;								// maximum skip length
					bytes[position++] = 0xc0 | (maskRunLength - 1);		// skip op-code
					x += maskRunLength;
				}
			}
			this.buffers.push(new Uint8Array(bytes.buffer.slice(0, position)));
		}
	}
	end() {
		let byteLength = 0;
		this.buffers.forEach(buffer => byteLength += buffer.byteLength);

		this.output = new Uint8Array(byteLength);
		let offset = 0;
		this.buffers.forEach(buffer => {
			this.output.set(buffer, offset);
			offset += buffer.byteLength;
		});
		this.bits = new Bitmap(this.windowWidth, this.windowHeight, Bitmap.RLE, this.output.buffer, 0);

		delete this.buffers;

	}
	pixelsToBytes(count) {
		return count * 2;		//@@ vary on pixelFormat
	}
	get bitmap() {
		return this.bits;
	}

	static encode(source, key) {
		let Poco = require("commodetto/Poco");
		let writer = new RLEOut({width: source.width, height: source.height, pixelFormat: "rgb565le", key: key});
		let render = new Poco(writer);
		render.begin();
			render.drawBitmap(source, 0, 0);
		render.end();
		return writer.bits;
	}

}
