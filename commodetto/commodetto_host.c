/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdio.h>
#include <stdlib.h>

#include "mc_xs.h"

void destructor(void *data)
{
	if (data)
		free(data);
}

void xs_host_loadFile(xsMachine *the)
{
	const char *path = xsToString(xsArg(0));
	FILE *font;
	int size;
	char *buffer;

	font = fopen(path, "rb");
	if (!font)
		xsErrorPrintf("can't open file");
	fseek(font, 0, SEEK_END);
	size = ftell(font);
	buffer = malloc(size);
	if (!buffer) {
		fclose(font);
		xsErrorPrintf("can't allocate file memory");
	}

	fseek(font, 0, SEEK_SET);
	fread(buffer, 1, size, font);
	fclose(font);

	xsResult = xsNewHostObject(destructor);
	xsSetHostData(xsResult, buffer);
	xsVars(1);
	xsVar(0) = xsInteger(size);
	xsSet(xsResult, xsID("byteLength"), xsVar(0));
}
