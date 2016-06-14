/*
    Copyright (C) 2016 Moddable Tech, Inc.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Commodetto render
	
		Graphics rendering object
		Delivers rendered pixels to a PixelOut instance
*/

export default class Render {
	/*
		Constructor binds this renderer to a PixelOut instance
		Provides pixels for rendering into
	*/
	constructor(pixelsOut, dictionary) {
		this.pixelsOut = pixelsOut;
	}

	begin(x = 0, y = 0, width = this.pixelsOut.width - x, height = this.pixelsOut.height - y) {
	}
	end() {
	}
	continue(...coodinates) {
		this.pixelsOut.continue();
		this.end();
		this.begin(...coodinates);
	}
}
