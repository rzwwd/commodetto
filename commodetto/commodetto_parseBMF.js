/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Commodetto BMF loader

		parses BMF data structure
		puts needed header fields and glyph metrics into ArrayBuffer for use by native rendering code

		http://www.angelcode.com/products/bmfont/doc/file_format.html
*/

export default function parseBMF(font)
{
	let parser = new Parser(font);
	return parser.parse();
}

class Parser {
	constructor(buffer) {
		this.buffer = buffer;
		this.bytes = new Uint8Array(buffer);
		this.position = undefined;
	}

	seekTo(position) {
		this.position = position;
	}
	seekBy(delta) {
		this.seekTo(this.position + delta);
	}
	readU32() {
		let value = this.bytes[this.position] |  (this.bytes[this.position + 1] << 8) | (this.bytes[this.position + 2] << 8) | (this.bytes[this.position + 3] << 8);
		this.position += 4;
		return value;
	}
	readS16() {
		let value = (this.bytes[this.position]) | (this.bytes[this.position + 1] << 8);
		if (value > 32767) value -= 65536;
		this.position += 2;
		return value;
	}
	readS8() {
		let value = this.bytes[this.position++];
		if (value > 127) value -= 256;
		return value;
	}
	readU8() {
		return this.bytes[this.position++];
	}

	parse() {
		let bmf = this.bytes, buffer = this.buffer;

		if ((0x42 != bmf[0]) || (0x4D != bmf[1]) || (0x46 != bmf[2]) || (3 != bmf[3]))
			throw new Error("Invalid BMF header");

		this.seekTo(4);

		// skip block 1
		if (1 != this.readU8())
			throw new Error("can't find info block");

		this.seekBy(this.readU32());

		// get lineHeight from block 2
		if (2 != this.readU8())
			throw new Error("can't find common block");
		let size = this.readU32();

		buffer.height = this.readS16();
		buffer.ascent = this.readS16();
		this.seekBy(size - 4);

		// skip block 3
		if (3 != this.readU8())
			throw new Error("can't find pages block");

		this.seekBy(this.readU32());

		// use block 4
		if (4 != this.readU8())
			throw new Error("can't find chars block");

		buffer.position = this.position;		// position of size of chars table

		size = this.readU32();
		if (size % 20)
			throw new Error("bad chars block size");

		let firstChar = this.readU32();
		let lastChar = firstChar + (size / 20);
		this.seekBy(20 - 4);
		for (let i = firstChar + 1; i < lastChar; i++) {
			let c = this.readU32();
			if (c != i)
				throw new Error("gap detected - chars must be consecutive");
			this.seekBy(20 - 4);
		}

		buffer.firstChar = firstChar;
		buffer.lastChar = lastChar;

		return buffer;
	}
};
