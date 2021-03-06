# Poco
Copyright 2016 Moddable Tech, Inc.

Revised: June 13, 2016

<!--
## Features

* Composed - draw lots of objects without having to worry about overlaps
* Big and little endian output
-->

## Examples
This following set of examples introduces many of the main concepts of working with the Poco renderer. The samples all use the JavaScript API. The reference for Poco follows, which fully describes each function call. These samples also use asset loaders and other capabilities of Commodetto.

To keep the examples concise and focused, the code makes several assumptions:

(1) The examples assume a `PixelsOut` object in the variable `screen`.

(2) The examples assume the following color variables are defined:

```javascript
let white = poco.makeColor(255, 255, 255);
let black = poco.makeColor(0, 0, 0);
let gray = poco.makeColor(128, 128, 128);
let red = poco.makeColor(255, 0, 0);
let green = poco.makeColor(0, 255, 0);
let blue = poco.makeColor(0, 0, 255);
```
(3) The examples assume that the drawing commands occur between calls to `begin` and `end`:

```javascript
poco.begin();
// example code here
poco.end();
```
> **Note**: Each example includes the image rendered by the code. The images are scaled 150% to make them easier to see. This scaling causes some blurring and introduces some jaggedness that is not in the actual image.

### Rectangle

This example fills `screen` with gray pixels, covers the left half with red pixels, and then uses a 50% blend level (128) to draw blue pixels over the middle half of the screen.

```javascript
poco.fillRectangle(gray, 0, 0, screen.width, screen.height);
poco.fillRectangle(red, 0, 0, screen.width / 2, screen.height);
poco.blendRectangle(blue, 128, screen.width / 4,
	0, screen.width / 2, screen.height);
```

<img src="assets/fillrectangle.png" width="180" height="135"/>

### Origin

The following example shows how to move the drawing origin. Poco maintains an origin stack which is pushed when the origin changes and popped when calling `origin` with no arguments. Each change to the origin offsets the previous origin. The origin stack is convenient when building container-based user interfaces.

```javascript
poco.fillRectangle(gray, 0, 0, screen.width, screen.height);

poco.origin(10, 10);
poco.fillRectangle(red, 0, 0, 40, 20);

poco.origin(25, 25);
poco.fillRectangle(green, 0, 0, 40, 20);

poco.origin(25, 25);
poco.fillRectangle(blue, 0, 0, 40, 20);

poco.blendRectangle(black, 128, -4, -4, 20, 10);
poco.origin();

poco.blendRectangle(black, 128, -4, -4, 20, 10);
poco.origin();

poco.blendRectangle(black, 128, -4, -4, 20, 10);
poco.origin();
```

<img src="assets/origin.png" width="180" height="135"/>

### Clip

The following example shows how to use the drawing clip. Poco maintains a clip stack which is pushed when the clip changes and popped when calling `clip` with no arguments. Each change intersects the clip with the previous clip. The clip stack is convenient when building container-based user interfaces.

```javascript
poco.fillRectangle(gray, 0, 0, screen.width, screen.height);

poco.clip(20, 20, screen.width - 40, screen.height - 40);
poco.fillRectangle(green, 0, 0, screen.width, screen.height);

poco.clip(0, 0, 40, 40);
poco.fillRectangle(blue, 0, 0, screen.width, screen.height);

poco.fillRectangle(white, 26, 0, 2, screen.height);

poco.clip();
poco.fillRectangle(red, 30, 0, 2, screen.height);

poco.clip();
poco.fillRectangle(black, 34, 0, 2, screen.height);
```

<img src="assets/clip.png" width="180" height="135"/>

### Monochrome bitmap

This example draws a monochrome bitmap (all pixels are either black or white) of an envelope. It shows how to control the color of the foreground and background pixels, as well as whether each is drawn. The bitmap is stored in a 1-bit BMP file with dimensions of 32 x 23.

```javascript
poco.fillRectangle(gray, 0, 0, screen.width, screen.height);

let envelope = parseBMP(new File.Map("/k1/envelope.bmp"));
poco.drawMonochrome(envelope, black, white, 14, 10)
poco.drawMonochrome(envelope, red, white, 14, 55)
poco.drawMonochrome(envelope, green, undefined, 74, 10)
poco.drawMonochrome(envelope, undefined, blue, 74, 55)
```

<img src="assets/monochrome.png" width="180" height="135"/>

### Color bitmap

This example draws a color bitmap image of a face in two ways using `drawBitmap`. On the left side of the screen, it draws the full image. On the right side of the screen only the eyes and mouth are drawn, using the optional source rectangle parameters of `drawBitmap`.

```javascript
poco.fillRectangle(gray, 0, 0, screen.width, screen.height);

let image = parseBMP(new File.Map("/k1/lvb.bmp"));

let x = 0;
let y = Math.round((screen.height - image.height) / 2);
poco.drawBitmap(image, x, y);

x = Math.round(screen.width / 2);
poco.drawBitmap(image, x + 25, y + 38, 25, 38, 11, 7);	// left eye
poco.drawBitmap(image, x +  7, y + 40,  7, 40, 10, 6);	// right eye
poco.drawBitmap(image, x + 15, y + 56, 15, 56, 16, 6);	// mouth
```
<img src="assets/bitmap.png" width="180" height="135"/>

### Pattern

This examples uses `fillPattern` to draw a single 30 pixel square pattern two different ways. First, the entire pattern is used to fill the screen. Then, a 7 pixel square area of the pattern is used to fill the center part of the screen.

Unlike previous samples, this one does not first call `fillRectangle` to clear the screen because the first call to `fillPattern` does so.

```javascript
let pattern = parseBMP(new File.Map("/k1/pattern1.bmp"));
poco.fillPattern(pattern, 0, 0, screen.width, screen.height);
poco.fillPattern(pattern, 28, 28, 63, 35, 21, 14, 7, 7);
```

<img src="assets/pattern.png" width="180" height="135"/>

### Gray bitmap

This example uses `drawGray` to draws a 16 level gray image in several colors. `drawGray` treats the pixel values as alpha levels, blending the specified color with the background. 

```javascript
poco.fillRectangle(gray, 0, 0, screen.width, screen.height);

let image = parseBMP(new File.Map("/k1/envelope-gray.bmp"));

poco.drawGray(image, black, 10, 2);
poco.drawGray(image, white, 10, 47);

poco.drawGray(image, black, 70, 2);
poco.drawGray(image, green, 70 + 2, 2 + 2);

poco.drawGray(image, white, 70, 47);
poco.drawGray(image, red, 70 + 2, 47 + 2);
```

<img src="assets/gray.png" width="180" height="135"/>

### Offscreen

This example uses `BufferOut` to create an offscreen bitmap, fills the bitmap with a series of inset squares, and then uses the offscreen bitmap as a pattern to fill the screen.

This example uses two instances of `Poco`, the first to draw to the offscreen and the second to draw to the screen.

```javascript
import BufferOut from "commodetto/BufferOut";

let offscreen = new BufferOut({width: 30, height: 30, pixelFormat: "rgb565le"});
let poco = new Poco(offscreen);
poco.begin();
	poco.fillRectangle(gray, 0, 0, 30, 30);
	poco.fillRectangle(red, 2, 2, 26, 26);
	poco.fillRectangle(black, 4, 4, 22, 22);
	poco.fillRectangle(blue, 6, 6, 18, 18);
	poco.fillRectangle(white, 8, 8, 14, 14);
	poco.fillRectangle(green, 10, 10, 10, 10);
	poco.fillRectangle(gray, 13, 13, 4, 4);
poco.end();

poco = new Poco(screen);
poco.begin();
	poco.fillPattern(offscreen.bitmap, 0, 0, screen.width, screen.height);
poco.end();
```

<img src="assets/offscreen.png" width="180" height="135"/>

### RLE
RLE encoding of bitmaps has two benefits. The first examples shows how a key color can be applied to full color bitmap to remove the background. The key color is used to generate a one bit mask. The `RLEOut.encode` function generates the RLE bitmap and applies the key color.

```javascript
poco.fillRectangle(gray, 0, 0, screen.width, screen.height);

let image = parseBMP(new File.Map("/k1/lvbmask.bmp"));
let keyed = RLEOut.encode(image, white);

poco.drawBitmap(image, 0, 4);
poco.drawBitmap(keyed, 60, 4)
```

<img src="assets/rle.png" width="180" height="135"/>

The second example shows the other benefit of RLE. Many computer generated graphics can be reduced significantly in size. The following example compresses an image to RLE, writes the result to a file, and reloads it for display. The anti-aliased check mark image shown below is reduced from 5000 to 1350 bytes by RLE encoding. In addition to reducing the data size, the RLE compressed image also renders more quickly.

```javascript
// load uncompressed image
let bmp = parseBMP(new File.Map("/k1/button.bmp"));
// compress to RLE (no key color applied)
let rle = RLEOut.encode(bmp);

// write RLE data to file
let fileOut = new File("/k1/button.rle", 1);
fileOut.write(rle.buffer);	
fileOut.close();

// generate Bitmap for RLE data written to file
rle = new Bitmap(bmp.width, bmp.height, Bitmap.raw,
			new File.Map("/k1/button.rle", 0);
// draw RLE bitmap loaded from file
poco.drawBitmap(rle, 0, 0);
```

<img src="assets/check.png" width="100" height="100">

### Alpha

The following example shows how to draw a bitmap through an alpha mask. The bitmap and mask are in separate bitmaps, allowing an image to be drawn using more than one alpha mask. This example draws one bitmap though both a circle and a square mask, and also draws the original image and mask.

```javascript
poco.fillRectangle(gray, 0, 0, screen.width, screen.height);

let girl = parseBMP(new File.Map("/k1/girl.bmp"));
let circle = parseBMP(new File.Map("/k1/mask_circle.bmp"));
let square = parseBMP(new File.Map("/k1/mask_square.bmp"));

poco.drawBitmap(girl, 0, 2);
poco.drawGray(circle, black, 40, 2);
poco.drawMasked(girl, 80, 2, 0, 0,
	circle.width, circle.height, circle, 0, 0);

poco.drawBitmap(girl, 0, 47);
poco.drawGray(square, black, 40, 47);
poco.drawMasked(girl, 80, 47, 0, 0,
	square.width, square.height, square, 0, 0);
```

<img src="assets/alpha.png" width="180" height="135"/>

### JPEG

These two examples show different methods for working with JPEG images. The first example decompresses the full JPEG into an bitmap in memory and then renders the bitmap to the screen. The call to console.log shows how to access the bitmap's width and height.

```javascript
import JPEG from "commodetto/readJPEG";

let piano = JPEG.decompress(new File.Map("/k1/piano.jpg"));
console.log(`width ${piano.width}, height ${piano.height}\n`);
poco.drawBitmap(piano, 0, 0);
```

The second example decodes the JPEG image one block at a time, drawing the block to the screen before decoding the next block. The advantage of this approach is that only a single JPEG block (typically 8 x 8 or 16 x 16 pixels) need be in memory at a time. The disadvantage is that the JPEG image appears to the user one block at a time, rather than all at once.

```javascript
let jpeg = new JPEG(new File.Map("/k1/piano.jpg"));
let block;
while (block = jpeg.read()) {
	poco.begin(block.x, block.y, block.width, block.height);
		poco.drawBitmap(block, block.x, block.y);
	poco.end();
}
```

When complete, each approach generates the same result.

<img src="assets/jpeg.png" width="180" height="135"/>

### Text

Poco supports two types of fonts for rendering text. The first is NFNT, a monochrome bitmap font used on the early Macintosh computer. NFNT font resources are extracted to standalone files. The second is BMFont, a gray or color font used in games for anti-aliased fonts. A BMFont consists of two files, the font metrics and the font image. The following example loads and draws a 12-point Palatino NFNT and a 36-point Palatino BMFont.

```javascript
import parseNFNT from "commodetto/parseNFNT";
import parseBMF from "commodetto/parseBMF";

poco.fillRectangle(gray, 0, 0, screen.width, screen.height);
poco.fillRectangle(white, 2, 2, screen.width - 4, screen.height - 4);

let palatino12 = parseNFNT(new File.Map("/k1/palatino_12.nfnt"));

let palatino36 = parseBMF(new File.Map("/k1/palatino_36.fnt"));
palatino36.bitmap = parseBMP(new File.Map("/k1/palatino_36_gray.bmp"));

poco.drawText("Hello.", palatino12, black, 4, 4);
poco.drawText("Hello.", palatino12, blue, 64, 4);

poco.drawText("Hello.", palatino36, black, 4, 20);
poco.drawText("Hello.", palatino36, green, 4, 55);
```

<img src="assets/text1.png" width="180" height="135"/>

Text is truncated when rendered by providing an optional `width` argument to `drawText` to indicate the horizontal space available for the text.

```javascript
poco.fillRectangle(gray, 0, 0, screen.width, screen.height);
poco.fillRectangle(white, 2, 2, screen.width - 4, screen.height - 4);

poco.drawText("Hello, world.", palatino36, red, 2, 10);
poco.drawText("Hello, world.", palatino36, green, 2, 45, screen.width - 2);
```

<img src="assets/text2.png" width="180" height="135"/>

Text is horizontally and vertically aligned using the `height` property of the font and measuring the width of strings using `getTextWidth`.

```javascript
poco.fillRectangle(gray, 0, 0, screen.width, screen.height);
poco.fillRectangle(white, 2, 2, screen.width - 4, screen.height - 4);

poco.drawText("T Left", palatino12, red,
	2, 2);
poco.drawText("T Right", palatino12, green,
	screen.width - 2 - poco.getTextWidth("T Right", palatino12), 2);

poco.drawText("B Left", palatino12, blue,
	2, screen.height - 2 - palatino12.height);
poco.drawText("B Right", palatino12, gray,
	screen.width - 2 - poco.getTextWidth("B Right", palatino12),
	screen.height - 2 - palatino12.height);

poco.drawText("Centered", palatino12, black,
	(screen.width - poco.getTextWidth("Centered", palatino12)) / 2,
	(screen.height - palatino12.height) / 2);
```
<img src="assets/text3.png" width="180" height="135"/>

`drawText` also accepts a 16 gray level alpha bitmap in the color argument. This is used to draw fonts which consists of multiple colors. The following example shows drawing Open Sans Bold Italic with both black and white pixels in the glyph images.

```javascript
poco.fillRectangle(green, 0, 0, screen.width, screen.height);

let openSans52 = parseBMF(new File.Map("/k1/OpenSans-BoldItalic-52.fnt"));
openSans52.bitmap = parseBMP(new File.Map("/k1/OpenSans-BoldItalic-52.bmp"));
openSans52.mask = parseBMP(new File.Map("/k1/OpenSans-BoldItalic-52-alpha.bmp"));

poco.drawText("Poco", openSans52, openSans52.mask, 0, 5);
```

<img src="assets/text4.png" width="180" height="135"/>

The following image is a section of the `OpenSans-BoldItalic-52.bmp` file which contains the glyph images.

<img src="assets/text_opensans_glyphs.png"/>

This is a section of the `OpenSans-BoldItalic-52-alpha.bmp` file which contains the alpha channel of the glyph images.

<img src="assets/text_opensans_mask.png"/>

## JavaScript API Reference

Poco is a renderer, a subclass of the Commodetto `Render` class.

	class Poco extends Render

### Functions

#### constructor(pixelsOut, dictionary)

Poco extends the Render dictionary with the `displayListLength` property which specifies the size of the display list buffer in bytes. Applications typically use the default display length list. Poco detects when a drawing operation would overflow the display list, ignores the drawing operation, and throws an exception when `end` is called.

```javascript
import Poco from "commodetto/Poco";
	
let screen = /* SPIOut instance */
let poco = new Poco(screen, {displayListLength: 4096});
```

> **Note**: Poco renders pixels in RGB 565 little-endian format ("rgb565le"). To accommodate display controllers that support only big-endian pixels, Poco outputs either "rgb565le" or "rgb565be" pixels, automatically configuring its output to match the pixel format of the `PixelsOut` passed to the constructor. Poco generates big-endian pixels by flipping the words of the rendered little-endian output prior to sending the pixels to the `PixelsOut`. Consequently, little-endian output is preferred as it is somewhat faster.

#### clip(x, y, width, height)

Poco maintains a clip rectangle which is applied to all drawing operations.

When `begin` is called, the clip rectangle is set to the update area passed to `begin`. Poco maintains a clip stack, eliminating the need for applications to save and restore the current clip. Calling `clip` with four arguments intersects the current clip with the area contained by the arguments. Calling `clip` wth no arguments pops the clip stack, restoring the previous clip.

```javascript
poco.clip(10, 10, 10, 10);
poco.clip();
```

The clip stack holds several clips:

```javascript
poco.begin();
// clip is entire PixelsOut area
poco.clip(10, 10, 10, 10);
// clip is {x: 10, y: 10, w: 10, h: 10}
poco.clip(0, 0, 15, 15);
// clip is {x: 10, y: 10, w: 5, h: 5}
poco.clip();
// clip is {x: 10, y: 10, w: 10, h: 10}
poco.clip()
// clip is entire PixelsOut area
poco.end();
```

If the clip stack overflows or underflows, an exception is thrown from `end`. The clip stack must be empty when `end` is called or an exception is thrown. 

> **Note**: clip and origin share the same stack, so must be popped in the order they were pushed.

#### origin(x, y)

Poco maintains an origin which is applied to all drawing operations. 

When `begin` is called, the origin is set to {x: 0, y: 0}. Poco maintains an origin stack, eliminating the need for applications to save and restore the current origin. Calling `origin` with two arguments offsets the current origin by the arguments. Calling `origin` wth no arguments pops the origin stack, restoring the previous origin.

```javascript
poco.begin();
// origin is {x: 0, y: 0}
poco.origin(10, 10);
// origin is {x: 10, y: 10}
poco.origin(5, 5);
// origin is {x: 15, y: 15}
poco.origin();
// origin is {x: 10, y: 10}
poco.origin();
// origin is {x: 0, y: 0}
poco.end();
```

If the origin stack overflows or underflows, an exception is thrown from `end`. The origin stack must be empty when `end` is called or an exception is thrown. 

> **Note**: Changing the origin does not change the clip rectangle.

> **Note**: clip and origin share the same stack, so must be popped in the order they were pushed.

#### makeColor(r, g, b)

The `makeColor` function takes red, green, and blue values between 0 and 255 and returns the corresponding pixel value. The returned pixel is in the format of the `PixelsOut` bound to the Poco instance.

```javascript
let red = poco.makeColor(255, 0, 0);
let green = poco.makeColor(0, 255, 0);
let blue = poco.makeColor(0, 0, 255);
let black = poco.makeColor(0, 0, 0);
let white = poco.makeColor(255, 255, 255);
let gray = poco.makeColor(127, 127, 127);
```

Many rendering functions take a color as an argument. Use `makeColor` to calculate the color to avoid a dependency on the pixel format.

#### fillRectangle(color, x, y, width, height)

The `fillRectangle` function fills the area specified by the `x`, `y`, `width`, and `height` arguments with the specified `color`.

```javascript
poco.fillRectangle(green, 10, 20, 40, 40);
```

#### blendRectangle(color, blend, x, y, width, height)

The `blendRectangle` function blends the specified `color` with the pixels in the area specified by the `x`, `y`, `width`, and `height` arguments. The `blend` argument determines the level of blending, from a value of 0 for transparent to a value of 255 for opaque.

The following sample draws 16 horizontal green lines with increasing opacity.

```javascript
let green = poco.makeColor(0, 255, 0);
for (let blend = 15, y = 0; blend < 256; blend += 16, y += 1)
	poco.blendRectangle(green, blend, 0, y, pixelOut.width, 1);
```

#### drawPixel(color, x, y)

The `drawPixel` function draws a single pixel of the specified `color` at location {`x`, `y`}.

```javascript
poco.drawPixel(poco.makeColor(0, 0, 127), 5, 5);
```

> **Note**: Use of many calls to `drawPixel` in a single frame can quickly fill the display list.

#### drawBitmap(bits, x, y, sx, sy, sw, sh)

The `drawBitmap` function draws all or part of a bitmap with pixels of the type `Bitmap.Raw` or `Bitmap.RLE`. The bitmap is specified in the `bits` argument, and the location to draw the bitmap is specified by the `x` and `y` arguments. The following code draws the entire image at location {x: 10, y: 5}.

```javascript
let image = parseBMP(new File.Map("/k1/image.bmp"));
poco.drawBitmap(image, 10, 5);
```

The optional `sx`, `sy`, `sw`, and `sh` arguments specify the area of the bitmap to draw. If they are omitted, the entire bitmap is drawn.

The following code draws the bottom half of the bitmap at location {x: 0, y: 0}.

```javascript
poco.drawBitmap(image, 0, 0, 0, image.height / 2, image.width, image.height / 2);
```

#### drawMonochrome(monochrome, fore, back, x, y, sx, sy, sw, sh)

The `drawMonochrome` function draws all or part of a bitmap with pixels of the type `Bitmap.Monochrome`. The bitmap is specified in the `bits` argument, and the location to draw the bitmap is specified by the `x` and `y` arguments.

The `fore` and `back` arguments specify the foreground and background colors to apply to the black and white pixels of the bitmap. If `fore` is `undefined`, the foreground pixels are not drawn; if `back` is `undefined`, the background pixels are not drawn.

```javascript
let red = poco.makeColor(255, 0, 0);
let gray = poco.makeColor(128, 128, 128);
let white = poco.makeColor(255, 255, 255);
let icon = parseBMP(new File.Map("/k1/icon.bmp"));
	
poco.drawMonochrome(icon, red, white, 0, 5); // red foreground and white background
poco.drawMonochrome(icon, gray, undefined, 0, 5); // only foreground pixels in gray
poco.drawMonochrome(icon, undefined, red, 0, 5); // only background pixels in red
```

The optional `sx`, `sy`, `sw`, and `sh` arguments specify the area of the bitmap to draw. If they are omitted, the entire bitmap is drawn.

> **Note**: `drawMonochrome` is used by `drawText` to render NFNT fonts

#### drawGray(bits, color, x, y, sx, sy, sw, sh)

The `drawGray` function draws all or part of a bitmap with pixels of the type `Bitmap.Gray`. The bitmap is specified in the `bits` argument, and the location to draw the bitmap is specified by the `x` and `y` arguments. The pixels of the bitmap are treated as alpha values, and are blended with the background. The `color` argument specifies the color to apply when blending.

The optional `sx`, `sy`, `sw`, and `sh` arguments specify the area of the bitmap to draw. If they are omitted, the entire bitmap is drawn.

> **Note**: `drawGray` is used by `drawText` to render anti-aliased grayscale BMFonts.

#### drawMasked(bits, x, y, sx, sy, sw, sh, mask, mask\_sx, mask\_sy)

The `drawMasked` function uses two bitmaps, an image and the alpha channel, to alpha blend the image through the mask onto the destination. The image, specified by the `bits` argument, is in the `Bitmap.raw` format. The alpha channel, specified by the `mask` argument, is in the `Bitmap.gray` format.

The `x` and `y` arguments specify where to locate the merged image in the output. The `sx`, `sy`, `sw`, and `sh` arguments specify the area of the image to use. The `mask_sx` and `mask_sy` arguments specify the top-left corner of the mask bitmap to use; the dimensions of the mask bitmap area are taken from the `sw` and `sh` arguments.

The following example draws a button image with an alpha channel. The image and alpha channel are stored in separate files, each previously extracted from a PNG with alpha channel.

```javascript
let buttonImage = parseBMP(new File.Map("/k1/button_image.bmp"));
let buttonAlpha = parseBMP(new File.Map("/k1/button_alpha.bmp"));
poco.drawMasked(buttonImage, 0, 0, 0, 0,
	buttonImage.width, buttonImage.height, buttonAlpha, 0, 0);
```

Storing the alpha channel separately from the image is unusual, and has benefits for resource constrained devices:

* The alpha channel image can be 4-bits per pixel, which gives good results at half the size.
* The image can be rendered with and without an alpha channel.
* A mask can be applied to any image, allowing for effects and animations.

#### fillPattern(bits, x, y, w, h, sx, sx, sx, sh)

The `fillPattern` function fills an area by repeatedly drawing all or part of a bitmap with pixels of the type `Bitmap.Raw`. The bitmap is specified in the `bits` argument. The location of the area to fill is specified by the `x` and `y` arguments, and the dimensions of the area are specified by the `w` and `h` arguments.

```javascript
let pattern = parseBMP(new File.Map("/k1/pattern.bmp"));
poco.fillPattern(pattern, 10, 10, 90, 90);
```

The optional `sx`, `sy`, `sw`, and `sh` arguments specify the area of the bitmap to use. If they are omitted, the entire bitmap is used.

```javascript
poco.fillPattern(pattern, 10, 10, 90, 90, 0, 0, 8, 8);
```

#### drawText(text, font, color, x, y [,width])

The `drawText` function draws the `text` string using the NFNT or BMF format font in the `font` argument. The text is drawn in in the color of the `color` argument at the location of the `x` and `y` arguments. Text is drawn using top-left alignment.

The following code draws the string "Hello, world" twice. The first time it is drawn in the top-left corner of the screen using the Chicago font in black. The second time it is drawn beneath the first string, using the Palatino 36 font in red.

```javascript
poco.drawText("Hello, world.", chicagoFont, black, 0, 0);
poco.drawText("Hello, world.", palatino36, red, 0, chicagoFont.height);
```

If the optional `width` argument is provided, the text is truncated on the right edge when it is too long to fit unclipped in the available width. When truncation occurs, three periods (e.g. "...") are drawn at the end of the string.

Characters in the text string that are not part of the font are ignored.

To draw full color text with anti-aliased edges, use a BMFont with a bitmap in `Bitmap.Raw` format. In place of the `color` argument, pass a mask bitmap in the format of a `Bitmap.Gray`. The mask must be at least as large as the BMFont's glyph atlas. When drawing each glyph, the pixels in the mask image corresponding to the glyph in the font image are used to alpha blend each glyph with the destination.

> **Implementation note**: The optional `width` argument is not yet implemented for NFNT fonts.

#### getTextWidth(text, font)

The `getTextWidth` function calculates the width in pixels of the `text` string when rendered using the `font`. The font is either an NFNT or BMFont.

The following sample draws the string "Hello, world." horizontally centered at the top of display.

```javascript
let text = "Hello, world".
let width = poco.getTextWidth(text, palatino36);
poco.drawText(text, palatino36, green, (pixelsOut.width - width) / 2, 0);
```

The height of the font is available in the `font.height` property.

Characters in the text string that are not part of the font are ignored.

## C API Reference

The Poco C API is a low level rendering engine. It is based on a display list, meaning that all drawing calls are queued to to a list prior to rendering. A display list allows the renderer to generate as little as a single scan line of fully composed output at a time, minimizing memory use by eliminating the need for a frame buffer in the memory of the application processor.

The Poco C API may be used independently of Commodetto and its JavaScript API. It makes no allocations and almost no external calls (only to `memcpy`), relying on the caller to provide memory.

### Data structures

#### PocoRecord

`PocoRecord` maintains state for Poco. Many of the fields are private to the implementation and should not be accessed directly by users of the library. The `PocoRecord` data structure should be initialized to zero, and the same `PocoRecord` must be passed to all Poco function calls.

The following fields in `PocoRecord` are public and can be accessed by users of the library. Poco expects these fields to be initialized by the users of the library before making the first call to Poco. 

* `width` - the width of the output in pixels
* `height` - the height of the output in pixels
* `displayList` - pointer to the start of memory for the display list
* `displayListEnd` - pointer to the end of memory for the display

The following fields are available to read from the `PocoRecord` between calls to `PocoDrawingBegin` and `PocoDrawingEnd`:

* `xOrigin` - the x coordinate of the drawing origin
* `yOrigin` - the y coordinate of the drawing origin
* `x` - the x coordinate of the drawing clip
* `y` - the y coordinate of the drawing clip
* `w` - the width of the drawing clip
* `h` - the height of the drawing clip
* `xMax` - the right coordinate of the drawing clip, `x + w`
* `yMax` - the bottom coordinate of the drawing clip, `y + h`

<!--

for origin and clip stack to work, pixelsLength and pixels must be initialized as well.

-->

#### PocoCoordinate

`PocoCoordinate` is a signed integer value. When used in Commodetto, it is aliased to `CommodettoCoordinate`. See the description of Coordinate in Commodetto for additional information.

#### PocoDimension

`PocoDimension` is an unsigned integer value. When used in Commodetto, it is aliased to `CommodettoDimension`. See the description of Dimension in Commodetto for additional information.

#### PocoPixel

`PocoPixel` is an integer value. When used in Commodetto, it is aliased to `CommodettoPixel`. See the description of Pixel in Commodetto for additional information.

#### PocoBitmapFormat

`PocoBitmapFormat ` is an integer value. When used in Commodetto, it is aliased to `PocoBitmapFormat`. See the description of Pixel format in Commodetto for additional information.

#### PocoRectangle

`PocoRectangle` defines the area enclosed by a rectangle with the top-left coordinate and dimensions.

```c
typedef struct {
	PocoCoordinate	x;
	PocoCoordinate	y;
	PocoDimension	w;
	PocoDimension	h;
} PocoRectangleRecord, *PocoRectangle;
```

#### PocoBitmap

The `PocoBitmap` structure contains the width and height of the bitmap in pixels, the format of the pixels in the bitmap, and a pointer to the pixel data. 

```c
typedef struct PocoBitmapRecord {
	PocoDimension			width;
	PocoDimension			height;
	PocoBitmapFormat		format;
	PocoPixel				*pixels;
} PocoBitmapRecord, *PocoBitmap;
```

The pixels are organized left-to-right, top-to-bottom with no padding between rows. There is no rowBytes or stride field in `PocoBitmap`.

> **Note**: Unlike `CommodettoBitmap`, `PocoBitmap` does not have an option to store an offset in place of the pixels pointer.

### Functions

Prior to making calls to Poco, a `PocoRecord` must be allocated and initialized. See the description of the `PocoRecord` data structure for details.

#### PocoMakeColor

```c
PocoPixel PocoMakeColor(uint8_t r, uint8_t g, uint8_t b);
```

`PocoMakeColor` takes red, green, and blue values from 0 to 255 and returns a `PocoPixel` value. The returned pixel value can be used as a color argument in some rendering calls.

#### PocoDrawingBegin

```c
void PocoDrawingBegin(Poco poco, PocoCoordinate x, PocoCoordinate y,
	PocoDimension w, PocoDimension h);
```

`PocoDrawingBegin` begins the rendering process for an update area of pixels bounded by the `x`, `y`, `w`, and `h` parameters. Calls to draw can only be made between calls to `PocoDrawingBegin` and `PocoDrawingEnd`.

> **Note**: The caller of `PocoDrawingBegin` is responsible for ensuring the drawing calls cover all pixels in the update area. Poco does not maintain the previous frame. Any pixels which are not drawn will contain unpredictable values.

#### PocoDrawingEnd

```c
int PocoDrawingEnd(Poco poco, PocoPixel *pixels, int byteLength,
	PocoRenderedPixelsReceiver pixelReceiver, void *refCon);

typedef void (*PocoRenderedPixelsReceiver)(PocoPixel *pixels,
	int byteCount, void *refCon);
```

`PocoDrawingEnd` renders the drawing commands added to the display list since the call to `PocoDrawingBegin`.

The caller of `PocoDrawingEnd` provides a buffer for the rendered pixels in the `pixels` and `byteLength` arguments. Poco renders as many rows of pixels as possible into the buffer and then calls the `pixelReceiver` function to output the pixels. The buffer need only be large enough to hold a single row of pixels. A buffer than can hold several rows of pixels reduces rendering overhead.

If an error occurs, adding commands to the display list as the result of drawing calls or in the execution of `PocoDrawingEnd`, `PocoDrawingEnd` returns a non-zero result:

* 1 - display list overflow
* 2 - clip and origin stack overflow
* 3 - clip and origin stack underflow or out of sequence pop

#### PocoRectangleFill

```c
void PocoRectangleFill(Poco poco, PocoPixel color, uint8_t blend,
	PocoCoordinate x, PocoCoordinate y, PocoDimension w, PocoDimension h);
```

`PocoRectangleFill` fills the area defined by the `x`, `y`, `w`, and `h` arguments with the `color`. If the `blend` level is `kPocoOpaque` (255), the color is drawn over with the background without blending; for other blend levels, the color is blended proportionally with the background.

#### PocoPixelDraw

```c
void PocoPixelDraw(Poco poco, PocoPixel color,
		PocoCoordinate x, PocoCoordinate y);
```

`PocoPixelDraw` renders a single pixel at location {`x`, `y`} in the provided `color`.

#### PocoBitmapDraw

```c
PocoCommand PocoBitmapDraw(Poco poco, PocoBitmap bits,
			PocoCoordinate x, PocoCoordinate y,
			PocoDimension sx, PocoDimension sy,
			PocoDimension sw, PocoDimension sh);
```

`PocoBitmapDraw` renders all or part of bitmap `bits` of type `kCommodettoBitmapRaw` or `kCommodettoBitmapPacked` at the location {`x`, `y`}. The `sx`, `sy`, `sw`, and `sw` arguments define the area of the bitmap to render.

#### PocoMonochromeBitmapDraw

```c
void PocoMonochromeBitmapDraw(Poco poco, PocoBitmap bits,
		PocoMonochromeMode mode, PocoPixel fgColor, PocoPixel bgColor,
		PocoCoordinate x, PocoCoordinate y,
		PocoDimension sx, PocoDimension sy,
		PocoDimension sw, PocoDimension sh);
```

`PocoMonochromeBitmapDraw` renders all or part of bitmap `bits` of type `kCommodettoBitmapMonochrome` at the location {`x`, `y`}. The `sx`, `sy`, `sw`, and `sw` arguments define the area of the bitmap to render. The `mode` argument determines which pixels are drawn:

* `kPocoMonochromeForeground` - only foreground pixels are drawn (1 pixels in the bitmap)
* `kPocoMonochromeBackground` - only background pixels are drawn (0 pixels in the bitmap)
* `kPocoMonochromeForeAndBackground` - both foreground and background pixels are drawn

The `fgColor` and `bgColor` arguments specify the colors to use to render the foreground and background pixels.

#### PocoGrayBitmapDraw

```c
void PocoGrayBitmapDraw(Poco poco, PocoBitmap bits, PocoPixel color,
		PocoCoordinate x, PocoCoordinate y,
		PocoDimension sx, PocoDimension sy,
		PocoDimension sw, PocoDimension sh);
```

`PocoGrayBitmapDraw` renders all or part of bitmap `bits` of type `kCommodettoBitmapGray16` at the location {`x`, `y`}. The `sx`, `sy`, `sw`, and `sw` arguments define the area of the bitmap to render. The pixels of the bitmap are treated as alpha blend levels and are used used to blend the `color` argument with the background pixels.

#### PocoBitmapDrawMasked

```c
void PocoBitmapDrawMasked(Poco poco, PocoBitmap bits,
		PocoCoordinate x, PocoCoordinate y,
		PocoDimension sx, PocoDimension sy,
		PocoDimension sw, PocoDimension sh,
		PocoBitmap mask, PocoDimension mask_sx, PocoDimension mask_sy);
```

`PocoBitmapDrawMasked` renders the pixels of bitmap `bits` of type `kCommodettoBitmapRaw` enclosed by `sx`, `sy`, `sw`, and `sw` at location {`x`, `y`}. The pixels are drawn using the corresponding pixels of the bitmap `mask` of type `kCommodettoBitmapGray16 ` enclosed by `mask_x`, `mask_y`, `sw`, and `sw` as alpha blend levels.

#### PocoBitmapPattern

```c
void PocoBitmapPattern(Poco poco, PocoBitmap bits,
		PocoCoordinate x, PocoCoordinate y,
		PocoDimension w, PocoDimension h,
		PocoDimension sx, PocoDimension sy,
		PocoDimension sw, PocoDimension sh);
```

`PocoBitmapPattern ` fills the area enclosed by the `x`, `y`, `w`, and `h` arguments with repeating copies of the area of the bitmap `bits` enclosed by the `sx`, `sy`, `sw`, and `sw` arguments. The bitmap must be of type `kCommodettoBitmapRaw`.

#### PocoClipPush

```c
void PocoClipPush(Poco poco, PocoCoordinate x, PocoCoordinate y,
		 PocoDimension w, PocoDimension h);
```

`PocoClipPush` pushes the current clip area on the stack, and then replaces the current clip with the intersection of the current clip and the area enclosed by the `x`, `y`, `w`, and `h` arguments.

#### PocoClipPop

```c
void PocoClipPop(Poco poco);
```

`PocoClipPop` pops the clip from the stack and replaces the current clip with the popped value.

#### PocoOriginPush

```c
void PocoOriginPush(Poco poco, PocoCoordinate x, PocoCoordinate y);
```

`PocoOriginPush` pushes the current origin on the stack, and then offsets the current origin by the `x` and `y` arguments.


#### PocoOriginPop

```c
void PocoOriginPop(Poco poco);
```

`PocoOriginPop` pops the origin from the stack and replaces the current origin with the popped value.

## Etc.

### Relationship to Commodetto

The C API of Poco may be used independently from Commodetto. Poco is the first renderer integrated into Commodetto. Poco runs on all hardware capable of supporting Commodetto, in particular the XS6 JavaScript engine. The Poco C API can also run on considerably less powerful hardware than Commodetto.

### License

Poco is provided under the [Mozilla Public License 2.0](https://www.mozilla.org/en-US/MPL/2.0/) (MPL). The MPL is a copyleft license which guarantees users of products that incorporate Poco have access to the version of the Poco source code used in that product. The MPL is an [OSI approved](https://opensource.org/licenses) open source license compatible with major open source licenses, including the GPL and Apache 2.0.

The MPL grants broad rights to developers incorporating Poco into their products. In exchange for those rights, developers have the obligation to acknowledge their use of Poco and to share the source code of any changes they make to Poco, as well other obligations described in the MPL.

### About the name "Poco"

The word "Poco" is a term used in music meaning "a little."
