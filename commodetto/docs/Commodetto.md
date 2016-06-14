# Commodetto
Copyright 2016 Moddable Tech, Inc.

Revised: June 13, 2016

Commodetto is a graphics library designed to bring modern user interface rendering to devices powered by a resource constrained micro-controller. For many applications, only a few kilobytes of RAM are needed by Commodetto, including the assets for rendering the user interface.

Commodetto consists of several parts:

* Lightweight Poco rendering engine, a display list renderer able to efficiently render a single scan line at a time, eliminating the need for a frame buffer
* Asset loaders for working with common file formats
* Pixel outputs for delivering rendered pixels to displays and files
* JavaScript API for all features

Every part of Commodetto is a module, making it straightforward to deploy only necessary modules, add new modules, and replace existing modules. Even the rendering capabilities are a module, allowing specialized rendering modules to be integrated.

The Poco rendering engine contains only the most essential rendering operations: fill or blend a rectangle with a solid color, and a small set of bitmap drawing operations including copy, pattern fill, and alpha blend. Text is implemented externally to the rendering engine using Poco bitmap rendering operations which allows integration of different engines for glyph generation, text layout, and text measurement.

Asset loaders prepare graphical assets, such as photos, user interface elements, and fonts, for rendering. Commodetto includes asset loaders for BMP images, JPEG photos, NFNT fonts, and BMFont fonts. Additional asset loader modules may be added. The asset loaders allow many types of assets to be rendered directly from flash storage (or ROM) without having to be loaded into RAM.

Pixel outputs deliver rendered pixels to their destination. Commodetto includes modules to output to in-memory bitmaps and files. Modules may be added to send the pixels to a display over the transports supported by the host device, including SPI, I2C, serial, and memory mapped port.

### Native data types

#### Pixel
To run well on resource-constrained hardware, Commodetto makes simplifying assumptions. One assumption is that only a single output pixel format is supported in a given deployment. A general purpose graphics library needs to support many different output pixel formats to be compatible with the many different displays and file formats in use. Commodetto assumes the device it is deployed to connects to a single type of screen. This assumption reduces the code size and reduces some runtime overhead.

Commodetto has the ability to support different pixel formats, with the choice of the pixel format to use set at compile time. To change the output pixel format, for example to use a different screen, simply requires recompiling Commodetto with a different output pixel format configured.

`CommodettoPixel` is the data type for pixel.

```c
typedef uint16_t CommodettoPixel;
```

The initial version of Commodetto supports 16-bit output pixels, in 565 RGB arrangement.

#### Pixel format
Commodetto implements support for multiple source pixel formats at the same time. This allows for efficient storage and rendering of different kinds of assets.

The output pixel format is always one of the supported source pixel formats. Other supported source pixel formats include 1-bit monochrome and 4-bit gray.

The `CommodettoBitmapFormat` enumeration defines the available pixel formats.

```c
typedef enum {
	kCommodettoBitmapRaw = 1,
	kCommodettoBitmapPacked,
	kCommodettoBitmapMonochrome,
	kCommodettoBitmapGray16
} CommodettoBitmapFormat;
```

* **Raw** - output pixel format. The format varies depending on how Commodetto is built.
* **Packed** - pixels are in the output pixel format, and packed together using an RLE algorithm for efficient storage and rendering. The encoding algorithm is described in the source code file `commodetto_RLEOut.js`.
* **Monochrome** - pixels are 1-bit data, packed into bytes where the high bit of the byte is the left-most pixel.
* **Gray16** - pixels are 4-bit data, where 0 represents white, 15 represents black, and the values between are proportionally interpolated gray levels. The pixels are packed into bytes, where the high nybble is the left-most pixel.

#### Coordinate

`CommodettoCoordinate` is the type used to specify coordinates:

```c
typedef int16_t CommodettoCoordinate;
```

The coordinate type is signed to allow applications to render objects that overlap the screen on any edge.

For devices with very small displays (128 pixels or less in each dimension) `CommodettoCoordinate` may be redefined as an 8-bit value, e.g. `int8_t`, to reduce memory requirements. When doing this, application code must take care not to use coordinates smaller than -128 or larger than 127 or the results are unpredictable.

#### Dimension

`CommodettoDimension` is the type used to specify dimensions:

```c
typedef uint16_t CommodettoDimension;
```

The dimension type is unsigned, as Commodetto assigns no meaning to a negative width or height.

`CommodettoDimension` may be redefined as `uint8_t` on devices with very small displays (256 pixels in each dimension) to reduce memory use. When doing this, application code must take care not to specify dimensions larger than 255 pixels.

#### Bitmap

The `CommodettoBitmap` structure defines a bitmap:

```c
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
```

The `w` and `h` fields are the width and height of the bitmap in pixels. The coordinates of the top left corner of a bitmap are always {x: 0, y: 0} so the bitmap does not contain x and y coordinates. The `format` field indicates the pixel format of the pixels in the bitmap.

The remaining fields indicate where the pixel data is located. If the `havePointer` field is non-zero, then the `data` field in the union is valid and points to the pixel data:

```c
CommodettoBitmap bitmap = /* get bitmap */;
CommodettoPixel *pixels = bitmap.bits.data;
```

If the `havePointer` field is zero, the address of the `pixels` is stored external to this bitmap data structure. The offset field indicates the number of bytes from that external pixels pointer that the pixel data begins. The `offset` is commonly used to calculate the address of pixels stored in an `ArrayBuffer`:

```c
CommodettoBitmap bitmap = /* get bitmap */;
unsigned char *buffer = xsToArrayBuffer(xsArg(0));
CommodettoPixel *pixels = (CommodettoPixel *)(bitmap.bits.offset + buffer);
```

### Host buffers

The built-in `ArrayBuffer` buffer object is the standard way to store and manipulate binary data in ES6 JavaScript. Commodetto supports storing the pixels of assets in an `ArrayBuffer`. However, the ES6 standard effectively requires an ArrayBuffer to reside in RAM. Because many embedded devices have limited RAM but relatively generous ROM (flash memory), it is desirable to use assets directly from ROM without first moving them to RAM.

To support this situation, the XS6 JavaScript engine Commodetto is built on adds a `HostBuffer` object. The `HostBuffer` has no constructor because it can only be created by native code. Once instantiated, the `HostBuffer` JavaScript API is identical to an `ArrayBuffer`. Because the memory referenced by a `HostBuffer` is in read-only memory, write operations should not be performed as the results are unpredictable. Depending on the target hardware, writes may crash or fail silently.

> **Note**: The Commodetto and Poco JavaScript to C bindings support using a `HostBuffer` anywhere an `ArrayBuffer` is allowed. However, not all JavaScript to C bindings accept a `HostBuffer` in place of an `ArrayBuffer` and throw an exception.

## JavaScript API

## Bitmap

The Commodetto `Bitmap` object contains the three pieces of information needed to render a bitmap: the bitmap dimensions, the format of the pixels in the bitmap, and a reference to the pixel data. The following example creates a `Bitmap` object with 16-bit pixels stored in an `ArrayBuffer`:

```c
import Bitmap from "commodetto/Bitmap";

let width = 40, height = 30;
let pixels = new ArrayBuffer(height * width * 2);
let bitmap = new Bitmap(width, height, Bitmap.Raw, pixels, 0);
```

The pixels for a `Bitmap` are often stored in a `HostBuffer`, to minimize use of RAM. For example, `File.Map` creates a `HostBuffer` corresponding to the file:

```c
let pixels = new File.Map("/k1/pixels.dat");
let bitmap = new Bitmap(width, height, Bitmap.Raw, pixels, 0);
```

### Functions

#### constructor(width, height, format, buffer, offset)

The `Bitmap` constructor takes the following arguments:

* `width` - number indicating the width of the bitmap in pixels
* `height` - number indicating the height of the bitmap in pixels
* `format` - the type of pixels in the bitmap, taken from Bitmap static properties, e.g. `Bitmap.Raw`, `Bitmap.RLE`, etc.
* `buffer` - the `ArrayBuffer` or `HostBuffer` containing the pixel data for the bitmap
* `offset` - a number indicating the offset in bytes from the start of `buffer` where the pixel data begins

All properties of a bitmap are fixed at the time it is constructed and cannot be changed after that.

### Properties

#### width

Returns the width in pixels of the bitmap. This property is read-only.

#### height

Returns the height in pixels of the bitmap. This property is read-only.

#### format

Returns the pixel format of pixels of the bitmap. This property is read-only.


### Static properties

#### Bitmap.Raw

A constant corresponding to the value of `kCommodettoBitmapRaw`.

#### Bitmap.RLE

A constant corresponding to the value of `kCommodettoBitmapPacked`.

#### Bitmap.Monochrome

A constant corresponding to the value of `kCommodettoBitmapMonochrome`.

#### Bitmap.Gray

A constant corresponding to the value of `kCommodettoBitmapGray16`.

## PixelsOut

The `PixelsOut` class is a generic base class used to output pixels. The `PixelsOut` class is overridden by other classes to output to different destinations:

* `SPIOut` - sends pixels to an LCD display connected over SPI
* `BufferOut` - sends pixels to an offscreen memory buffer
* `BMPOut` - writes pixels to a BMP file
* `RLEOut` - compresses pixels to the Commodetto RLE format

A `PixelsOut` instance is initialized with a dictionary of values. The dictionary allows an arbitrary number of name/value pairs to be provided to the constructor. Different `PixelsOut` subclasses require different initialization parameters.

```javascript
let display = new SPIOut({width: 320, height: 240,
		pixelFormat: "rgb565be", orientation: 90, dataPin: 30});
	
let offScreen = new BufferOut({width: 40, height: 40,
		pixelFormat: "rgb565le"});

let bmpOut = new BMPOut({width: 240, height: 240,
		pixelFormat: "rgb565le", path: "/k1/test.bmp"});
```

> **Note**: Developers building applications using Commodetto do not need to use the majority of the `PixelsOut` API directly. An application uses the constructor of a subclass of `PixelsOut` and then immediately binds it to a `Renderer` instance which calls the `PixelsOut` as needed. The application interacts exclusively with the `Renderer`. The remaining information in this `PixelsOut` section is of interest to developers implementing their own `PixelsOut` subclasses.

Once constructed, a `PixelsOut` instance can receive pixels. Three function calls are used to send pixels to the output: `begin`, `send`, and `end`. The following example shows how to output black pixels to a portion of a `PixelsOut` instance.

```javascript
let x = 10, y = 20;
let width = 40, height = 50;
display.begin(x, y, width, height);
	
let scanLine = new ArrayBuffer(display.pixelsToBytes(width));
for (let line = 0; line < height; line++)
	display.send(scanLine);
	
display.end();
```

The `begin` call indicates the area of the PixelsOut bitmap that to update. One or more calls to `send` follow that containing the pixels. In the above example, `send` is called 50 times, each time with a buffer of 40 black (all 0) pixels. The number of bytes of data passed by the combined calls to `send` must be exactly the number of bytes needed to fill the area specified when calling `begin`. Once all data has been provided, call `end`. 

The `PixelsOut` API does not define when the data is transmitted to the output. Different `PixelsOut` subclasses implement it differently. Some transmit the data immediately and synchronously, some use asynchronous transfers, and others buffer the data to display only when `end` is called.

The following shows the definition of the `PixelsOut` class (function bodies omitted):

```javascript
class PixelsOut {
	constructor(dictionary) {}
	begin(x, y, width, height) {}
	end() {}
	continue(x, y, width, height) {}

	send(pixels, offset = 0, count = pixels.byteLength - offset) {}

	get width() {}
	get height() {}

	get pixelFormat() {}
	pixelsToBytes(count) {}
}
```

### Functions

#### constructor(dictionary)

The constructor takes a single argument, a dictionary of property names and values. The `PixelsOut` base class defines three properties that are defined for all subclasses of `PixelsOut`. 

* `width` - a number specifying the width in pixels of the output
* `height` - a number specifying the height in pixels of the output
* `pixelFormat` - a string specifying the format of the pixels of the output, for example "rgb565le" for 16-bit 565 little-endian pixels and "rgb565be" for 16-bit 565 big-endian pixels.

Subclasses define additional properties in their dictionary as needed.

```javascript
import PixelsOut from "commodetto/PixelsOut";

let out = new PixelsOut({width: 120, height: 160, pixelFormat: "rgb565be"});
```

> **Note**: The `width`, `height`, and `pixelFormat` properties of a `PixelsOut` are fixed at the time the object is created and cannot be changed. In general, all properties included in the dictionary provided to the constructor should be considered read-only to keep the implementation of the `PixelsOut` subclasses simple. However, subclasses of `PixelsOut` may choose to provide ways to modify some properties.

#### begin(x, y, width, height)

The `begin` function starts the delivery of a frame to the output. The `x` and `y` arguments indicate the top left corner of the frame; the `width` and `height` arguments, the size of the frame. The area contained by the arguments to the `begin` function must fit entirely within the dimensions provided to the constructor in its dictionary.

#### end()

The `end` function indicates that delivery of the current frame to the output is complete. Calling `end` is required for proper output of the frame.

#### continue(x, y, width, height)

The `continue` function is a way to draw to more than one area of a single frame. In concept, calling `continue` is equivalent to calling `end` following by `begin`. That is to say:

```javascript
out.continue(10, 20, 30, 40);
```

is almost the same as

```javascript
out.end();
out.begin(10, 20, 30, 40);
```
	
The difference is that using `continue`, all output pixels are part of the same frame whereas when using `end`/`begin` pixels transmitted before `end` are part of one frame and pixels transmitted after are part of the following frame.

For some outputs, for example the `BufferOut`, there is no difference. For others, for example a hardware accelerated renderer, the results are visually different.

> **Note**: Not all `PixelsOut` subclasses implement `continue`. If it is not supported, an exception is thrown. The documentation for the subclass indicates if `continue` is supported.

#### send(pixels, offset = 0, count = pixels.byteLength - offset)

The `send` function transmits pixels to the output. The `pixels` argument is an `ArrayBuffer` or `HostBuffer` containing the pixels. The `offset` argument is the offset in bytes into the buffer where the pixels begin, and `count` is the number of bytes to transmit from the buffer.

```javascript
let pixels = new ArrayBuffer(40);
out.send(pixels);		// send all pixels in buffer
out.send(pixels, 30);	// send last 5 pixels in buffer
out.send(pixels, 2, 2)	// send 2nd pixel in buffer
```

> **Note**: The `offset` and `count` arguments are in units of bytes not pixels.

#### pixelsToBytes(count)

The `pixelsToBytes` function calculates the number of bytes required to store the number of pixels specified by the `count` argument. The following allocates an `ArrayBuffer` corresponding to a single scan line of pixels 

```javascript
let buffer = new ArrayBuffer(out.pixelsToBytes(width));
```

### Properties

#### width

Returns the width of the `PixelsOut` in pixels. This property is read-only.

#### height

Returns the height of the `PixelsOut` in pixels. This property is read-only.

#### pixelFormat

Returns the format of the pixels of the `PixelsOut`. This property is read-only.

## BufferOut

BufferOut is a subclass of PixelsOut that implements an offscreen in-memory bitmap.

```javascript
class BufferOut extends PixelsOut;
```

Because memory tends to be a scarce resource on target devices, applications should use `BufferOut` sparingly.

To create a `BufferOut`:

```javascript
import BufferOut from "commodetto/BufferOut";

let offscreen = new BufferOut({width: 40, height: 30, pixelFormat: "rgb565le"});
```

The `BufferOut` constructor allocates the buffer for the bitmap pixels. Once the offscreen has been created, pixels are delivered to it using the `send` function, as described for `PixelsOut` objects. The pixels in the `BufferOut` instance are accessed through the `BufferOut`'s bitmap:

```javascript
let bitmap = offscreen.bitmap;
```

`BufferOut` implements the optional `continue` function of `PixelsOut`.

### Properties

#### bitmap

The `bitmap` property of a `BufferOut` returns a `Bitmap` instance to access the pixels of the `BufferOut`. The `bitmap` property is read-only.

## BMPOut

`BMPOut` is a subclass of `PixelsOut` that receives pixels and writes them in a BMP file with 16 bit rgb565 pixels. The `BMPOut` implementation writes pixels to the file incrementally, so it can create files larger than available free memory.

`BMPOut` adds the `path` property to the constructor's dictionary, is a string containing the full path to the output BMP file.

## RLEOut

`RLEOut` is a subclass of `PixelsOut` that receives pixels and packs them into an RLE encoded `Bitmap`.

```javascript
class RLEOut extends PixelsOut;
```

There are several benefits to storing a bitmap in the RLE format:

* For many common bitmap uses, RLE encoded bitmaps draw more quickly
* RLE encoded bitmaps are smaller
* RLE encoded bitmaps can include a 1-bit mask to draw non-rectangular images, including images with holes

`RLEOut` extends the dictionary passed to the constructor with the optional `key` property which contains a pixel value to use as a key color when encoding the image. All pixels transmitted to `RLEOut` that match the value of the key color pixel are excluded from the RLE encoded image, making them invisible. This key color capability is effectively 1-bit mask.

The `RLEOut` class may be used on the device. More commonly, a bitmap is RLE encoded and written to storage at build time. This avoids the time and memory required to encode the image on the device, while saving storage space.

### Properties

#### bitmap

The `bitmap` property of a `RLEOut` returns a `Bitmap` instance to access the pixels of the `RLEOut`. The `bitmap` property is read-only.

### Static functions

#### encode(source, key)

The `encode` function converts a Raw bitmap, specified by the `source` argument, into an RLE encoded bitmap. The optional `key` argument is the pixel value to use as the key color for generating the 1-bit mask.

```javascript
import RLEOut from "commodetto/RLEOut";

let uncompressed = /* a Bitmap of format Bitmap.Raw */
let compressed = RLEOut.encode(uncompressed);
let compressedWithBlackKey = RLEOut.encode(uncompressed, 0);
```

### SPIOut

This subclass of `PixelsOut` is a placeholder for an implementation of a `PixelsOut` that sends pixels to a display connected over SPI.

```javascript
class SPIOut extends PixelsOut;
```

## Asset parsing

Building a user interface for a display requires visual assets - icons, bitmaps, photos, fonts, etc. There are many commonly used file formats for storing these assets, some of which work well on constrained devices. Commodetto includes functions to use several common asset file formats directly. However, Commodetto does not support all features of these file formats. Graphic designers creating assets for use with Commodetto need to be aware of the asset format requirements.

> **Note**: for optimal render performance and minimum storage size it is beneficial to use tools at build time to convert assets to the preferred format of the target device. That is out of scope for this release of Commodetto but may be addressed in the future.

### BMP

The [BMP](https://msdn.microsoft.com/en-us/library/dd183391.aspx) file format was created by Microsoft for use on Windows. It is a flexible container for uncompressed pixels of various formats. The format is unambiguously documented and well supported by graphic tools. BMP is the preferred format for uncompressed bitmaps in Commodetto. The Commodetto BMP file parser supports the following variants of BMP:

* **16-bit 565 little-endian pixels**. The image width must be a multiple of 2. When using Photoshop to save the BMP file, select Advanced Mode in the BMP dialog and check "R5 G6 B5."
* **16-bit 555 little-endian pixels**. The image width must be a multiple of 2. This format is the most common 16-bit format for BMP files, but it is not recommended as the pixels must be converted to 565, which takes time and requires memory to store the converted pixels.
* **4-bit gray**. The image width must be a multiple of 8. The image must have a gray palette. When working in Photoshop, this means setting the Image Mode to "Grayscale" before saving the image in BMP format.
* **1-bit black and white**. The image width must be a multiple of 32.

By default, BMP files are stored with the bottom line of the bitmap first in the file, e.g. bottom-to-top order. Commodetto requires bitmaps to be stored top-to-bottom. When saving a BMP file, select the option to store it in top-to-bottom order. In Photoshop, check "flip row order" to store the BMP in top-to-bottom order.

The `parseBMP` function creates a bitmap from an `ArrayBuffer` or `HostBuffer` containing BMP data.

```javascript
import parseBMP from "commodetto/parseBMP";

let bmpData = new File.Map("/k1/image.bmp");
let bitmap = parseBMP(bmpData)
console.log(`Bitmap width ${bitmap.width}, height ${bitmap.height}\n`);
```

The `parseBMP` function performs validation to confirm that the file format is supported. `parseBMP` throws an exception when it detects an unsupported variant of BMP.

### JPEG

The JPEG file format is the most common format for storing photos. Many resource constrained devices have the performance to decompress JPEG images, though not all have the memory to store the result. Commodetto provides a way to render a JPEG image to an output, even though the decompressed JPEG image cannot fit into memory.

The JPEG decoder used in Commodetto supports a subset of the JPEG specification. YUV encoding with H1V1 is supported, with plans to support H2V2 in a future release. Grayscale JPEG images are supported. There are no restrictions on the width and height of the JPEG image. Progressive JPEG images are not supported.

To decompress JPEG data to an offscreen bitmap, use the static `decompress` function.

```javascript
import JPEG from "commodetto/readJPEG";

let jpegData = File.Map("/k1/image.jpg");
let bitmap = JPEG.decompress(jpegData);
```

The JPEG decoder also implements a block-based decode mode to return a single block of decompressed data at a time.

```javascript
import JPEG from "commodetto/readJPEG";

let jpegData = File.Map("/k1/image.jpg");
let decoder = new JPEG(jpegData);
	
while (true) {
	let block = decoder.read();
	if (!block) break;		// all blocks decoded

	console.log(`block x: ${block.x}, y: ${block.y},
			width: ${block.width}, height: ${block.height}\n`);
}
```

Each block returned is a bitmap. The `width` and `height` fields of the bitmap indicate the dimensions of the block. The `width` and `height` can change from block to block. The `x` and `y` properties indicate the placement of the block relative to the top-left corner of the full JPEG image. Blocks are returned in a left-to-right, top-to-bottom order.

The same bitmap object is used for all blocks, so the contents of the block change after each call to read. This means an application cannot collect all the blocks into an array for later rendering. To do that, the application needs to make a copy of the data from each block.

Using a renderer it is straightforward to incrementally send a JPEG image to a display block-by-block as it is decoded, eliminating the need to copy the data of each block. The Poco renderer documentation includes an example of this technique.

> **Note**: Commodetto uses the excellent public domain [picojpeg](https://code.google.com/archive/p/picojpeg/) decoder which is optimized to minimize memory use. Some quality and performance are sacrificed, though the results are generally quite good. Small changes have been made to picojpeg to eliminate compiler warnings. Those changes are part of the Commodetto source code distribution.

### NFNT

[NFNT](http://mirror.informatimago.com/next/developer.apple.com/documentation/mac/Text/Text-250.html#MARKER-9-414) is the bitmap font format used in the original Macintosh computer (note: NFNT is a small update to the original FONT resource). NFNT fonts are 1-bit glyphs with compact metrics tables, making them easy to fit into the most constrained of devices. An NFNT may be as small as 3 KB. The design of many NFNT fonts is beautiful, with carefully hand-tuned pixels.

To use NFNT in Commodetto the NFNT resource must be extracted to a file. Several tools to do this extraction including [rezycle](https://itunes.apple.com/us/app/rezycle/id485082834?mt=12), which is available in the Mac App Store.

The `parseNFNT` function prepares an NFNT resource for measuring and drawing text with a `Render` object.

```javascript
import parseNFNT from "commodetto/parseNFNT";

let chicagoResource = File.Map("/k1/chicago_12.nfnt");
let chicagoFont = parseNFNT(chicagoResource);
```

The font object returned by `parseNFNT` contains the information required by a Render object, including the monochrome bitmap containing the glyphs. The glyph bitmap may be accessed directly:

```javascript
let chicagoBitmap = chicagoFont.bitmap;
```

### BMFont

[BMFont](http://www.angelcode.com/products/bmfont/) is a format for storing bitmap fonts. It is widely used in games to embed distinctive fonts in games in a format that is efficiently rendered using OpenGL. BMFont is well designed and straightforward to support. Commodetto uses BMFont to store both anti-aliased and multi-color fonts, capabilities unavailable using NFNT. In addition, BMFont has good tool support, in particular the excellent [Glyph Designer](https://71squared.com/glyphdesigner) which converts Mac OS X fonts to a Commodetto compatible BMFont.

BMFont stores a font's metrics data separately from the font's glyph atlas (bitmap). This means loading a BMFont requires two steps, loading the metrics and loading the glyph atlas. BMFont allows the metrics data to be stored in a variety of formats including text, XML, and binary. Commodetto supports the binary format for metrics.

The `parseBMF` function prepares the BMFont binary metrics file for use with a Render object.

```javascript
import parseBMF from "commodetto/parseBMF";
import parseBMP from "commodetto/parseBMP";

let palatino36 = parseBMF(File.Map("/k1/palatino36.fnt"));
palatino36.bitmap = parseBMP(File.Map("/k1/palatino36.bmp");
```

After preparing the metrics with `parseBMF`, the glyph atlas is prepared using `parseBMP` and attached to the metrics as the `bitmap` property.

The BMFont format allows multiple discontiguous ranges of characters, but parseBMF requires that the characters are a single contiguous range.

For anti-aliased text, the BMP file containing the glyph atlas bitmap must be in 4-bit gray format. For multi-color text, the bitmap must be in raw (16-bit) format.

## Rendering

Commodetto is designed to support multiple rendering engines. The initial engine is Poco, a small bitmap-based scan-line renderer. A renderer knows how to draw pixels and relies on a `PixelsOut` instance output those pixels, whether to a display, offscreen buffer, or file.

When a renderer is created, it is bound to an output. For example to render to a BMP file:

```javascript
import BMPOut from "commodetto/BMPOut";
import Poco from "commodetto/Poco";

let bmpOut = new BMPOut({width: decoded.width, height: decoded.height,
		pixelFormat: "rgb565le", path: "/k1/allegra64.bmp"});
let render = new Poco(bmpOut);
```

To render to a display, use the `SPIOut` in place of `BMPOut`:

```javascript
import SPIOut from "commodetto/SPIOut";
import Poco from "commodetto/Poco";

let display = new SPIOut({width: 320, height: 240,
		pixelFormat: "rgb565be", orientation: 90, dataPin: 30});
let render = new Poco(display);
```

The Poco renderer documentation describes its rendering operations with examples of common uses.

## Render

The `Render` class is a generic base class used to generate pixels. The `Render` class is overridden by specific rendering engines, such as Poco. The `Render` class has only four functions, which manage the rendering process but do no rendering themselves. The specific rendering operations available are defined by subclasses of `Render`.

The following example shows using the Poco renderer with a `SPIOut` to render a screen of a white background and a 10 pixel red square at location {5, 5}.

```javascript
let display = new SPIOut({width: 320, height: 240,
		pixelFormat: "rgb565le", orientation: 90, dataPin: 30});
let render = new Poco(display);
	
let white = poco.makeColor(255, 255, 255);
let red = poco.makeColor(255, 0, 0);
	
render.begin();
render.fillRectangle(white, 0, 0, display.width, display.height);
render.fillRectangle(red, 5, 5, 10, 10);
render.end();
```

### Functions

#### constructor(pixelsOut, dictionary)

The `Render` constructor takes two arguments. The first is the PixelsOut instance the render object sends pixels to for output. The second is a dictionary to configure rendering options. All dictionary properties are defined by the subclasses of `Render`.

#### begin(x, y, width, height)

The `begin` function starts the rendering of a frame. The area to be updated is specified with the `x` and `y` coordinates and `width` and `height` dimensions. The area must be fully contained within the bounds of the pixelsOut bound to the renderer:

```javascript
render.begin(x, y, width, height);
```

All drawing is clipped to the updated area defined by `begin`.

Calling `begin` with no arguments is equivalent to calling `begin` with `x` and `y` of zero, and `width` and `height` equal to the `width` and `height` of the pixelsOut:

```javascript
render.begin()
```
	
is equivalent to:

```javascript
render.begin(0, 0, pixelsOut.width, pixelsOut.height);
```

If the `width` and `height` are omitted the update area is the rectangle defined by the `x` and `y` coordinates passed to `begin` and the bottom right corner of the pixelsOut bounds:

```javascript
render.begin(x, y);
```

is equivalent to:

```javascript
render.begin(x, y, pixelsOut.width - x, pixelsOut.height - y);
```

#### end()

The `end` function completes the rendering of a frame. All pending rendering operations are completed by the `end` function.

> Note: For a display list renderer, such as Poco, all rendering occurs during the execution of `end`.

#### continue(x, y, width, height)

The `continue` function is used when there are multiple update areas in a single frame. The arguments to `continue` behave in the same manner as `begin`. The `continue` function is nearly equivalent to the following sequence:

```javascript
render.end();
render.begin(x, y, width, height);
```

The difference is for displays with a buffer, for example when Commodetto is running on a display with page flipping hardware or on a computer simulator. In that case, the output frame is only displayed when `end` is called. Using `continue`, intermediate results remain hidden offscreen until the full frame is rendered.

The following fragment shows three separate update areas for a single frame:

```javascript
render.begin(10, 10, 20, 20);
// draw
render.continue(200, 100, 40, 40);
// draw more
render.continue(300, 0, 20, 20);
// draw even more
render.end();
```

## Etc.

### Target hardware

The primary design constraint for Commodetto is to render a modern user interface on a resource constrained micro-controller. The target devices have the following broad characteristics:

- ARM Cortex M3/M4 CPU
- Single core
- 80 to 200 MHZ CPU speed
- 128 to 512 KB of RAM
- 1 to 4 MB of flash storage
- No graphics rendering hardware

If a more capable processor is available Commodetto gets better. More memory allows for more offscreen bitmaps and more complex scenes. More CPU speed allows for greater use of computationally demanding graphics operations. More flash storage allows for more assets to be stored. Hardware graphics acceleration allows for faster rendering of more complex screens.

Commodetto runs on any target hardware that supports the XS6 JavaScript engine. Commodetto is written in ANSI C, with only a handful of calls to external functions (`memcpy`, `malloc`, `free`). The core Poco renderer allocates no memory.

<!--
...and has been verified to run on an Arduino running at 16 MHz with 2 KB of RAM.
-->

### XS6

The JavaScript APIs in Commodetto are implemented for use with the [XS6](https://github.com/Kinoma/kinomajs/tree/master/xs6) JavaScript engine. XS6 is a JavaScript engine optimized for use on embedded platforms while maintaining [compatibility](http://kangax.github.io/compat-table/es6/) with the full ES6 JavaScript language.

### License

Commodetto is provided under the [Mozilla Public License 2.0](https://www.mozilla.org/en-US/MPL/2.0/) (MPL). The MPL is a copyleft license which guarantees users of products that incorporate Commodetto have access to the version of the Commodetto source code used in that product. The MPL is an [OSI approved](https://opensource.org/licenses) open source license compatible with major open source licenses, including the GPL and Apache 2.0.

The MPL grants broad rights to developers incorporating Commodetto into their products. In exchange for those rights, developers have the obligation to acknowledge their use of Commodetto and to share the source code of any changes they make to Commodetto, as well other obligations described in the MPL.

### About the name "Commodetto"

The word "Commodetto" is a term used in music meaning "leisurely." The use of Commodetto here is taken from a set of piano variations by Beethoven, specifically the third variation of WoO 66. A sample of the variation is [available for listening](http://www.prestoclassical.co.uk/r/Warner%2BClassics/5857612). The feeling of the variation is light and leisurely, though there is nothing simple or trivial about the composition or the performance.
