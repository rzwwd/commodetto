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


#include "mc_xs.h"
#include "mc_misc.h"
#include "mc.xs.h"	//@@ use xsID_* in place of xsID("*")

//#define ID(symbol) (xsID_##symbol)
#define ID(symbol) (xsID(#symbol))

#define kHeader (16)

#define kOffset (0)
#define kWindowWidth (1)
#define kWindowOffset (2)
#define kRowBump (3)

void xs_BufferOut_send(xsMachine *the)
{
	int argc = xsToInteger(xsArgc);
	char *src, *dst;
	uint32_t *header;
	int offsetIn, count, offsetOut;

	if (argc >= 1)
		offsetIn = xsToInteger(xsArg(1));
	else
		offsetIn = 0;

	if (argc >= 2)
		count = xsToInteger(xsArg(2));
	else
		count = xsGetArrayBufferLength(xsArg(0)) - offsetIn;

 	if (xsIsInstanceOf(xsArg(0), xsArrayBufferPrototype))
		src = xsToArrayBuffer(xsArg(0));
	else
		src = xsGetHostData(xsArg(0));

	xsVars(1);
	xsGet(xsVar(0), xsThis, ID(buffer));
	header = xsToArrayBuffer(xsVar(0));
	dst = (char *)header;

	offsetOut = header[kOffset];

	while (count) {
		int copy = header[kWindowWidth] - header[kWindowOffset];
		if (copy > count)
			copy = count;
		count -= copy;
		header[kWindowOffset] += copy;
		memcpy(dst + offsetOut, src + offsetIn, copy);
		offsetOut += copy;
		offsetIn += copy;
		if (header[kWindowOffset] == header[kWindowWidth]) {
			header[kWindowOffset] = 0;
			offsetOut += header[kRowBump];
		}
	}

	header[kOffset] = offsetOut;
}

/*
	send(pixels, offsetIn = 0, count = pixels.byteLength - offsetIn) {
		let offsetOut = this.offset;
		let bytes = this.bytes;
		pixels = new Uint8Array(pixels);		//@@

		while (count) {
			let copy = this.windowWidth - this.windowOffset;
			if (copy > count)
				copy = count;
			count -= copy;
			this.windowOffset += copy;
			while (copy--)
				bytes[offsetOut++] = pixels[offsetIn++];
			if (this.windowOffset == this.windowWidth) {
				this.windowOffset = 0;
				offsetOut += this.rowBump;
			}
		}

		this.offset = offsetOut;
	}
*/
