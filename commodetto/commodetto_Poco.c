/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "commodetto_PocoBlit.h"
#include "commodetto_Bitmap.h"

#include "stddef.h"		// for offsetof macro

#include "mc_xs.h"
#include "mc_misc.h"
#include "mc.xs.h"	//@@ use xsID_* in place of xsID("*")

//#define ID(symbol) (xsID_##symbol)
#define ID(symbol) (xsID(#symbol))

#define xsGetHostDataPoco(slot) ((void *)((char *)xsGetHostData(slot) - offsetof(PocoRecord, pixels)))

void xs_poco_destructor(void *data)
{
	if (data)
		mc_free(((uint8_t *)data) - offsetof(PocoRecord, pixels));
}

void xs_poco_build(xsMachine *the)
{
	Poco poco;
	int pixelsLength = xsToInteger(xsArg(2));
	int pixelFormat = xsToInteger(xsArg(3));
	int displayListLength = xsToInteger(xsArg(4));
	int byteLength = pixelsLength + displayListLength;

	poco = mc_malloc(sizeof(PocoRecord) + byteLength);
	xsSetHostData(xsThis, poco->pixels);

	poco->width = (PocoDimension)xsToInteger(xsArg(0));
	poco->height = (PocoDimension)xsToInteger(xsArg(1));
	poco->bigEndian = 1 == pixelFormat;
	poco->pixelsLength = pixelsLength;

	poco->displayList = ((char *)poco->pixels) + pixelsLength;
	poco->displayListEnd = poco->displayList + displayListLength;

	xsVars(1);
	xsVar(0) = xsInteger(pixelsLength);
	xsSet(xsThis, ID(byteLength), xsVar(0));
}

void xs_poco_begin(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	int argc = xsToInteger(xsArgc);
	PocoCoordinate x, y;
	PocoDimension w, h;

	if (argc >= 1)
		x = (PocoCoordinate)xsToInteger(xsArg(0));
	else
		x = 0;

	if (argc >= 2)
		y = (PocoCoordinate)xsToInteger(xsArg(1));
	else
		y = 0;

	if (argc >= 3)
		w = (PocoDimension)xsToInteger(xsArg(2));
	else
		w = poco->width - x;

	if (argc >= 4)
		h = (PocoDimension)xsToInteger(xsArg(3));
	else
		h = poco->height - y;

	PocoDrawingBegin(poco, x, y, w, h);

	xsEnableGarbageCollection(false);		//@@ to keep derefereced pixels from sliding out from under us
}

static void pixelReceiverLittleEndian(PocoPixel *pixels, int byteLength, void *refCon)
{
	xsMachine *the = refCon;

	xsVar(1) = xsInteger(0);			// offset
	xsVar(2) = xsInteger(byteLength);
	xsCall3(xsVar(0), ID(send), xsVar(3), xsVar(1), xsVar(2));
}

static void pixelReceiverBigEndian(PocoPixel *pixels, int byteLength, void *refCon)
{
	xsMachine *the = refCon;
	int i, count = byteLength / 2;

	for (i = 0; i < count; i++) {
		PocoPixel pixel = pixels[i];
		pixels[i] = (pixel >> 8) | ((pixel & 255) << 8);
	}

	xsVar(1) = xsInteger(0);			// offset
	xsVar(2) = xsInteger(byteLength);
	xsCall3(xsVar(0), ID(send), xsVar(3), xsVar(1), xsVar(2));
}

void xs_poco_end(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	int result;

	xsVars(5);

	xsGet(xsVar(0), xsThis, ID(pixelsOut));
	xsVar(1) = xsInteger(poco->x);
	xsVar(2) = xsInteger(poco->y);
	xsVar(3) = xsInteger(poco->w);
	xsVar(4) = xsInteger(poco->h);
	xsCall4(xsVar(0), ID(begin), xsVar(1), xsVar(2), xsVar(3), xsVar(4));

	xsVar(3) = xsThis;		// Poco doubles as pixels
	result = PocoDrawingEnd(poco, poco->pixels, poco->pixelsLength, poco->bigEndian ? pixelReceiverBigEndian : pixelReceiverLittleEndian, the);
	if (result) {
		if (1 == result)
			xsErrorPrintf("display list overflowed");
		if (2 == result)
			xsErrorPrintf("clip/origin stack not cleared");
		if (3 == result)
			xsErrorPrintf("clip/origin stack under/overflow");
		xsErrorPrintf("unknown error");
	}

	xsEnableGarbageCollection(true);		//@@ slide away!
	xsCollectGarbage();						//@@ not every time!!

	if ((xsToInteger(xsArgc) > 0) && xsTest(xsArg(0)))
		xsCall0(xsVar(0), ID(continue));
	else
		xsCall0(xsVar(0), ID(end));
}

void xs_poco_makeColor(xsMachine *the)
{
	int r, g, b, color;

	r = xsToInteger(xsArg(0));
	g = xsToInteger(xsArg(1));
	b = xsToInteger(xsArg(2));

	color = PocoMakeColor(r, g, b);

	xsResult = xsInteger(color);
}

void xs_poco_clip(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	int argc = xsToInteger(xsArgc);

	if (argc) {
		PocoCoordinate x, y;
		PocoDimension w, h;

		x = xsToInteger(xsArg(0)) + poco->xOrigin;
		y = xsToInteger(xsArg(1)) + poco->yOrigin;
		w = xsToInteger(xsArg(2));
		h = xsToInteger(xsArg(3));
		PocoClipPush(poco, x, y, w, h);
	}
	else
		PocoClipPop(poco);
}

void xs_poco_origin(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	int argc = xsToInteger(xsArgc);

	if (argc) {
		PocoCoordinate x, y;

		x = xsToInteger(xsArg(0));
		y = xsToInteger(xsArg(1));
		PocoOriginPush(poco, x, y);
	}
	else
		PocoOriginPop(poco);
}

void xs_poco_fillRectangle(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	PocoPixel color;
	PocoCoordinate x, y;
	PocoDimension w, h;

	color = (PocoPixel)xsToInteger(xsArg(0));
	x = (PocoCoordinate)xsToInteger(xsArg(1)) + poco->xOrigin;
	y = (PocoCoordinate)xsToInteger(xsArg(2)) + poco->yOrigin;
	w = (PocoDimension)xsToInteger(xsArg(3));
	h = (PocoDimension)xsToInteger(xsArg(4));

	PocoRectangleFill(poco, color, kPocoOpaque, x, y, w, h);
}

void xs_poco_blendRectangle(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	PocoPixel color;
	PocoCoordinate x, y;
	PocoDimension w, h;
	uint8_t blend;

	color = (PocoPixel)xsToInteger(xsArg(0));
	blend = (uint8_t)xsToInteger(xsArg(1));
	x = (PocoCoordinate)xsToInteger(xsArg(2)) + poco->xOrigin;
	y = (PocoCoordinate)xsToInteger(xsArg(3)) + poco->yOrigin;
	w = (PocoDimension)xsToInteger(xsArg(4));
	h = (PocoDimension)xsToInteger(xsArg(5));

	PocoRectangleFill(poco, color, blend, x, y, w, h);
}

void xs_poco_drawPixel(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	PocoPixel color;
	PocoCoordinate x, y;

	color = (PocoPixel)xsToInteger(xsArg(0));
	x = (PocoCoordinate)xsToInteger(xsArg(1)) + poco->xOrigin;
	y = (PocoCoordinate)xsToInteger(xsArg(2)) + poco->yOrigin;

	PocoPixelDraw(poco, color, x, y);
}

void xs_poco_drawBitmap(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	int argc = xsToInteger(xsArgc);
	PocoBitmapRecord bits;
	PocoCoordinate x, y;
	PocoDimension sx, sy, sw, sh;
	CommodettoBitmap cb;

	xsVars(1);

	cb = xsGetHostData(xsArg(0));
	bits.width = cb->w;
	bits.height = cb->h;
	bits.format = cb->format;

	if (cb->havePointer)
		bits.pixels = cb->bits.data;
	else {
		xsGet(xsVar(0), xsArg(0), ID(buffer));
		bits.pixels = (PocoPixel *)(xsToArrayBuffer(xsVar(0)) + cb->bits.offset);
	}

	x = xsToInteger(xsArg(1)) + poco->xOrigin;
	y = xsToInteger(xsArg(2)) + poco->yOrigin;

	sx = 0, sy = 0;
	sw = bits.width, sh = bits.height;

	if (argc > 3) {
		sx = xsToInteger(xsArg(3));
		sy = xsToInteger(xsArg(4));
		if (argc > 5) {
			sw = xsToInteger(xsArg(5));
			sh = xsToInteger(xsArg(6));
		}
	}
	PocoBitmapDraw(poco, &bits, x, y, sx, sy, sw, sh);
}

void xs_poco_drawMonochrome(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	int argc = xsToInteger(xsArgc);
	PocoBitmapRecord bits;
	PocoCoordinate x, y;
	PocoDimension sx, sy, sw, sh;
	PocoPixel fgColor = 0, bgColor = 0;
	PocoMonochromeMode mode = 0;
	CommodettoBitmap cb;

	xsVars(1);

	cb = xsGetHostData(xsArg(0));
	bits.width = cb->w;
	bits.height = cb->h;
	bits.format = cb->format;

	if (cb->havePointer)
		bits.pixels = cb->bits.data;
	else {
		xsGet(xsVar(0), xsArg(0), ID(buffer));
		bits.pixels = (PocoPixel *)(xsToArrayBuffer(xsVar(0)) + cb->bits.offset);
	}

	if (xsUndefinedType != xsTypeOf(xsArg(1))) {
		fgColor = xsToInteger(xsArg(1));
		mode = kPocoMonochromeForeground;
	}

	if (xsUndefinedType != xsTypeOf(xsArg(2))) {
		bgColor = xsToInteger(xsArg(2));
		mode |= kPocoMonochromeBackground;
	}

	x = xsToInteger(xsArg(3)) + poco->xOrigin;
	y = xsToInteger(xsArg(4)) + poco->yOrigin;

	sx = 0, sy = 0;
	sw = bits.width, sh = bits.height;

	if (argc > 5) {
		sx = xsToInteger(xsArg(5));
		sy = xsToInteger(xsArg(6));
		if (argc > 7) {
			sw = xsToInteger(xsArg(7));
			sh = xsToInteger(xsArg(8));
		}
	}

	PocoMonochromeBitmapDraw(poco, &bits, mode, fgColor, bgColor, x, y, sx, sy, sw, sh);
}

void xs_poco_drawGray(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	int argc = xsToInteger(xsArgc);
	PocoBitmapRecord bits;
	PocoCoordinate x, y;
	PocoDimension sx, sy, sw, sh;
	PocoPixel color = 0;
	CommodettoBitmap cb;

	xsVars(1);

	cb = xsGetHostData(xsArg(0));
	bits.width = cb->w;
	bits.height = cb->h;
	bits.format = cb->format;

	if (cb->havePointer)
		bits.pixels = cb->bits.data;
	else {
		xsGet(xsVar(0), xsArg(0), ID(buffer));
		bits.pixels = (PocoPixel *)(xsToArrayBuffer(xsVar(0)) + cb->bits.offset);
	}

	color = xsToInteger(xsArg(1));

	x = xsToInteger(xsArg(2)) + poco->xOrigin;
	y = xsToInteger(xsArg(3)) + poco->yOrigin;

	sx = 0, sy = 0;
	sw = bits.width, sh = bits.height;

	if (argc > 4) {
		sx = xsToInteger(xsArg(4));
		sy = xsToInteger(xsArg(5));
		if (argc > 6) {
			sw = xsToInteger(xsArg(6));
			sh = xsToInteger(xsArg(7));
		}
	}

	PocoGrayBitmapDraw(poco, &bits, color, x, y, sx, sy, sw, sh);
}

void xs_poco_drawMasked(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	PocoBitmapRecord bits, mask;
	PocoCoordinate x, y, mask_sx, mask_sy;
	PocoDimension sx, sy, sw, sh;
	CommodettoBitmap cb;

	xsVars(1);

	cb = xsGetHostData(xsArg(0));
	bits.width = cb->w;
	bits.height = cb->h;
	bits.format = cb->format;

	if (cb->havePointer)
		bits.pixels = cb->bits.data;
	else {
		xsGet(xsVar(0), xsArg(0), ID(buffer));
		bits.pixels = (PocoPixel *)(xsToArrayBuffer(xsVar(0)) + cb->bits.offset);
	}

	x = xsToInteger(xsArg(1)) + poco->xOrigin;
	y = xsToInteger(xsArg(2)) + poco->yOrigin;
	sx = xsToInteger(xsArg(3));
	sy = xsToInteger(xsArg(4));
	sw = xsToInteger(xsArg(5));
	sh = xsToInteger(xsArg(6));

	cb = xsGetHostData(xsArg(7));
	mask.width = cb->w;
	mask.height = cb->h;
	mask.format = cb->format;

	if (cb->havePointer)
		mask.pixels = cb->bits.data;
	else {
		xsGet(xsVar(0), xsArg(7), ID(buffer));
		mask.pixels = (PocoPixel *)(xsToArrayBuffer(xsVar(0)) + cb->bits.offset);
	}

	mask_sx = xsToInteger(xsArg(8));
	mask_sy = xsToInteger(xsArg(9));

	PocoBitmapDrawMasked(poco, &bits, x, y, sx, sy, sw, sh, &mask, mask_sx, mask_sy);
}

void xs_poco_fillPattern(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	PocoBitmapRecord bits;
	PocoCoordinate x, y;
	PocoDimension w, h, sx, sy, sw, sh;
	CommodettoBitmap cb;

	xsVars(1);

	cb = xsGetHostData(xsArg(0));
	bits.width = cb->w;
	bits.height = cb->h;
	bits.format = cb->format;

	if (cb->havePointer)
		bits.pixels = cb->bits.data;
	else {
		xsGet(xsVar(0), xsArg(0), ID(buffer));
		bits.pixels = (PocoPixel *)(xsToArrayBuffer(xsVar(0)) + cb->bits.offset);
	}

	x = xsToInteger(xsArg(1)) + poco->xOrigin;
	y = xsToInteger(xsArg(2)) + poco->yOrigin;
	w = xsToInteger(xsArg(3));
	h = xsToInteger(xsArg(4));
	sx = xsToInteger(xsArg(5));
	sy = xsToInteger(xsArg(6));
	sw = xsToInteger(xsArg(7));
	sh = xsToInteger(xsArg(8));

	PocoBitmapPattern(poco, &bits, x, y, w, h, sx, sy, sw, sh);
}

void xs_poco_getTextWidth(xsMachine *the)
{
	const unsigned char *text = (const unsigned char *)xsToString(xsArg(0));
	int firstChar, lastChar, width = 0;

	xsVars(2);
	xsGet(xsVar(0), xsArg(1), ID(glyphs));
	if (xsUndefinedType == xsTypeOf(xsVar(0)) ) {
		// BMF
		const unsigned char *chars;

		if (xsIsInstanceOf(xsArg(1), xsArrayBufferPrototype))
			chars = xsToArrayBuffer(xsArg(1));
		else
			chars = xsGetHostData(xsArg(1));

		xsGet(xsVar(0), xsArg(1), ID(position));
		chars += xsToInteger(xsVar(0));

		lastChar = (*(int *)chars) / 20;
		chars += 4;
		firstChar = *(int *)chars;
		lastChar += firstChar;

		while (true) {
			int c = *text++;
			if (!c)
				break;

			if (c > lastChar)
				continue;

			c -= firstChar;
			if (c < 0)
				continue;

			width += *(uint16_t *)(chars + (c * 20) + 16);	// +16 -> offset to xadvance
		}
	}
	else {
		// NFNT
		const uint8_t *header = xsToArrayBuffer(xsVar(0));		//@@ no HostBuffer case - NFNT glyph table built in RAM for now
		const uint8_t *glyphs = header + 16;					// skip header

		firstChar = *(int16_t *)(header + 0);
		lastChar = *(int16_t *)(header + 2);

		while (true) {
			int c = *text++;
			if (!c)
				break;

			if (c > lastChar)
				continue;

			c -= firstChar;
			if (c < 0)
				continue;

			width += glyphs[(c * 4) + 3];		// width
		}
	}

	xsResult = xsInteger(width);
}

void xs_poco_drawText(xsMachine *the)
{
	Poco poco = xsGetHostDataPoco(xsThis);
	int argc = xsToInteger(xsArgc);
	const unsigned char *text = (const unsigned char *)xsToString(xsArg(0));
	int firstChar, lastChar;
	PocoCoordinate x, y;
	PocoPixel color;
	CommodettoBitmap cb;
	PocoBitmapRecord bits;
	static const char *ellipsis = "...";
	PocoDimension ellipsisWidth;
	int width;

	x = xsToInteger(xsArg(3)) + poco->xOrigin;
	y = xsToInteger(xsArg(4)) + poco->yOrigin;

	if ((x >= poco->xMax) || (y >= poco->yMax))		// clipped off right or bottom
		return;

	if (argc > 5)
		width = xsToInteger(xsArg(5));
	else
		width = 0;

	xsVars(3);
	xsGet(xsVar(0), xsArg(1), ID(glyphs));
	if (xsUndefinedType == xsTypeOf(xsVar(0)) ) {
		// BMF
		const unsigned char *chars;
		uint8_t isColor;
		PocoBitmapRecord mask;

		if (xsIsInstanceOf(xsArg(1), xsArrayBufferPrototype))
			chars = xsToArrayBuffer(xsArg(1));
		else
			chars = xsGetHostData(xsArg(1));

		xsGet(xsVar(0), xsArg(1), ID(position));
		chars += xsToInteger(xsVar(0));

		lastChar = (*(int *)chars) / 20;
		chars += 4;
		firstChar = *(int *)chars;
		lastChar += firstChar;

		if (width) {
			const uint8_t *cc = chars + (('.' - firstChar) * 20);	//@@ assumes period is in character set
			ellipsisWidth = *(uint16_t *)(cc + 16);					// +16 -> offset to xadvance
			ellipsisWidth *= 3;
		}
		else
			ellipsisWidth = 0;

		// fill out the bitmap
		xsGet(xsVar(0), xsArg(1), ID(bitmap));
		cb = xsGetHostData(xsVar(0));
		bits.width = cb->w;
		bits.height = cb->h;
		bits.format = cb->format;

		if (cb->havePointer)
			bits.pixels = cb->bits.data;
		else {
			xsGet(xsVar(1), xsVar(0), ID(buffer));
			bits.pixels = (PocoPixel *)(xsToArrayBuffer(xsVar(1)) + cb->bits.offset);
		}

		if (kCommodettoBitmapRaw == cb->format) {
			isColor = 1;
			if (xsReferenceType == xsTypeOf(xsArg(2))) {
				isColor = 2;

				cb = xsGetHostData(xsArg(2));
				mask.width = cb->w;
				mask.height = cb->h;
				mask.format = cb->format;
				if (cb->havePointer)
					mask.pixels = cb->bits.data;
				else {
					xsGet(xsVar(1), xsArg(2), ID(buffer));
					mask.pixels = (PocoPixel *)(xsToArrayBuffer(xsVar(1)) + cb->bits.offset);
				}
			}
		}
		else {
			isColor = 0;
			color = (PocoPixel)xsToInteger(xsArg(2));
		}

		while (true) {
			const uint8_t *cc;
			uint16_t xadvance;
			int c = *text++;
			if (!c)
				break;

			if (c > lastChar)
				continue;

			c -= firstChar;
			if (c < 0)
				continue;

			cc = chars + (c * 20);
			xadvance = *(uint16_t *)(cc + 16);	// +16 -> offset to xadvance

			if (ellipsisWidth && ((width - xadvance) <= ellipsisWidth)) {
				// measure the rest of the string to see if it fits
				const unsigned char *t = text - 1;
				int w = 0;
				while (w < width) {
					c = *t++;
					if (!c)
						break;

					if (c > lastChar)
						continue;

					c -= firstChar;
					if (c < 0)
						continue;
					w += *(uint16_t *)((chars + (c * 20)) + 16);
				}
				if (w <= width)
					ellipsisWidth = 0;		// enough room to draw the test of the string
				else {
					text = (const unsigned char *)ellipsis;		// draw ellipsis
					continue;
				}
			}

			if (isColor) {
				if (1 == isColor)
					PocoBitmapDraw(poco, &bits, x + *(uint16_t *)(cc + 12), y + *(uint16_t *)(cc + 14), *(uint16_t *)(cc + 4), *(uint16_t *)(cc + 6), *(uint16_t *)(cc + 8), *(uint16_t *)(cc + 10));
				else
					PocoBitmapDrawMasked(poco, &bits, x + *(uint16_t *)(cc + 12), y + *(uint16_t *)(cc + 14), *(uint16_t *)(cc + 4), *(uint16_t *)(cc + 6), *(uint16_t *)(cc + 8), *(uint16_t *)(cc + 10),
						&mask, *(uint16_t *)(cc + 4), *(uint16_t *)(cc + 6));
			}
			else
				PocoGrayBitmapDraw(poco, &bits, color, x + *(uint16_t *)(cc + 12), y + *(uint16_t *)(cc + 14), *(uint16_t *)(cc + 4), *(uint16_t *)(cc + 6), *(uint16_t *)(cc + 8), *(uint16_t *)(cc + 10));

			x += xadvance;
			width -= xadvance;
		}

	}
	else {
		// NFNT
		const uint8_t *header, *glyphs;
		int maxKern;

		color = (PocoPixel)xsToInteger(xsArg(2));

//		xsGet(xsVar(0), xsArg(1), ID(glyphs));
		header = xsToArrayBuffer(xsVar(0));		//@@ no HostBuffer case - NFNT glyph table built in RAM for now
		glyphs = header + 16;		// skip header
		maxKern = *(int16_t *)(header + 4);
		bits.height = *(int16_t *)(header + 6);

		if ((y + bits.height) <= poco->y)								// clipped off the top
			return;

		xsGet(xsVar(0), xsArg(1), ID(bitmap));
		cb = xsGetHostData(xsVar(0));
		bits.width = cb->w;
		bits.height = cb->h;
		bits.format = cb->format;

		if (cb->havePointer)
			bits.pixels = cb->bits.data;
		else {
			xsGet(xsVar(1), xsVar(0), ID(buffer));
			bits.pixels = (PocoPixel *)(xsToArrayBuffer(xsVar(1)) + cb->bits.offset);
		}

		firstChar = *(int16_t *)(header + 0);
		lastChar = *(int16_t *)(header + 2);

		while (*text && (x < poco->xMax)) {
			PocoDimension location, next, fx;
			int c = *text++;
			if (c > lastChar)
				continue;

			c -= firstChar;
			if (c < 0)
				continue;

			location = *(uint16_t *)&glyphs[c * 4];
			next = *(uint16_t *)&glyphs[(c + 1) * 4];

			fx = x + maxKern + glyphs[(c * 4) + 2];		// xOffset
			PocoMonochromeBitmapDraw(poco, &bits, kPocoMonochromeForeground, color, color, fx, y, location, 0, next - location, bits.height);

			x += glyphs[(c * 4) + 3];		// width
		}
	}
}
