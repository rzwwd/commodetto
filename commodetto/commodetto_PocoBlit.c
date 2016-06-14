/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "commodetto_PocoBlit.h"

#include <string.h>

typedef enum {
	kPocoCommandRectangleFill = 0,		// must start at 0 to match gDrawRenderCommand
	kPocoCommandRectangleBlend,
	kPocoCommandPixelDraw,
	kPocoCommandBitmapDraw,
	kPocoCommandPackedBitmapDrawUnclipped,
	kPocoCommandPackedBitmapDrawClipped,
	kPocoCommandMonochromeBitmapDraw,
	kPocoCommandMonochromeForegroundBitmapDraw,
	kPocoCommandGray16BitmapDraw,
	kPocoCommandBitmapDrawMasked,
	kPocoCommandBitmapPattern,
	kPocoCommandDrawMax = kPocoCommandBitmapPattern + 1
} PocoCommandID;

struct PocoCommandRecord {
	PocoCommandID	command;
	uint8_t			length;			// bytes
	PocoCoordinate	x;
	PocoCoordinate	y;
	PocoDimension	w;
	PocoDimension	h;
};

typedef void (*PocoRenderCommandProc)(Poco poco, PocoCommand pc, PocoPixel *dst, PocoDimension h);

#define PocoReturnIfNoSpace(DISPLAY_LIST_POSITION, bytes) \
	if (((char *)poco->displayListEnd - (char *)(DISPLAY_LIST_POSITION)) < (bytes)) {	\
		poco->displayListOverflow = 1;												\
		return;																		\
	}

#define PocoReturnNULLIfNoSpace(DISPLAY_LIST_POSITION, bytes) \
	if (((char *)poco->displayListEnd - (char *)(DISPLAY_LIST_POSITION)) < (bytes)) {	\
		poco->displayListOverflow = 1;												\
		return NULL;																\
	}

/*
	here begin the functions to build the drawing list
*/

typedef struct ColorDrawRecord {
	PocoPixel	color;
} ColorDrawRecord, *ColorDraw;

typedef struct BlendDrawRecord {
	PocoPixel	color;
	uint8_t		blend;
} BlendDrawRecord, *BlendDraw;

typedef struct RenderBitsRecord {
	void		*pixels;
	uint16_t	rowBump;
} RenderBitsRecord, *RenderBits;

typedef struct RenderPackedBitsUnclippedRecord {
	const void	*pixels;
} RenderPackedBitsUnclippedRecord, *RenderPackedBitsUnclipped;

typedef struct RenderPackedBitsClippedRecord {
	const unsigned char	*pixels;
	PocoCoordinate		left;
	PocoCoordinate		right;
} RenderPackedBitsClippedRecord, *RenderPackedBitsClipped;

typedef struct RenderMonochromeBitsRecord {
	const unsigned char	*pixels;
	uint16_t			rowBump;
	uint8_t				mask;
	uint8_t				mode;
	PocoPixel			fore;
	PocoPixel			back;
} RenderMonochromeBitsRecord, *RenderMonochromeBits;

typedef struct RenderGray16BitsRecord {
	const unsigned char	*pixels;
	uint16_t			rowBump;
	uint8_t				mask;
	PocoPixel			color;
} RenderGray16BitsRecord, *RenderGray16Bits;

typedef struct RenderMaskedBitsRecord {
	const void	*pixels;
	uint16_t	rowBump;

	const void	*maskBits;
	uint8_t		mask;
	uint16_t	maskBump;
} RenderMaskedBitsRecord, *RenderMaskedBits;

typedef struct PatternBitsRecord {
	const void	*pixels;
	int16_t		rowBump;

	const PocoPixel *patternStart;
	uint16_t	xOffset;
	uint16_t	dy;
	uint16_t	patternW;
	uint16_t	patternH;
} PatternBitsRecord, *PatternBits;

void PocoRectangleFill(Poco poco, PocoPixel color, uint8_t blend, PocoCoordinate x, PocoCoordinate y, PocoDimension w, PocoDimension h)
{
    PocoCommand pc = poco->next;
	PocoCoordinate xMax, yMax;

	PocoReturnIfNoSpace(pc, sizeof(PocoCommandRecord) + sizeof(BlendDrawRecord));		// BlendDraw is worst case

	xMax = x + w;
	yMax = y + h;

	if (x < poco->x)
		x = poco->x;

	if (xMax > poco->xMax)
		xMax = poco->xMax;

	if (x >= xMax)
		return;

	w = xMax - x;

	if (y < poco->y)
		y = poco->y;

	if (yMax > poco->yMax)
		yMax = poco->yMax;

	if (y >= yMax)
		return;

	h = yMax - y;

	if (kPocoOpaque == blend) {
		pc->command = kPocoCommandRectangleFill;
		pc->length = sizeof(PocoCommandRecord) + sizeof(ColorDrawRecord);;
		((ColorDraw)(pc + 1))->color = color;
	}
	else {
		BlendDraw bd = (BlendDraw)(pc + 1);

		if (0 == blend)
			return;

		pc->command = kPocoCommandRectangleBlend;
		pc->length = sizeof(PocoCommandRecord) + sizeof(BlendDrawRecord);
		bd->color = color;
		bd->blend = blend >> 3;		// 5 bit blend level
	}
	pc->x = x, pc->y = y, pc->w = w, pc->h = h;

	poco->next = (PocoCommand)(pc->length + (char *)pc);
}

void PocoPixelDraw(Poco poco, PocoPixel color, PocoCoordinate x, PocoCoordinate y)
{
	PocoCommand pc = poco->next;

	PocoReturnIfNoSpace(pc, sizeof(PocoCommandRecord) + sizeof(ColorDrawRecord));

	if ((x < poco->x) || (y < poco->y) || (x >= poco->xMax) || (y >= poco->yMax))
		return;

	pc->command = kPocoCommandPixelDraw;
	pc->length = sizeof(PocoCommandRecord) + sizeof(ColorDrawRecord);
	pc->x = x, pc->y = y, pc->w = 1, pc->h = 1;
	((ColorDraw)(pc + 1))->color = color;

	poco->next = (PocoCommand)(pc->length + (char *)pc);
}

// this function does not work in all cases. it is designed for these two situations
//	1. full scan lines (start to end)
//	2. parsing to the end of a scan line
const unsigned char *skipPackedPixels(const unsigned char *pixels, uint32_t skipPixels)
{  
	while (skipPixels) {
		signed char opcode = READ_PROG_MEM_UNSIGNED_BYTE(pixels++);
		unsigned char count = (opcode & 0x3F) + 1;
		if (opcode < 0) {
			if (!(opcode & 0x40))	// repeat (not skip)
				pixels += sizeof(PocoPixel);
		}
		else						// literal
			pixels += count * sizeof(PocoPixel);
		skipPixels -= count;
	}
	return pixels;
}

PocoCommand PocoBitmapDraw(Poco poco, PocoBitmap bits, PocoCoordinate x, PocoCoordinate y, PocoDimension sx, PocoDimension sy, PocoDimension sw, PocoDimension sh)
{
    const PocoPixel *pixels;
    int16_t d;
    PocoCommand pc = poco->next;

	PocoReturnNULLIfNoSpace(pc, sizeof(PocoCommandRecord) + sizeof(RenderMonochromeBitsRecord));

	if ((x >= poco->xMax) || (y >= poco->yMax))
		return NULL;

	if (x < poco->x) {
		d = poco->x - x;
		if (sw <= d)
			return NULL;
		sx += d;
		sw -= d;
		x = poco->x;
	}

	if (y < poco->y) {
		d = poco->y - y;
		if (sh <= d)
			return NULL;
		sy += d;
		sh -= d;
		y = poco->y;
	}

	if ((x + sw) > poco->xMax)
		sw = poco->xMax - x;

	if ((y + sh) > poco->yMax)
		sh = poco->yMax - y;

	if ((sx >= bits->width) || (sy >= bits->height) || ((sx + sw) > bits->width) || ((sy + sh) > bits->height) || !sw || !sh)
		return NULL;

	pixels = bits->pixels;

    pc->x = x, pc->y = y, pc->w = sw, pc->h = sh;

	if (kCommodettoBitmapRaw == bits->format) {
		pc->command = kPocoCommandBitmapDraw;
		pc->length = sizeof(PocoCommandRecord) + sizeof(RenderBitsRecord);
		((RenderBits)(pc + 1))->pixels = (void *)(pixels + (sy * bits->width) + sx);
		((RenderBits)(pc + 1))->rowBump = bits->width;
	}
    else
    if (kCommodettoBitmapPacked == bits->format) {
       PocoCoordinate right = bits->width - sx - sw;

       if (sy)
         pixels = (const PocoPixel *)skipPackedPixels((const unsigned char *)pixels, bits->width * sy);  // clipped off top. skip sy scan lines.

        if (right || sx) {
			pc->command = kPocoCommandPackedBitmapDrawClipped;
			pc->length = sizeof(PocoCommandRecord) + sizeof(RenderPackedBitsClippedRecord);
			((RenderPackedBitsClipped)(pc + 1))->left = sx;
			((RenderPackedBitsClipped)(pc + 1))->right = right;
        }
		else {
			pc->command = kPocoCommandPackedBitmapDrawUnclipped;
			pc->length = sizeof(PocoCommandRecord) + sizeof(RenderPackedBitsUnclippedRecord);
		}

		((RenderPackedBitsClipped)(pc + 1))->pixels = (const unsigned char *)pixels;
    }
    else
    if (kCommodettoBitmapMonochrome == bits->format) {
		uint8_t mask;
		RenderMonochromeBits srcBits = (RenderMonochromeBits)(pc + 1);

		pixels = (PocoPixel *)(((char *)pixels) + ((bits->width + 7) >> 3) * sy);  // clipped off top. skip sy scan lines.
		pixels = (PocoPixel *)(((char *)pixels) + (sx >> 3));
		sx &= 0x07;
		mask = 1 << (7 - sx);

		pc->command = kPocoCommandMonochromeBitmapDraw;
		pc->length = sizeof(PocoCommandRecord) + sizeof(RenderMonochromeBitsRecord);
		srcBits->pixels = (const unsigned char *)pixels;
		srcBits->rowBump = ((bits->width + 7) >> 3) - (((sx + sw) + 7) >> 3);
		srcBits->mask = mask;
		srcBits->mode = kPocoMonochromeForeAndBackground;	// mode, fore, amd back are patched in PocoMonochromeBitmapDraw
		srcBits->fore = 0x00;
		srcBits->back = ~0;
    }
    else
    if (kCommodettoBitmapGray16 == bits->format) {
		uint8_t mask;
		RenderGray16Bits srcBits = (RenderGray16Bits)(pc + 1);

		pixels = (PocoPixel *)(((char *)pixels) + ((bits->width + 1) >> 1) * sy);  // clipped off top. skip sy scan lines.
		pixels = (PocoPixel *)(((char *)pixels) + (sx >> 1));
		mask = (sx & 1) ? 0x0F : 0xF0;

		pc->command = kPocoCommandGray16BitmapDraw;
		pc->length = sizeof(PocoCommandRecord) + sizeof(RenderGray16BitsRecord);
		srcBits->pixels = (const unsigned char *)pixels;
		srcBits->rowBump = ((bits->width + 1) >> 1) - ((((sx & 1) + sw) + 1) >> 1);
		srcBits->mask = mask;
		srcBits->color = 0x00;
    }
    else
		return NULL;

	poco->next = (PocoCommand)(pc->length + (char *)pc);

	return pc;
}

void PocoMonochromeBitmapDraw(Poco poco, PocoBitmap bits, PocoMonochromeMode mode, PocoPixel fgColor, PocoPixel bgColor, PocoCoordinate x, PocoCoordinate y, PocoDimension sx, PocoDimension sy, PocoDimension sw, PocoDimension sh)
{
	PocoCommand pc = PocoBitmapDraw(poco, bits, x, y, sx, sy, sw, sh);

	if (kCommodettoBitmapMonochrome != bits->format)
		return;

	if (pc) {
		RenderMonochromeBits rmb = (RenderMonochromeBits)(sizeof(PocoCommandRecord) + (char *)pc);
		rmb->mode = mode;
		rmb->fore = fgColor;
		rmb->back = bgColor;
		if (kPocoMonochromeForeground == mode)
			pc->command = kPocoCommandMonochromeForegroundBitmapDraw;
	}
}

void PocoGrayBitmapDraw(Poco poco, PocoBitmap bits, PocoPixel color, PocoCoordinate x, PocoCoordinate y, PocoDimension sx, PocoDimension sy, PocoDimension sw, PocoDimension sh)
{
	PocoCommand pc = PocoBitmapDraw(poco, bits, x, y, sx, sy, sw, sh);

	if (kCommodettoBitmapGray16 != bits->format)
		return;

	if (pc) {
		RenderGray16Bits rg16 = (RenderGray16Bits)(sizeof(PocoCommandRecord) + (char *)pc);
		rg16->color = color;
	}
}

void PocoBitmapDrawMasked(Poco poco, PocoBitmap bits, PocoCoordinate x, PocoCoordinate y, PocoDimension sx, PocoDimension sy, PocoDimension sw, PocoDimension sh,
			PocoBitmap mask, PocoDimension mask_sx, PocoDimension mask_sy)
{
	const uint8_t *maskBits;
	PocoCoordinate xMax, yMax;
    PocoCommand pc = poco->next;

	PocoReturnIfNoSpace(pc, sizeof(PocoCommandRecord) + sizeof(RenderMaskedBitsRecord));

	xMax = x + sw;
	yMax = y + sh;

	if (x < poco->x) {
		sx += (poco->x - x);
		mask_sx += (poco->x - x);
		x = poco->x;
	}

	if (xMax > poco->xMax)
		xMax = poco->xMax;

	if (x >= xMax)
		return;

	sw = xMax - x;

	if (y < poco->y) {
		sy += (poco->y - y);
		mask_sy += (poco->y - y);
		y = poco->y;
	}

	if (yMax > poco->yMax)
		yMax = poco->yMax;

	if (y >= yMax)
		return;

	sh = yMax - y;

	if ((mask_sx >= mask->width) || ((mask_sx + sw) > mask->width) || (mask_sy >= mask->height) || ((mask_sy + sh) > mask->height))
		return;

    pc->x = x, pc->y = y, pc->w = sw, pc->h = sh;

	pc->command = kPocoCommandBitmapDrawMasked;
	pc->length = sizeof(PocoCommandRecord) + sizeof(RenderMaskedBitsRecord);
	((RenderMaskedBits)(pc + 1))->pixels = (void *)(bits->pixels + (sy * bits->width) + sx);
	((RenderMaskedBits)(pc + 1))->rowBump = bits->width - sw;

	maskBits = (((const uint8_t *)mask->pixels) + ((mask->width + 1) >> 1) * mask_sy);  // clipped off top. skip sy scan lines.
	maskBits += (mask_sx >> 1);

	((RenderMaskedBits)(pc + 1))->maskBits = maskBits;
	((RenderMaskedBits)(pc + 1))->maskBump = ((mask->width + 1) >> 1) - ((((mask_sx & 1) + sw) + 1) >> 1);
	((RenderMaskedBits)(pc + 1))->mask = (mask_sx & 1) ? 0x0F : 0xF0;

	poco->next = (PocoCommand)(pc->length + (char *)pc);
}

void PocoBitmapPattern(Poco poco, PocoBitmap bits, PocoCoordinate x, PocoCoordinate y, PocoDimension w, PocoDimension h, PocoDimension sx, PocoDimension sy, PocoDimension sw, PocoDimension sh)
{
	PocoCoordinate xMax, yMax;
    PocoCommand pc = poco->next;
	PatternBits pb;

	PocoReturnIfNoSpace(pc, sizeof(PocoCommandRecord) + sizeof(PatternBitsRecord));

	pc->command = kPocoCommandBitmapPattern;
	pc->length = sizeof(PocoCommandRecord) + sizeof(PatternBitsRecord);
	pb = (PatternBits)(pc + 1);
	pb->rowBump = bits->width - sw;
	pb->patternStart = ((PocoPixel *)bits->pixels) + (sy * bits->width) + sx;
	pb->xOffset = 0;
	pb->dy = 0;
	pb->patternW = sw;
	pb->patternH = sh;

	xMax = x + w;
	yMax = y + h;

	if (x < poco->x) {
		pb->xOffset = (poco->x - x) % sw;
		sx += pb->xOffset;
		x = poco->x;
	}

	if (xMax > poco->xMax)
		xMax = poco->xMax;

	if (x >= xMax)
		return;

	w = xMax - x;

	if (y < poco->y) {
		pb->dy = (poco->y - y) % sh;
		sy += pb->dy;
		y = poco->y;
	}

	if (yMax > poco->yMax)
		yMax = poco->yMax;

	if (y >= yMax)
		return;

	h = yMax - y;

    pc->x = x, pc->y = y, pc->w = w, pc->h = h;

	pb->pixels = pb->patternStart + (pb->dy * bits->width) + pb->xOffset;
	pb->patternStart += pb->xOffset;
	pb->rowBump += pb->patternW - ((pb->xOffset + w) % pb->patternW) + pb->xOffset;

	poco->next = (PocoCommand)(pc->length + (char *)pc);
}

/*
	here begin the functions to render the drawing list
*/

void doFillRectangle(Poco poco, PocoCommand pc, PocoPixel *dst, PocoDimension h)
{
	PocoCoordinate w = pc->w;
	PocoCoordinate rowBump = poco->w - w;
	PocoPixel color = ((ColorDraw)(pc + 1))->color;

	while (h--) {
		PocoCoordinate tw = w;

		while (tw >= 4) {
			*dst++ = color;
			*dst++ = color;
			*dst++ = color;
			*dst++ = color;
			tw -= 4;
		}
		while (tw--)
			*dst++ = color;
		dst += rowBump;
	}
}

// based on FskFastBlend565SE - one multiply per pixel
void doBlendRectangle(Poco poco, PocoCommand pc, PocoPixel *d, PocoDimension h)
{
	PocoCoordinate w = pc->w;
	PocoCoordinate rowBump = poco->w - w;
	BlendDraw bd = (BlendDraw)(pc + 1);
	uint8_t blend = bd->blend;		// 5 bit blend level
	int src32;

	src32 = bd->color;
	src32 |= src32 << 16;
	src32 &= 0x07E0F81F;

	while (h--) {
		PocoCoordinate tw = w;

		while (tw--) {
			int	dst, src;

			dst = *d;
			dst |= dst << 16;
			dst &= 0x07E0F81F;
			src = src32 - dst;
			dst = blend * src + (dst << 5) - dst;
			dst += 0x02008010;
			dst += (dst >> 5) & 0x3E0F81F;
			dst >>= 5;
			dst &= 0x07E0F81F;
			dst |= dst >> 16;
			*d++ = (PocoPixel)dst;
		}
		d += rowBump;
	}
}

void doDrawPixel(Poco poco, PocoCommand pc, PocoPixel *dst, PocoDimension h)
{
   *dst = ((ColorDraw)(pc + 1))->color;
;
}

void doDrawBitmap(Poco poco, PocoCommand pc, PocoPixel *dst, PocoDimension h)
{
	RenderBits srcBits = (RenderBits)(pc + 1);
	PocoPixel *src = srcBits->pixels;
	PocoCoordinate srcBump = srcBits->rowBump;
	int count = pc->w * sizeof(PocoPixel);

	while (h--) {
		memcpy(dst, src, count);    // inline could be faster? (seems no)
		dst += poco->w;
		src += srcBump;
	}
	srcBits->pixels = src;
}

void doDrawPackedBitmapUnclipped(Poco poco, PocoCommand pc, PocoPixel *dst, PocoDimension h)
{
	RenderPackedBitsUnclipped srcBits = (RenderPackedBitsUnclipped)(pc + 1);
	const unsigned char *src = srcBits->pixels;
	PocoCoordinate w = pc->w;
	PocoCoordinate scanBump = poco->w - w;

	while (h--) {
		PocoCoordinate tw = w;
		while (tw) {
			signed char opcode = READ_PROG_MEM_UNSIGNED_BYTE(src++);
			unsigned char count = (opcode & 0x3F) + 1;

			// blit
			if (opcode < 0) {
				if (!(opcode & 0x40)) {   // repeat
					unsigned char c = count;
					unsigned char *d = (unsigned char *)dst;
					unsigned char color = READ_PROG_MEM_UNSIGNED_BYTE(src++);
					#if (8 == kPocoPixelSize)
						while (c--)
							*d++ = color;
					#else
						unsigned char color2 = READ_PROG_MEM_UNSIGNED_BYTE(src++);
						while (c--) {
							*d++ = color;
							*d++ = color2;
						}
					#endif
				}
			}
			else {    // literal
				unsigned char c = count;
				unsigned char *d = (unsigned char *)dst;

				while (c--) {
					*d++ = READ_PROG_MEM_UNSIGNED_BYTE(src++);
					#if (16 == kPocoPixelSize)
						*d++ = READ_PROG_MEM_UNSIGNED_BYTE(src++);
					#endif
				}
			}
			tw -= count;
			dst += count;
		}
		dst += scanBump;
	}

	srcBits->pixels = (const void *)src;
}

void doDrawPackedBitmapClipped(Poco poco, PocoCommand pc, PocoPixel *dst, PocoDimension h)
{
	RenderPackedBitsClipped srcBits = (RenderPackedBitsClipped)(pc + 1);
	const unsigned char *src = srcBits->pixels;
	PocoCoordinate w = pc->w;
	PocoCoordinate scanBump = poco->w - w;

	while (h--) {
		PocoCoordinate tw = w, left = srcBits->left, right = srcBits->right;
		while (tw) {
			signed char opcode = READ_PROG_MEM_UNSIGNED_BYTE(src++);
			unsigned char count = (opcode & 0x3F) + 1, blitCount;

			// left clip
			if (left) {
				if (left >= count) {  // consume this entire run
					if (opcode < 0) {
						if (!(opcode & 0x40))  // repeat
							src += sizeof(PocoPixel);
					}
					else    // literal
						src += count * sizeof(PocoPixel);
					left -= count;
					continue;
				}

				// partial run
				if (opcode >= 0)  // literal
					src += left * sizeof(PocoPixel);
				count -= left;
				left = 0;
				// fall through to blit this opcode
			}

			// right clip
			blitCount = count;
			if (count > tw) {
				right -= (count - tw);
				blitCount = tw;
			}

            // blit
			if (opcode < 0) {
				if (!(opcode & 0x40)) {  // repeat
					unsigned char c = blitCount;
					unsigned char *d = (unsigned char *)dst;
					unsigned char color = READ_PROG_MEM_UNSIGNED_BYTE(src++);
					#if (8 == kPocoPixelSize)
						while (c--)
							*d++ = color;
					#else
						unsigned char color2 = READ_PROG_MEM_UNSIGNED_BYTE(src++);
						while (c--) {
							*d++ = color;
							*d++ = color2;
						}
					#endif
				}
			}
			else {    // literal
				unsigned char c = blitCount;
				unsigned char *d = (unsigned char *)dst;

				while (c--) {
					*d++ = READ_PROG_MEM_UNSIGNED_BYTE(src++);
					#if (16 == kPocoPixelSize)
						*d++ = READ_PROG_MEM_UNSIGNED_BYTE(src++);
					#endif
				}

				src += (count - blitCount) * sizeof(PocoPixel);
			}
			tw -= blitCount;
			dst += blitCount;
		}

		if (right)
			src = skipPackedPixels(src, right);

  	   dst += scanBump;
	}

	srcBits->pixels = src;
}

void doDrawMonochromeBitmapPart(Poco poco, PocoCommand pc, PocoPixel *dst, PocoDimension h)
{
	RenderMonochromeBits srcBits = (RenderMonochromeBits)(pc + 1);
	const uint8_t *src = srcBits->pixels;
	uint8_t mask = srcBits->mask;
	PocoCoordinate w = pc->w;
	PocoCoordinate scanBump = poco->w - w;
	PocoPixel fore = srcBits->fore, back = srcBits->back, drawFore = (srcBits->mode & kPocoMonochromeForeground) != 0, drawBack = (srcBits->mode & kPocoMonochromeBackground) != 0;

	while (h--) {
		PocoCoordinate tw = w;
		uint8_t bits = READ_PROG_MEM_UNSIGNED_BYTE(src++);
		uint8_t tm = mask;

		while (tw--) {
			if (0 == tm) {
				tm = 0x80;
				bits = READ_PROG_MEM_UNSIGNED_BYTE(src++);
			}

			if (bits & tm) {
				if (drawFore)
					*dst = fore;
			}
			else
			if (drawBack)
				*dst = back;
			dst += 1;

			tm >>= 1;
		}

		src += srcBits->rowBump;
		dst += scanBump;
	}

	srcBits->pixels = src;
}

void doDrawMonochromeForegroundBitmapPart(Poco poco, PocoCommand pc, PocoPixel *dst, PocoDimension h)
{
	RenderMonochromeBits srcBits = (RenderMonochromeBits)(pc + 1);
	const uint8_t *src = srcBits->pixels;
	uint8_t mask = srcBits->mask;
	PocoCoordinate w = pc->w;
	PocoCoordinate scanBump = poco->w - w;
	PocoPixel fore = srcBits->fore;

	while (h--) {
		PocoCoordinate tw = w;
		uint8_t bits = READ_PROG_MEM_UNSIGNED_BYTE(src++);
		uint8_t tm = mask;

		while (tw--) {
			if (0 == tm) {
				tm = 0x80;
				bits = READ_PROG_MEM_UNSIGNED_BYTE(src++);
			}

			if (bits & tm)
				*dst = fore;
			dst += 1;

			tm >>= 1;
		}

		src += srcBits->rowBump;
		dst += scanBump;
	}

	srcBits->pixels = src;
}

void doDrawGray16BitmapPart(Poco poco, PocoCommand pc, PocoPixel *d, PocoDimension h)
{
	RenderGray16Bits srcBits = (RenderGray16Bits)(pc + 1);
	const uint8_t *src = srcBits->pixels;
	uint8_t mask = srcBits->mask;
	PocoCoordinate w = pc->w;
	PocoCoordinate scanBump = poco->w - w;
	int src32;

	src32 = srcBits->color;
	src32 |= src32 << 16;
	src32 &= 0x07E0F81F;

	while (h--) {
		PocoCoordinate tw = w;
		uint8_t bits = ~READ_PROG_MEM_UNSIGNED_BYTE(src++);
		uint8_t tm = mask;

		while (tw--) {
			if (0 == tm) {
				tm = 0xF0;
				bits = ~READ_PROG_MEM_UNSIGNED_BYTE(src++);
			}

			if (bits & tm) {
				int blend = (tm & 1) ? (bits & 0x0F) : (bits >> 4);
				if (15 == blend)
					*d = (PocoPixel)srcBits->color;
				else {
					int src, dst;

					blend = (blend << 1) | (blend >> 3);		// 5-bit blend

					dst = *d;
					dst |= dst << 16;
					dst &= 0x07E0F81F;
					src = src32 - dst;
					dst = blend * src + (dst << 5) - dst;
					dst += 0x02008010;
					dst += (dst >> 5) & 0x3E0F81F;
					dst >>= 5;
					dst &= 0x07E0F81F;
					dst |= dst >> 16;
					*d = (PocoPixel)dst;
				}
			}
			d += 1;

			tm >>= 4;
		}

		src += srcBits->rowBump;
		d += scanBump;
	}

	srcBits->pixels = src;
}

void doDrawMaskedBitmap(Poco poco, PocoCommand pc, PocoPixel *d, PocoDimension h)
{
	RenderMaskedBits rmb = (RenderMaskedBits)(pc + 1);
	const PocoPixel *src = rmb->pixels;
	const uint8_t *maskBits = rmb->maskBits;
	uint8_t mask = rmb->mask;
	PocoCoordinate w = pc->w;
	PocoCoordinate scanBump = poco->w - w;

	while (h--) {
		PocoCoordinate tw = w;
		uint8_t bits = ~READ_PROG_MEM_UNSIGNED_BYTE(maskBits++);
		uint8_t tm = mask;

		while (tw--) {
			if (0 == tm) {
				tm = 0xF0;
				bits = ~READ_PROG_MEM_UNSIGNED_BYTE(maskBits++);
			}

			if (bits & tm) {
				int blend = (tm & 1) ? (bits & 0x0F) : (bits >> 4);
				if (15 == blend)
					*d = *src;
				else {
					int src32, dst;

					src32 = *src;
					src32 |= src32 << 16;
					src32 &= 0x07E0F81F;

					blend = (blend << 1) | (blend >> 3);		// 5-bit blend

					dst = *d;
					dst |= dst << 16;
					dst &= 0x07E0F81F;
					dst = blend * (src32 - dst) + (dst << 5) - dst;
					dst += 0x02008010;
					dst += (dst >> 5) & 0x3E0F81F;
					dst >>= 5;
					dst &= 0x07E0F81F;
					dst |= dst >> 16;
					*d = (PocoPixel)dst;
				}
			}
			src += 1;
			d += 1;

			tm >>= 4;
		}

		src += rmb->rowBump;
		maskBits += rmb->maskBump;
		d += scanBump;
	}

	rmb->pixels = src;
	rmb->maskBits = maskBits;
}

void doDrawPattern(Poco poco, PocoCommand pc, PocoPixel *dst, PocoDimension h)
{
	PatternBits pb = (PatternBits)(pc + 1);
	const PocoPixel *src = pb->pixels;

	while (h--) {
		PocoCoordinate w = pc->w;
		PocoPixel *d = dst;
		uint16_t dx = pb->xOffset;

		while (w) {
			PocoCoordinate count = w;

			if (count > (pb->patternW - dx))
				count = (pb->patternW - dx);
			memcpy(d, src, count * sizeof(PocoPixel));

			d += count;
			src += count;
			w -= count;

			dx += count;
			if (dx == pb->patternW) {
				dx = 0;
				src -= pb->patternW;
			}
		}

		src += pb->rowBump;

		pb->dy += 1;
		if (pb->dy == pb->patternH) {
			pb->dy = 0;
			src = pb->patternStart;
		}

		dst += poco->w;
	}

	pb->pixels = src;
}

/*
	vector of drawing functions to avoid switch in dispatch
*/

static const PocoRenderCommandProc gDrawRenderCommand[kPocoCommandDrawMax] = {
	doFillRectangle,
	doBlendRectangle,
	doDrawPixel,
	doDrawBitmap,
	doDrawPackedBitmapUnclipped,
	doDrawPackedBitmapClipped,
	doDrawMonochromeBitmapPart,
	doDrawMonochromeForegroundBitmapPart,
	doDrawGray16BitmapPart,
	doDrawMaskedBitmap,
	doDrawPattern
};

void PocoDrawingBegin(Poco poco, PocoCoordinate x, PocoCoordinate y, PocoDimension w, PocoDimension h)
{
	poco->next = (PocoCommand)poco->displayList;
	poco->displayListOverflow = 0;
	poco->stackProblem = 0;
	poco->stackDepth = 0;
	poco->xOrigin = poco->yOrigin = 0;

	poco->x = x;
	poco->y = y;
	poco->w = w;
	poco->h = h;

	poco->xMax = x + w;
	poco->yMax = y + h;
 }

/*
	the subtract of poco->x the inner loop could be moved to the Command record builder
*/

int PocoDrawingEnd(Poco poco, PocoPixel *pixels, int byteLength, PocoRenderedPixelsReceiver pixelReceiver, void *refCon)
{
	PocoCoordinate yMin, yMax;
	int16_t rowBytes;
	int16_t displayLines;
	PocoCommand displayList, displayListEnd;

	if (poco->displayListOverflow)
		return 1;

	if (poco->stackDepth)
		return 2;

	if (poco->stackProblem)
		return 3;

	rowBytes = poco->w * sizeof(PocoPixel);

	displayLines = byteLength / rowBytes;
	if (displayLines <= 0) return 0;

	// walk through a slab of displayList at a time
	displayList = (PocoCommand)poco->displayList;
	displayListEnd = poco->next;
	for (yMin = poco->y; yMin < poco->yMax; yMin = yMax) {
		PocoCommand walker;

		yMax = yMin + displayLines;
		if (yMax > poco->yMax)
			yMax = poco->yMax;

		for (walker = displayList; walker != displayListEnd; walker = (PocoCommand)(walker->length + (char *)walker)) {
			PocoCoordinate y = walker->y;
			PocoDimension h;

			if (y >= yMax)
				continue;						// completely below the slab being drawn

			h = walker->h;
			if ((y + h) <= yMin) {				// full object above the slab being drawn
				walker->y = poco->yMax;			// move it below bottom, so this loop can exit early next time
				continue;
			}

			if (y < yMin) {
				h -= (yMin - y);
				y = yMin;
			}
			if ((y + h) > yMax)
				h = yMax - y;

			(gDrawRenderCommand[walker->command])(poco, walker, pixels + ((y - yMin) * poco->w) + (walker->x - poco->x), h);
		}

		(pixelReceiver)(pixels, rowBytes * (yMax - yMin), refCon);
	}

	return 0;
}

void PocoClipPush(Poco poco, PocoCoordinate x, PocoCoordinate y, PocoDimension w, PocoDimension h)
{
	PocoRectangle clip;
	PocoCoordinate xMax, yMax;

	if (poco->pixelsLength < (sizeof(PocoRectangleRecord) * (poco->stackDepth + 1))) {
		poco->stackProblem = 1;
		return;
	}

	clip = ((PocoRectangle)poco->pixels) + poco->stackDepth++;

	// save current clip
	clip->x = poco->x;
	clip->y = poco->y;
	clip->w = poco->w;
	clip->h = poco->h;

	// intersect new clip with current clip
	xMax = x + w;
	yMax = y + h;

	if (x < poco->x)
		x = poco->x;

	if (xMax > poco->xMax)
		xMax = poco->xMax;

	if (y < poco->y)
		y = poco->y;

	if (yMax > poco->yMax)
		yMax = poco->yMax;

	if ((x >= xMax) || (y >= yMax)) {
		// clipped out
		poco->w = 0;
		poco->h = 0;
		return;
	}

	// apply new clip
	poco->x = x;
	poco->y = y;
	poco->w = xMax - x;
	poco->h = yMax - y;
	poco->xMax = xMax;
	poco->yMax = yMax;
}

void PocoClipPop(Poco poco)
{
	PocoRectangle clip;

	if (0 == poco->stackDepth) {
		poco->stackProblem = 1;
		return;
	}

	clip = ((PocoRectangle)poco->pixels) + --poco->stackDepth;

	poco->x = clip->x;
	poco->y = clip->y;
	poco->w = clip->w;
	poco->h = clip->h;
	poco->xMax = clip->x + clip->w;
	poco->yMax = clip->y + clip->h;
}

void PocoOriginPush(Poco poco, PocoCoordinate x, PocoCoordinate y)
{
	PocoRectangle clip;

	if (poco->pixelsLength < (sizeof(PocoRectangleRecord) * (poco->stackDepth + 1))) {
		poco->stackProblem = 1;
		return;
	}

	clip = ((PocoRectangle)poco->pixels) + poco->stackDepth++;

	// save current origin
	clip->x = poco->xOrigin;
	clip->y = poco->yOrigin;
	clip->w = clip->h = ~0;

	// apply new origin
	poco->xOrigin += x;
	poco->yOrigin += y;
}

void PocoOriginPop(Poco poco)
{
	PocoRectangle clip;

	if (0 == poco->stackDepth) {
		poco->stackProblem = 1;
		return;
	}

	clip = ((PocoRectangle)poco->pixels) + --poco->stackDepth;

	poco->xOrigin = clip->x;
	poco->yOrigin = clip->y;
}



