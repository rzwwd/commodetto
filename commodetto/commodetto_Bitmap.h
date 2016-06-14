/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef __commodetto_Bitmap_h__
#define __commodetto_Bitmap_h__

#include <stdint.h>

typedef enum {
	kCommodettoBitmapRaw = 1,
	kCommodettoBitmapPacked,
	kCommodettoBitmapMonochrome,
	kCommodettoBitmapGray16
}  CommodettoBitmapFormat;

typedef int16_t CommodettoCoordinate;
typedef uint16_t CommodettoDimension;
typedef uint16_t CommodettoPixel;

typedef struct {
	CommodettoDimension		w;
	CommodettoDimension		h;
	CommodettoBitmapFormat	format;
	int8_t					havePointer;
	union {
		void				*data;
		int32_t				offset;
	} bits;
} CommodettoBitmapRecord, *CommodettoBitmap;

#endif
