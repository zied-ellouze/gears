// Copyright 2008, Google Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


function assertBlobsEqual(expectedBlob, actualBlob) {
  if (isDebug)
    assert(actualBlob.hasSameContentsAs(expectedBlob));
  else
    assertEqual(expectedBlob.length, actualBlob.length);
}

function loadBlob(filename, callback) {
  var request = google.gears.factory.create('beta.httprequest');
  request.open('GET', '/testcases/data/' + filename);
  request.onreadystatechange = function(){
    if (request.readyState == 4) {
      callback(request.responseBlob, filename);
    }
  };
  request.send();
}

// Loads a bunch of blobs and when all of them are loaded, calls the callback
// passing a map from filename to blob.
function loadBlobs(filenames, callback) {
  var numLoaded = 0;
  var blobs = {};

  for (var i = 0; i < filenames.length; ++i) {
    loadBlob(filenames[i], function(blob, filename) {
      blobs[filename] = blob;
      if (++numLoaded == filenames.length) {
        callback(blobs);
      }
    });
  }
}

function testGetContext() {
  var canvas = google.gears.factory.create('beta.canvas');
  var ctx = canvas.getContext('gears-2d');
  assertEqual(canvas, ctx.canvas);
  var ctx2 = canvas.getContext('gears-2d');
  assertEqual(ctx2, ctx);
  assertNull(canvas.getContext('foobar'));
 }

// Tests that properties have the correct initial values, and that they can be
// read and written.
function testProperties() {
  var canvas = google.gears.factory.create('beta.canvas');
  var ctx = canvas.getContext('gears-2d');
  assertEqual(300, canvas.width);
  assertEqual(150, canvas.height);
  assertEqual(1.0, ctx.globalAlpha);
  assertEqual('source-over', ctx.globalCompositeOperation);
  assertEqual('#000000', ctx.fillStyle);

  var width = 50, height = 40;
  canvas.width = width;
  canvas.height = height;
  assertEqual(width, canvas.width);
  assertEqual(height, canvas.height);
  // TODO(kart): set to bad values and test?

  var alpha = 0.4;
  ctx.globalAlpha = alpha;
  assertEqual(alpha, ctx.globalAlpha);
  ctx.globalAlpha = 9.4; // should be a no-op.
  assertEqual(alpha, ctx.globalAlpha);

  var compositeOperation = 'copy';
  ctx.globalCompositeOperation = compositeOperation;
  assertEqual(compositeOperation, ctx.globalCompositeOperation);
  ctx.globalCompositeOperation = 'foobar'; // should be a no-op.
  assertEqual(compositeOperation, ctx.globalCompositeOperation);

  var fillStyle = '#34ab56';
  ctx.fillStyle = fillStyle ;
  assertEqual(fillStyle , ctx.fillStyle);
  // ctx.fillStyle = 'foobar';  // should be a no-op.
  // ctx.fillStyle = 42;  // should be a no-op.
  assertEqual(fillStyle , ctx.fillStyle);
}


function testCanvasBlankInitially() {
  startAsync();
  loadBlob('blank-300x150.png', function(blankBlob) {
    var canvas = google.gears.factory.create('beta.canvas');
    assertBlobsEqual(blankBlob, canvas.toBlob());
    completeAsync();
  });
}

// Tests that changing width or height resets all pixels to transparent black.
function testCanvasBlankAfterChangingDimensions() {
  var originalFilename = 'sample-original.jpeg';
  var filenames = [originalFilename,
                   'blank-313x120.png',
                   'blank-240x234.png'];
  startAsync();
  loadBlobs(filenames, function(blobs) {
    var canvas = google.gears.factory.create('beta.canvas');
    var originalBlob = blobs[originalFilename];
    canvas.load(originalBlob);
    assertEqual(313, canvas.width);
    assertEqual(234, canvas.height);

    // Test that changing the height resets the canvas.
    canvas.height = 120;
    assertBlobsEqual(blobs['blank-313x120.png'], canvas.toBlob());

    // Test that changing the width resets the canvas.
    canvas.load(originalBlob);
    canvas.width = 240;
    assertBlobsEqual(blobs['blank-240x234.png'], canvas.toBlob());

    completeAsync();
  });
}

// Tests that assigning the width or height its present value resets all
// pixels to transparent black, as the HTML5 canvas spec requires.
function testCanvasBlankAfterDimensionsSelfAssignment() {
  var originalFilename = 'sample-original.jpeg';
  var filenames = [originalFilename,
                   'blank-313x234.png'];
  startAsync();
  loadBlobs(filenames, function(blobs) {
    var canvas = google.gears.factory.create('beta.canvas');
    var originalBlob = blobs[originalFilename];
    canvas.load(originalBlob);

    // Make sure the original file has the right dimensions.
    assertEqual(313, canvas.width);
    assertEqual(234, canvas.height);

    // The canvas is not blank.
    assertNotEqual(canvas.toBlob().length, blobs['blank-313x234.png'].length);

    // Setting width to current width must blank the canvas:
    canvas.width = canvas.width;
    assertBlobsEqual(canvas.toBlob(), blobs['blank-313x234.png']);

    // Test the same for height:
    canvas.load(originalBlob);
    canvas.height = canvas.height;
    assertBlobsEqual(canvas.toBlob(), blobs['blank-313x234.png']);

    completeAsync();
  });
}

function runLoadAndExportTest(blobs) {
  var formats = ['png', 'jpeg'];
  for (var i = 0; i < formats.length; ++i) {
    var format = formats[i];
    var canvas = google.gears.factory.create('beta.canvas');
    canvas.load(blobs['sample-original.' + format]);

    var pngBlob = canvas.toBlob('image/png');

    // The default format is png.
    assertBlobsEqual(canvas.toBlob(), pngBlob);

    var jpegBlob = canvas.toBlob('image/jpeg');
    var jpegLowBlob = canvas.toBlob('image/jpeg', { quality: 0.02 });

    // Now compare the exported versions with golden files.
    // The golden files have been manually checked for format and size.
    assertBlobsEqual(pngBlob,
        blobs['sample-' + format + '-exported-as-png.png']);
    assertBlobsEqual(jpegBlob,
        blobs['sample-' + format + '-exported-as-jpeg.jpeg']);
    assertBlobsEqual(jpegLowBlob,
        blobs['sample-' + format + '-exported-as-jpeg-with-quality-0.02.jpeg']);
  }
  completeAsync();
}

function testLoadAndExport() {
  startAsync();
  var filenames = [
    'sample-original.jpeg',
    'sample-original.png',
    'sample-jpeg-exported-as-jpeg-with-quality-0.02.jpeg',
    'sample-jpeg-exported-as-jpeg.jpeg',
    'sample-jpeg-exported-as-png.png',
    'sample-png-exported-as-jpeg-with-quality-0.02.jpeg',
    'sample-png-exported-as-jpeg.jpeg',
    'sample-png-exported-as-png.png'];
  loadBlobs(filenames, runLoadAndExportTest);
}

function testLoadUnsupportedFormat() {
  var canvas = google.gears.factory.create('beta.canvas');
  startAsync();
  loadBlob('sample.txt', function(blob) {
    assertError(function() {
      canvas.load(blob);
    });
    completeAsync();
  });
}

function testExportToUnsupportedFormat() {
  var canvas = google.gears.factory.create('beta.canvas');
  assertError(function() {
    canvas.toBlob('image/gif');
  });
}

function runCloneTest(blob, dummyBlob) {
  // Create a canvas and set various properties on it and on its context.
  var originalCanvas = google.gears.factory.create('beta.canvas');
  var originalCtx = originalCanvas.getContext('gears-2d');
  originalCanvas.load(blob);

  var originalWidth = originalCanvas.width;
  var originalHeight = originalCanvas.height;
  var originalBlob = originalCanvas.toBlob();

  var alpha = 0.5;
  var compositeOperation = 'copy';
  var fillStyle = '#397F90';
  var font = '24px tahoma';
  var textAlign = 'center';

  originalCtx.globalAlpha = alpha;
  originalCtx.globalCompositeOperation = compositeOperation;
  originalCtx.fillStyle = fillStyle;
  // TODO(kart): After implementing font and textAlign, uncomment these.
  // originalCtx.font = font;
  // originalCtx.textAlign = textAlign;


  // Now clone the canvas and check that the clone and its context are
  // identical to the originals.
  var clonedCanvas = originalCanvas.clone();
  var cloneCtx = clonedCanvas.getContext('gears-2d');

  assertEqual(originalWidth, clonedCanvas.width);
  assertEqual(originalHeight, clonedCanvas.height);
  assertBlobsEqual(originalBlob, clonedCanvas.toBlob());

  assertEqual(originalCtx.globalAlpha, cloneCtx.globalAlpha);
  assertEqual(originalCtx.globalCompositeOperation,
      cloneCtx.globalCompositeOperation);
  assertEqual(originalCtx.fillStyle, cloneCtx.fillStyle);
  // assertEqual(originalCtx.font, cloneCtx.font);
  // assertEqual(originalCtx.textAlign, cloneCtx.textAlign);


  // Now perform various operations on the clone and check that the original
  // and its context aren't affected.
  clonedCanvas.resize(500, 400);
  function checkOriginal() {
    assertEqual(originalWidth, originalCanvas.width);
    assertEqual(originalHeight, originalCanvas.height);
    assertBlobsEqual(originalBlob, originalCanvas.toBlob());

    assertEqual(alpha, originalCtx.globalAlpha);
    assertEqual(compositeOperation, originalCtx.globalCompositeOperation);
    assertEqual(fillStyle, originalCtx.fillStyle);
    // assertEqual(font, ctx.font);
    // assertEqual(textAlign, ctx.textAlign);
  }
  checkOriginal();

  clonedCanvas.crop(60, 60, 130, 10);
  checkOriginal();

  clonedCanvas.width = 70;
  checkOriginal();

  clonedCanvas.height = 65;
  checkOriginal();

  // Load a random image into the clone and check that the original isn't
  // affected. We don't care what the dummy is, just that it's a valid image.
  clonedCanvas.load(dummyBlob);
  checkOriginal();
}

function testClone() {
  startAsync();
  var dummy = 'blank-313x234.png';
  loadBlobs(['sample-original.jpeg', 'sample-original.png', dummy],
      function (blobs) {
        runCloneTest(blobs['sample-original.jpeg'], blobs[dummy]);
        runCloneTest(blobs['sample-original.png'], blobs[dummy]);
        completeAsync();
      });
}

function testCrop() {
  startAsync();
  var goldenFilename = 'sample-jpeg-cropped-40-40-100-100.png';
  loadBlobs(['sample-original.jpeg', goldenFilename], function(blobs) {
    var canvas = google.gears.factory.create('beta.canvas');
    canvas.load(blobs['sample-original.jpeg']);
    canvas.crop(40, 40, 100, 100);
    var exportedBlob = canvas.toBlob();

    var goldenBlob = blobs[goldenFilename];
    // Ensure that the golden file is what we think it is.
    var goldenCanvas = google.gears.factory.create('beta.canvas');
    goldenCanvas.load(goldenBlob);
    assertEqual(100, goldenCanvas.width);
    assertEqual(100, goldenCanvas.height);

    assertBlobsEqual(goldenBlob, exportedBlob);
    completeAsync();
  });
}

function testCropToZeroSize() {
  startAsync();
  loadBlob('sample-original.png', function(blob) {
    var canvas = google.gears.factory.create('beta.canvas');
    canvas.load(blob);
    canvas.crop(40, 40, 0, 0);
    assertEqual(0, canvas.width);
    assertEqual(0, canvas.height);
    completeAsync();
  });
}

function testCropNoop() {
  startAsync();
  loadBlob('sample-original.jpeg', function(blob) {
    var canvas = google.gears.factory.create('beta.canvas');
    canvas.load(blob);
    var originalBlob = canvas.toBlob();
    var originalWidth = canvas.width;
    var originalHeight = canvas.height;
    canvas.crop(0, 0, canvas.width, canvas.height);
    assertBlobsEqual(originalBlob, canvas.toBlob());
    assertEqual(originalWidth, canvas.width);
    assertEqual(originalHeight, canvas.height);
    completeAsync();
  });
}

function testResize() {
  var originalFilename = 'sample-original.jpeg';
  var resizedFilename = 'sample-jpeg-resized-to-400x40.png';
  startAsync();
  loadBlobs([originalFilename , resizedFilename], function(blobs) {
    var canvas = google.gears.factory.create('beta.canvas');
    canvas.load(blobs[originalFilename]);

    var newWidth = 400;
    var newHeight = 40;
    canvas.resize(newWidth, newHeight);
    assertEqual(newWidth, canvas.width);
    assertEqual(newHeight, canvas.height);
    assertBlobsEqual(blobs[resizedFilename], canvas.toBlob());
    completeAsync();
  });
}

function testResizeWeirdCases() {
  var canvas = google.gears.factory.create('beta.canvas');
  assertError(function() {
    canvas.resize(-4, 9);
  });
  canvas.resize(0, 0);
  assertEqual(0, canvas.width);
  assertEqual(0, canvas.height);
}

// Use the two-argument drawImage() to clone an image.
function testDraw2ImageClone() {
  startAsync();
  loadBlob('sample-original.png', function(inputBlob) {
    var srcCanvas = google.gears.factory.create('beta.canvas');
    srcCanvas.load(inputBlob);
    var srcCanvasBeforeDrawBlob = srcCanvas.toBlob();

    var destCanvas = google.gears.factory.create('beta.canvas');
    destCanvas.width = srcCanvas.width;
    destCanvas.height = srcCanvas.height;
    var ctx = destCanvas.getContext('gears-2d');
    ctx.drawImage(srcCanvas, 0, 0);

    // The source should not have been affected.
    assertBlobsEqual(srcCanvasBeforeDrawBlob, srcCanvas.toBlob());

    // Both canvii must be the same.
    assertBlobsEqual(srcCanvas.toBlob(), destCanvas.toBlob());

    completeAsync();
  });
}

function runDrawImage2SmallerAndBiggerTest(srcFilename, goldenFilename, blobs,
    isSrcBigger, offsetX, offsetY) {
  var srcCanvas = google.gears.factory.create('beta.canvas');
  srcCanvas.load(blobs[srcFilename]);
  var srcCanvasBlobBeforeDraw = srcCanvas.toBlob();

  var destCanvas = google.gears.factory.create('beta.canvas');
  if (isSrcBigger) {
    assert(srcCanvas.width > destCanvas.width);
    assert(srcCanvas.height > destCanvas.height);
  } else {
    assert(srcCanvas.width < destCanvas.width);
    assert(srcCanvas.height < destCanvas.height);
  }

  var destCanvasOriginalWidth = destCanvas.width;
  var destCanvasOriginalHeight = destCanvas.height;

  var destCtx = destCanvas.getContext('gears-2d');
  destCtx.drawImage(srcCanvas, offsetX, offsetY);

  // The source should not have been affected.
  assertBlobsEqual(srcCanvasBlobBeforeDraw, srcCanvas.toBlob());

  // The destination's dimensions should remain untouched.
  assertEqual(destCanvasOriginalWidth, destCanvas.width);
  assertEqual(destCanvasOriginalHeight, destCanvas.height);

  assertBlobsEqual(blobs[goldenFilename], destCanvas.toBlob());
}

// Use the two-argument drawImage() (which does not resize the image while
// drawing) to draw a canvas onto a bigger one, which should leave a part of
//  the destination canvas untouched.
// Then run the same test but draw onto a smaller canvas, which should fail
// because the bigger image won't fit onto the smaller one.
// In both cases, the destination canvas's dimensions and the source canvas
// should remain unchanged.
function testDrawImage2SmallerAndBigger() {
  var smallerSrcFile = 'sample-jpeg-cropped-40-40-100-100.png';
  var biggerSrcFile = 'sample-original.png';
  var smallerGoldenFile = 'drawImage2Smaller-output.png';

  startAsync();
  loadBlobs([smallerSrcFile, biggerSrcFile, smallerGoldenFile],
      function(blobs) {
        runDrawImage2SmallerAndBiggerTest(smallerSrcFile, smallerGoldenFile,
            blobs, false, 10, 35);
        assertError(function() {
            runDrawImage2SmallerAndBiggerTest(biggerSrcFile, null,
              blobs, true, 0, 0);
            }, 'Destination rectangle stretches beyond');
        completeAsync();
      });
}

// Use the two-argument drawImage() to draw an image at various invalid offsets.
// Since a rectangle has four edges and four corners, there are eight kinds of
// invalid offsets, all of which we test.
function testDrawImage2ToBadOffset() {
  startAsync();
  loadBlob('sample-original.png', function(inputBlob) {
    var srcCanvas = google.gears.factory.create('beta.canvas');
    srcCanvas.load(inputBlob);

    var destCanvas = google.gears.factory.create('beta.canvas');
    var ctx = destCanvas.getContext('gears-2d');

    assertError(function() {
      ctx.drawImage(srcCanvas, -4, -3);
    });
    assertError(function() {
      ctx.drawImage(srcCanvas, -4, 0);
    });
    assertError(function() {
      ctx.drawImage(srcCanvas, 0, -3);
    });
    assertError(function() {
      ctx.drawImage(srcCanvas, 2 * canvas.width, 2 * canvas.height);
    });
    assertError(function() {
      ctx.drawImage(srcCanvas, 2 * canvas.width, 0);
    });
    assertError(function() {
      ctx.drawImage(srcCanvas, 0, 2 * canvas.height);
    });
    assertError(function() {
      ctx.drawImage(srcCanvas, 2 * canvas.width, -3);
    });
    assertError(function() {
      ctx.drawImage(srcCanvas, -4, 2 * canvas.height);
    });

    completeAsync();
  });
}

function testDrawImage2WithAlphaZeroIsNoop() {
  startAsync();
  loadBlob('sample-original.png', function(inputBlob) {
    var srcCanvas = google.gears.factory.create('beta.canvas');
    srcCanvas.load(inputBlob);

    var destCanvas = google.gears.factory.create('beta.canvas');
    // Make the destCanvas big enough to hold the image in the srcCanvas.
    destCanvas.width = 400;
    destCanvas.height = 400;
    var ctx = destCanvas.getContext('gears-2d');
    var destSnapshot = destCanvas.toBlob();

    ctx.globalAlpha = 0;

    // Should be a noop.
    ctx.drawImage(srcCanvas, 3, 9);

    assertBlobsEqual(destSnapshot, destCanvas.toBlob());
    completeAsync();
    });
}

// TODO(kart): restore should do nothing if the stack is
// empty (i.e, blank canvas)

// TODO(kart): clearRect and fillRect have no effect is width or height is zero.
// strokeRect has no effect if both width and height are zero, and draws a line
// if exactly one of width and height is zero.

// TODO(kart): If any of the arguments to createImageData() or getImageData()
// are infinite or NaN, or if either the sw or sh arguments are zero, the method
// must instead raise an INDEX_SIZE_ERR exception.

// TODO(kart): The values of the data array may be changed
// (the length of the array,
//  and the other attributes in ImageData objects, are all read-only).
// On setting, JS undefined values must be converted to zero. Other values must
// first be converted to numbers using JavaScript's ToNumber algorithm, and if
// the result is a NaN value, a TYPE_MISMATCH_ERR exception must be raised. If
// the result is less than 0, it must be clamped to zero. If the result is more
// than 255, it must be clamped to 255. If the number is not an integer, it must
//  be rounded to the nearest integer using the IEEE 754r roundTiesToEven
// rounding mode.

// TODO(kart): putImageData: If the first argment to the method is null or
//  not an
// ImageData object that was returned by createImageData() or getImageData()
// then the putImageData() method must raise a TYPE_MISMATCH_ERR exception. If
// any of the arguments to the method are infinite or NaN, the method must raise
// an INDEX_SIZE_ERR exception.

// TODO(kart): make sure
// context.putImageData(context.getImageData(x, y, w, h), x, y);
// is a noop.

// NPAPI-TEMP - Disable tests that don't currently work in npapi build.
testGetContext._disable_in_npapi = true;