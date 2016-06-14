/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "commodetto_Bitmap.h"

#include <stdio.h>
#include <stdlib.h>

#include "mc_xs.h"
#include "mc_misc.h"

void xs_Bitmap_destructor(void *data)
{
	if (data)
		mc_free(data);
}

void xs_Bitmap(xsMachine *the)
{
	CommodettoBitmap cb = mc_malloc(sizeof(CommodettoBitmapRecord));
	int offset;

	if (!cb)
		xsErrorPrintf("no memory for bitmap info");

	xsSetHostData(xsThis, cb);

	cb->w = xsToInteger(xsArg(0));
	cb->h = xsToInteger(xsArg(1));
	cb->format = xsToInteger(xsArg(2));
	offset = xsToInteger(xsArg(4));

	if (xsIsInstanceOf(xsArg(3), xsArrayBufferPrototype)) {
		cb->havePointer = false;
		cb->bits.offset = offset;
	}
	else {
		cb->havePointer = true;
		cb->bits.data = offset + (char *)xsGetHostData(xsArg(3));
	}

	xsSet(xsThis, xsID("buffer"), xsArg(3));

	if (0 == cb->format)
		xsErrorPrintf("invalid bitmap format");
}

void xs_bitmap_get_width(xsMachine *the)
{
	CommodettoBitmap cb = xsGetHostData(xsThis);
	xsResult = xsInteger(cb->w);
}

void xs_bitmap_get_height(xsMachine *the)
{
	CommodettoBitmap cb = xsGetHostData(xsThis);
	xsResult = xsInteger(cb->h);
}

void xs_bitmap_get_format(xsMachine *the)
{
	CommodettoBitmap cb = xsGetHostData(xsThis);
	xsResult = xsInteger(cb->format);
}
