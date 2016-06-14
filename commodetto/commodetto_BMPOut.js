/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Commodetto BMP Output
		
		implements PixelOut to write pixels to a BMP file

	//@@ rework so it accepts file object
*/

import PixelsOut from "PixelsOut";
import File from "file";

export default class BMPOut extends PixelsOut {
	constructor(dictionary) {
		if (("rgb565le" != dictionary.pixelFormat))
			throw new Error("invalid display settings");

		super(dictionary);

		this.file = new File(dictionary.path, 1);
	}
	begin(x, y, width, height) {
		let rowBytes = this.pixelsToBytes(width);

		this.file.length = 0;
		this.file.position = 0;

		// 0x36 byte header
		this.file.write("BM");						// imageFileType
		this.write32((rowBytes * height) + 0x36);	// fileSize
		this.write16(0);							// reserved1
		this.write16(0);							// reserved2
		this.write32(0x36);							// imageDataOffset

		this.write32(0x28);							// biSize
		this.write32(width);						// biWidth
		this.write32(-height);						// biHeight (negative, because we write top-to-bottom)
		this.write16(1);							// biPlanes
		this.write16(16);							// biBitCount
		this.write32(0);							// biCompression
		this.write32((rowBytes * height) + 2);		// biSizeImage
		this.write32(0x0b12);						// biXPelsPerMeter
		this.write32(0x0b12);						// biYPelsPerMeter
		this.write32(0);							// biClrUsed
		this.write32(0);							// biClrImportant
	}

	send(pixels, offset = 0, count = pixels.byteLength - offset) {
		let bytes = new Uint8Array(pixels);
		count >>= 1;
		while (count--) {
			let pixel = bytes[offset++];
			pixel |= bytes[offset++] << 8;

			let r = (pixel >> 11);
			let g = (pixel >> 6) & 0x1f;			// drop low bit (16 bit BMP is 555)
			let b = pixel & 0x1f;

			this.write16((r << 10) | (g << 5) | b);
		}
	}
	end() {
		this.file.close();
		delete this.file;
	}
	pixelsToBytes(count) {
		return count * 2;		//@@ vary on pixelFormat
	}

	write16(value) {
		this.file.write(value & 0xff, (value >> 8) & 0xff);
	}
	write32(value) {
		this.file.write(value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff, (value >> 24) & 0xff);
	}
}
