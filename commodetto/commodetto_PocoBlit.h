/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdint.h>

#ifndef kPocoPixelSize
	#define kPocoPixelSize (16)
#endif

#include "commodetto_Bitmap.h"

typedef enum {
	kPocoMonochromeForeground = 1,
	kPocoMonochromeBackground = 2,
	kPocoMonochromeForeAndBackground = (kPocoMonochromeForeground | kPocoMonochromeBackground)
 }  PocoMonochromeMode;

#define kPocoOpaque (255)

typedef CommodettoCoordinate PocoCoordinate;
typedef CommodettoDimension PocoDimension;
typedef CommodettoPixel PocoPixel;
typedef CommodettoBitmapFormat PocoBitmapFormat;

typedef struct {
	PocoCoordinate	x;
	PocoCoordinate	y;
	PocoDimension	w;
	PocoDimension	h;
} PocoRectangleRecord, *PocoRectangle;

typedef struct PocoCommandRecord PocoCommandRecord;
typedef struct PocoCommandRecord *PocoCommand;

struct PocoRecord {
	PocoDimension		width;
	PocoDimension		height;
//	PocoBitmapFormat	format;

	int					pixelsLength;

	char				*displayList;
	const char			*displayListEnd;
	PocoCommand			next;

	// clip rectangle of active drawing operation
	PocoCoordinate		x;
	PocoCoordinate		y;
	PocoDimension		w;
	PocoDimension		h;
	PocoCoordinate		xMax;
	PocoCoordinate		yMax;

	// origin
	PocoCoordinate		xOrigin;
	PocoCoordinate		yOrigin;

	uint8_t				displayListOverflow;
	uint8_t				stackProblem;
	uint8_t				bigEndian;
	uint8_t				stackDepth;

	PocoPixel			pixels[1];			// displayList follows pixels
};

typedef struct PocoRecord PocoRecord;
typedef struct PocoRecord *Poco;

typedef struct PocoBitmapRecord {
	PocoDimension		width;
	PocoDimension		height;
	PocoBitmapFormat	format;

	PocoPixel			*pixels;
} PocoBitmapRecord, *PocoBitmap;

typedef void (*PocoRenderedPixelsReceiver)(PocoPixel *pixels, int byteCount, void *refCon);

//#define READ_PROG_MEM_UNSIGNED_BYTE(a) (pgm_read_byte_near(a))
#define READ_PROG_MEM_UNSIGNED_BYTE(a) (*(unsigned char *)a)

#define PocoMakeColor(r, g, b) (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3))

#ifdef __cplusplus
extern "C" {
#endif

void PocoDrawingBegin(Poco poco, PocoCoordinate x, PocoCoordinate y, PocoDimension w, PocoDimension h);
int PocoDrawingEnd(Poco poco, PocoPixel *pixels, int byteLength, PocoRenderedPixelsReceiver pixelReceiver, void *refCon);

void PocoRectangleFill(Poco poco, PocoPixel color, uint8_t blend, PocoCoordinate x, PocoCoordinate y, PocoDimension w, PocoDimension h);
void PocoPixelDraw(Poco poco, PocoPixel color, PocoCoordinate x, PocoCoordinate y);

PocoCommand PocoBitmapDraw(Poco poco, PocoBitmap bits, PocoCoordinate x, PocoCoordinate y, PocoDimension sx, PocoDimension sy, PocoDimension sw, PocoDimension sh);
void PocoMonochromeBitmapDraw(Poco poco, PocoBitmap bits, PocoMonochromeMode mode, PocoPixel fgColor, PocoPixel bgColor, PocoCoordinate x, PocoCoordinate y, PocoDimension sx, PocoDimension sy, PocoDimension sw, PocoDimension sh);
void PocoGrayBitmapDraw(Poco poco, PocoBitmap bits, PocoPixel color, PocoCoordinate x, PocoCoordinate y, PocoDimension sx, PocoDimension sy, PocoDimension sw, PocoDimension sh);

void PocoBitmapDrawMasked(Poco poco, PocoBitmap bits, PocoCoordinate x, PocoCoordinate y, PocoDimension sx, PocoDimension sy, PocoDimension sw, PocoDimension sh,
			PocoBitmap mask, PocoDimension mask_sx, PocoDimension mask_sy);

void PocoBitmapPattern(Poco poco, PocoBitmap bits, PocoCoordinate x, PocoCoordinate y, PocoDimension w, PocoDimension h, PocoDimension sx, PocoDimension sy, PocoDimension sw, PocoDimension sh);

void PocoClipPush(Poco poco, PocoCoordinate x, PocoCoordinate y, PocoDimension w, PocoDimension h);
void PocoClipPop(Poco poco);

void PocoOriginPush(Poco poco, PocoCoordinate x, PocoCoordinate y);
void PocoOriginPop(Poco poco);

#ifdef __cplusplus
}
#endif

