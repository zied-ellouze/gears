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

#ifndef GEARS_IMAGE_BACKING_IMAGE_H__
#define GEARS_IMAGE_BACKING_IMAGE_H__

#ifdef OFFICIAL_BUILD
// The Image API has not been finalized for official builds
#else

#include "gears/third_party/libgd/gd.h"
#include "gears/blob/blob_interface.h"

// To create an image, construct it, then initialize it from a blob with
// Init(blob).  If Init is not called, or returns false, the
// image is not initialized and should not be used until it is initialized.
class BackingImage {
 public:
  // IJG quality value in the range [0..95], or -1 for the libGD default:
  static const int kJpegQuality = -1;

  // This is the maximum width or height that can be generated by this module.
  // Note that it is possible to load an image that is larger than this.
  static const int kMaxImageDimension = 4096;

  enum ImageFormat {
    FORMAT_PNG = 0,
    FORMAT_GIF = 1,
    FORMAT_JPEG = 2
  };

  // Creates an uninitialised image.
  BackingImage();

  // Initializes this with the contents of blob and returns true, or returns
  // false on failure.
  bool Init(BlobInterface *blob, std::string16 *error);

  // Resizes the image, or returns false if width or height are negative, or if
  // width or height are larger than kMaxImageDimension.
  bool Resize(int width, int height, std::string16 *error);

  // Crops the image to the bounding box with a displacement of (x, y) from the
  // left and top edges respectively, and the given width and height.  Returns
  // false if the bounding box is invalid or out of bounds.
  bool Crop(int x, int y, int width, int height, std::string16 *error);

  // Rotates by degrees anti-clockwise.  Only supports orthogonal rotations for
  // now.  Non-orthogonal rotations return false.
  bool Rotate(int degrees, std::string16 *error);

  // Flips the image about the vertical axis.
  bool FlipHorizontal(std::string16 *error);

  // Flips the image about the horizontal axis.
  bool FlipVertical(std::string16 *error);

  // Composes the given image at the given displacement from the left and top.
  bool DrawImage(const BackingImage* img, int x, int y, std::string16 *error);

  int Width() const;

  int Height() const;

  // Returns a blob containing the image data in the format specified.
  void ToBlob(scoped_refptr<BlobInterface> *out, ImageFormat format);

  // Returns a blob of the format from which it was originally saved.
  void ToBlob(scoped_refptr<BlobInterface> *out);

  ~BackingImage();

 private:
  gdImagePtr img_ptr_;

  ImageFormat original_format_;
};

#endif  // not OFFICIAL_BUILD

#endif  // GEARS_IMAGE_BACKING_IMAGE_H__
