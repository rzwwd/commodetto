/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Commodetto readJPEG

		wraps picojpeg
		
	To do:

		PJPG_YH2V2
*/

#include "mc_xs.h"
#include "mc_misc.h"
#include "mc.xs.h"	//@@ use xsID_* in place of xsID("*")

//#define ID(symbol) (xsID_##symbol)
#define ID(symbol) (xsID(#symbol))

#include "commodetto_PocoBlit.h"

#include "picojpeg.h"

typedef struct {
	int					position;
	int					length;
	xsMachine			*the;
	xsSlot				buffer;
	xsSlot				pixels;
	xsSlot				bitmap;
	CommodettoBitmap	cb;
	char				isArrayBuffer;

	unsigned char		blockWidth;
	unsigned char		blockHeight;
	unsigned char		blockX;
	unsigned char		blockY;

   unsigned char		mcuWidth;
   unsigned char		mcuHeight;
   unsigned char		mcuWidthRight;
   unsigned char		mcuHeightBottom;

   pjpeg_scan_type_t	scanType;

   unsigned char		*r;
   unsigned char		*g;
   unsigned char		*b;

   int					lastByteLength;
} JPEGRecord, *JPEG;

static unsigned char needBytes(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data);

void xs_JPEG_destructor(void *data)
{
	if (data)
		mc_free(data);
}

void xs_JPEG_constructor(xsMachine *the)
{
	unsigned char result;
	pjpeg_image_info_t info;

	JPEG jpeg = mc_malloc(sizeof(JPEGRecord));
	if (!jpeg)
		xsErrorPrintf("jpeg out of memory");

	xsSetHostData(xsThis, jpeg);

	xsVars(1);
	xsGet(xsVar(0), xsArg(0), ID(byteLength));
	jpeg->length = xsToInteger(xsVar(0));
	jpeg->position = 0;
	jpeg->the = the;
	jpeg->buffer = xsArg(0);

	xsSet(xsThis, ID(buffer), xsArg(0));

 	jpeg->isArrayBuffer = xsIsInstanceOf(xsArg(0), xsArrayBufferPrototype);

	result = pjpeg_decode_init(&info, needBytes, jpeg, 0);
	if (0 != result)
		xsErrorPrintf("jpeg init failed");

	jpeg->blockWidth = info.m_MCUSPerRow;
	jpeg->blockHeight = info.m_MCUSPerCol;
	jpeg->blockX = 0;
	jpeg->blockY = 0;
	jpeg->mcuWidth = info.m_MCUWidth;
	jpeg->mcuHeight = info.m_MCUHeight;
	jpeg->mcuWidthRight = info.m_width % jpeg->mcuWidth;
	if (!jpeg->mcuWidthRight)
		jpeg->mcuWidthRight = jpeg->mcuWidth;
	jpeg->mcuHeightBottom = info.m_height % jpeg->mcuHeight;
	if (!jpeg->mcuHeightBottom)
		jpeg->mcuHeightBottom = jpeg->mcuHeight;
	jpeg->scanType = info.m_scanType;
	jpeg->r = info.m_pMCUBufR;
	jpeg->g = info.m_pMCUBufG;
	jpeg->b = info.m_pMCUBufB;

	jpeg->pixels = xsArrayBuffer(NULL, jpeg->mcuWidth * jpeg->mcuHeight * sizeof(PocoPixel));
	xsSet(xsThis, ID(pixels), jpeg->pixels);

	xsVar(0) = xsInteger(info.m_width);
	xsSet(xsThis, ID(width), xsVar(0));
	xsVar(0) = xsInteger(info.m_height);
	xsSet(xsThis, ID(height), xsVar(0));

	xsVar(0) = xsCall0(xsThis, ID(initialize));
	jpeg->bitmap = xsVar(0);
	jpeg->cb = xsGetHostData(xsVar(0));		// offset and format set in initialize calls
}

void xs_JPEG_read(xsMachine *the)
{
	JPEG jpeg = xsGetHostData(xsThis);
	unsigned char result;
	int i, pixelCount, outWidth, outHeight;
	PocoPixel *pixels;

	result = pjpeg_decode_mcu();
	if (0 != result) {
		if (PJPG_NO_MORE_BLOCKS == result)
			return;
		xsErrorPrintf("jpeg read failed");
	}

	xsVars(1);
	xsVar(0) = xsInteger(jpeg->blockX * jpeg->mcuWidth);
	xsSet(jpeg->bitmap, ID(x), xsVar(0));
	xsVar(0) = xsInteger(jpeg->blockY * jpeg->mcuHeight);
	xsSet(jpeg->bitmap, ID(y), xsVar(0));

	if (jpeg->blockY >= jpeg->blockHeight)
		outHeight = jpeg->mcuHeightBottom;
	else
		outHeight = jpeg->mcuHeight;

	if ((jpeg->blockX + 1) == jpeg->blockWidth) {
		outWidth = jpeg->mcuWidthRight;

		jpeg->blockX = 0;
		jpeg->blockY += 1;
	}
	else {
		outWidth = jpeg->mcuWidth;

		jpeg->blockX += 1;
	}

	jpeg->cb->w = outWidth;
	jpeg->cb->h = outHeight;

	pixels = (PocoPixel *)xsToArrayBuffer(jpeg->pixels);

	if (PJPG_YH1V1 == jpeg->scanType) {
		if (jpeg->mcuWidth == outWidth) {		// no horizontal clipping
			for (i = 0, pixelCount = jpeg->mcuWidth * outHeight; i < pixelCount; i++) {
				*pixels++ =		((jpeg->r[i] >> 3) << 11) |
								((jpeg->g[i] >> 2) << 5) |
								((jpeg->b[i] >> 3) << 0);
			}
		}
		else {									// horizontal clipping (right edge)
			int x, y;
			i = 0;
			for (y = 0; y < outHeight; y++) {
				for (x = 0; x < outWidth; x++) {
					*pixels++ =		((jpeg->r[i] >> 3) << 11) |
									((jpeg->g[i] >> 2) << 5) |
									((jpeg->b[i] >> 3) << 0);
					i++;
				}
				i += jpeg->mcuWidth - outWidth;
			}
		}
	}
	else if (PJPG_YH2V2 == jpeg->scanType) {
		xsErrorPrintf("jpeg YH2V2 scan type not yet support");
	}
	else if (PJPG_GRAYSCALE == jpeg->scanType) {
		if (jpeg->mcuWidth == outWidth) {		// no horizontal clipping
			for (i = 0, pixelCount = jpeg->mcuWidth * outHeight; i < pixelCount; i++) {
				PocoPixel gray = ~jpeg->r[i];
				*pixels++ =		((gray >> 3) << 11) |
								((gray >> 2) << 5) |
								((gray >> 3) << 0);
			}
		}
		else {									// horizontal clipping (right edge)
			int x, y;
			i = 0;
			for (y = 0; y < outHeight; y++) {
				for (x = 0; x < outWidth; x++) {
					PocoPixel gray = ~jpeg->r[i];
					*pixels++ =		((gray >> 3) << 11) |
									((gray >> 2) << 5) |
									((gray >> 3) << 0);
					i++;
				}
				i += jpeg->mcuWidth - outWidth;
			}
		}
	}
	else
		xsErrorPrintf("jpeg unsupported scan type");

	xsResult = jpeg->bitmap;
}

unsigned char needBytes(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data)
{
	JPEG jpeg = pCallback_data;
	const unsigned char *buffer;
	xsMachine *the = jpeg->the;

	if (buf_size > (jpeg->length - jpeg->position))
		buf_size = jpeg->length - jpeg->position;

	if (jpeg->isArrayBuffer)
		buffer = xsToArrayBuffer(jpeg->buffer);
	else
		buffer = xsGetHostData(jpeg->buffer);

	memcpy(pBuf, buffer + jpeg->position, buf_size);

	*pBytes_actually_read = buf_size;
	jpeg->position += buf_size;

	return 0;
}
