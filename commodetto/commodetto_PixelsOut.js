/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Commodetto pixel sink
	
		Receives a stream of pixels
*/

export default class PixelsOut {
	/*
		Dictonary contains:
			width and height are in pixels
			pixelFormat is a string
			orientation is 0, 90, 180, 270 (not suppported on all displays)
	*/
	constructor(dictionary) {
		this.w = dictionary.width;
		this.h = dictionary.height;
		this.format = dictionary.pixelFormat;
	}
	/*
		before any pixels are sent
		area of display to update.
	*/
	begin(x, y, width, height) {
	}

	/*
		after the last pixel has been sent
	*/
	end() {
	}
	/*
		in-between a sequence of begin/end within a single frame
		subclass must override to allow
	*/
	continue() {
		throw new Error("continue not supported for this PixelOut")
	}
	/*
		send block of pixels (contained in ArrayBuffer) to display
		optional offset and count indicate the part of the pixels ArrayBuffer to use
		multiple calls to send may be required to send a full frame of pixels
	*/
	send(pixels, offset = 0, count = pixels.byteLength - offset) {
	}

	/*
		returns width and height of display in pixels
	*/
	get width() {
		return this.w;
	}
	get height() {
		return this.h;
	}
	/*
		returns format of pixel in use
	*/
	get pixelFormat() {
		return this.format;
	}
	/*
		returns byteCount of count pixels
	*/
	pixelsToBytes(count) {
		throw new Error("subclass implements pixelsToBytes")
	}
}

