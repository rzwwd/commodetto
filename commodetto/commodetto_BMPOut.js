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
		super(dictionary);

		switch (dictionary.pixelFormat) {
			case "rgb565le":
				this.depth = 16;
				break;

			case "g4":
				this.depth = 4;
				break;

			default:
				throw new Error("unsupported BMP pixel fornat");
				break;

		}

		this.file = new File(dictionary.path, 1);
	}
	begin(x, y, width, height) {
		let rowBytes = this.pixelsToBytes(width);

		this.file.length = 0;
		this.file.position = 0;

		if (16 == this.depth) {
			if (width % 2)
				throw new Error("width must be multiple of 2");

			// 0x46 byte header
			this.file.write("BM");						// imageFileType
			this.write32((rowBytes * height) + 0x46);	// fileSize
			this.write16(0);							// reserved1
			this.write16(0);							// reserved2
			this.write32(0x46);							// imageDataOffset

			this.write32(0x38);							// biSize
			this.write32(width);						// biWidth
			this.write32(-height);						// biHeight (negative, because we write top-to-bottom)
			this.write16(1);							// biPlanes
			this.write16(16);							// biBitCount
			this.write32(3);							// biCompression (3 == 565 pixels (see mask below), 0 == 555 pixels)
			this.write32((rowBytes * height) + 2);		// biSizeImage
			this.write32(0x0b12);						// biXPelsPerMeter
			this.write32(0x0b12);						// biYPelsPerMeter
			this.write32(0);							// biClrUsed
			this.write32(0);							// biClrImportant

			this.file.write(0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);	// masks for 565 pixels
		}
		else if (4 == this.depth) {
			if (width % 8)
				throw new Error("width must be multiple of 8");

			// 0x76 byte header
			this.file.write("BM");						// imageFileType
			this.write32((rowBytes * height) + 0x76);	// fileSize
			this.write16(0);							// reserved1
			this.write16(0);							// reserved2
			this.write32(0x76);							// imageDataOffset

			this.write32(0x28);							// biSize
			this.write32(width);						// biWidth
			this.write32(-height);						// biHeight (negative, because we write top-to-bottom)
			this.write16(1);							// biPlanes
			this.write16(4);							// biBitCount
			this.write32(0);							// biCompression (3 == 565 pixels (see mask below), 0 == 555 pixels)
			this.write32((rowBytes * height) + 2);		// biSizeImage
			this.write32(0x0b12);						// biXPelsPerMeter
			this.write32(0x0b12);						// biYPelsPerMeter
			this.write32(0);							// biClrUsed
			this.write32(0);							// biClrImportant

			for (let i = 0; i < 16; i++) {
				let j = (i << 4) | 4;
				this.file.write(j, j, j, 0);
			}
		}
		else
			throw new Error("unsupported depth");
	}

	send(pixels, offset = 0, count = pixels.byteLength - offset) {
		if ((0 == offset) && (count == pixels.byteLength) && (pixels instanceof ArrayBuffer))		//@@ file.write should support HostBuffer
			this.file.write(pixels);
		else {
			let bytes = new Uint8Array(pixels);
			while (count--)
				this.file.write(bytes[offset++]);
		}
	}
	end() {
		this.file.close();
		delete this.file;
	}
	pixelsToBytes(count) {
		return (count * this.depth) >> 3;		//@@ vary on pixelFormat - round up
	}

	write16(value) {
		this.file.write(value & 0xff, (value >> 8) & 0xff);
	}
	write32(value) {
		this.file.write(value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff, (value >> 24) & 0xff);
	}
}
