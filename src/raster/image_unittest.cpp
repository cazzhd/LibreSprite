/* Aseprite
 * Copyright (C) 2001-2013  David Capello
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtest/gtest.h>

#include "base/unique_ptr.h"
#include "raster/image.h"
#include "raster/image_bits.h"
#include "raster/primitives.h"

using namespace base;
using namespace raster;

template<typename T>
class ImageAllTypes : public testing::Test {
protected:
  ImageAllTypes() { }
};

typedef testing::Types<RgbTraits, GrayscaleTraits, IndexedTraits, BitmapTraits> ImageAllTraits;
TYPED_TEST_CASE(ImageAllTypes, ImageAllTraits);

TYPED_TEST(ImageAllTypes, PutGetAndIterators)
{
  typedef TypeParam ImageTraits;

  std::vector<int> lengths;
  lengths.push_back(1);
  lengths.push_back(4);
  lengths.push_back(7);
  lengths.push_back(8);
  lengths.push_back(9);
  lengths.push_back(15);
  lengths.push_back(16);
  lengths.push_back(17);
  lengths.push_back(31);
  lengths.push_back(32);
  lengths.push_back(33);

  std::vector<gfx::Size> sizes;
  for (size_t i=0; i<lengths.size(); ++i)
    for (size_t j=0; j<lengths.size(); ++j)
      sizes.push_back(gfx::Size(lengths[j], lengths[i]));

  for (std::vector<gfx::Size>::iterator sizes_it=sizes.begin(); sizes_it!=sizes.end(); ++sizes_it) {
    int w = sizes_it->w;
    int h = sizes_it->h;
    UniquePtr<Image> image(Image::create(ImageTraits::pixel_format, w, h));
    std::vector<int> data(w*h);

    for (int y=0; y<h; ++y)
      for (int x=0; x<w; ++x)
        data[y*w+x] = (std::rand() % ImageTraits::max_value);

    for (int i=0; i<w*h; ++i)
      put_pixel(image, i%w, i/w, data[i]);

    for (int i=0; i<w*h; ++i)
      ASSERT_EQ(data[i], get_pixel(image, i%w, i/w));

    std::vector<gfx::Rect> areas;

    // Read-only iterator (whole image)
    {
      const LockImageBits<ImageTraits> bits((const Image*)image);
      typename LockImageBits<ImageTraits>::const_iterator
        begin = bits.begin(),
        it = begin,
        end = bits.end();

      for (int i=0; it != end; ++it, ++i) {
        assert(data[i] == *it);
        ASSERT_EQ(data[i], *it);
      }
    }

    // Read-only iterator (areas)
    for (int i=0; ; ++i) {
      gfx::Rect bounds(i, i, w-i*2, h-i*2);
      if (bounds.w <= 0 || bounds.h <= 0)
        break;

      const LockImageBits<ImageTraits> bits((const Image*)image, bounds);
      typename LockImageBits<ImageTraits>::const_iterator
        begin = bits.begin(),
        it = begin,
        end = bits.end();

      for (int y=bounds.y; y<bounds.y+bounds.h; ++y) {
        for (int x=bounds.x; x<bounds.x+bounds.w; ++x, ++it) {
          SCOPED_TRACE(x);
          SCOPED_TRACE(y);

          ASSERT_TRUE(it != end);
          EXPECT_EQ(data[y*w+x], *it);
        }
      }

      EXPECT_TRUE(it == end);
    }

    // Write iterator (whole image)
    {
      LockImageBits<ImageTraits> bits(image, Image::WriteLock);
      typename LockImageBits<ImageTraits>::iterator
        begin = bits.begin(),
        it = begin,
        end = bits.end();

      for (int i=0; it != end; ++it, ++i) {
        *it = 1;
        EXPECT_EQ(1, *it);
      }

      it = begin;
      for (int i=0; it != end; ++it, ++i) {
        EXPECT_EQ(1, *it);
      }
    }
  }
}

TEST(Image, DiffRgbImages)
{
  UniquePtr<Image> a(Image::create(IMAGE_RGB, 32, 32));
  UniquePtr<Image> b(Image::create(IMAGE_RGB, 32, 32));

  clear_image(a, rgba(0, 0, 0, 0));
  clear_image(b, rgba(0, 0, 0, 0));

  ASSERT_EQ(0, count_diff_between_images(a, b));

  put_pixel(a, 0, 0, rgba(255, 0, 0, 0));
  ASSERT_EQ(1, count_diff_between_images(a, b));

  put_pixel(a, 1, 1, rgba(0, 0, 255, 0));
  ASSERT_EQ(2, count_diff_between_images(a, b));
}

TYPED_TEST(ImageAllTypes, DrawHLine)
{
  typedef TypeParam ImageTraits;

  std::vector<int> lengths;
  lengths.push_back(7);
  lengths.push_back(8);
  lengths.push_back(9);
  lengths.push_back(15);
  lengths.push_back(16);
  lengths.push_back(17);
  lengths.push_back(31);
  lengths.push_back(32);
  lengths.push_back(33);

  std::vector<gfx::Size> sizes;
  for (size_t i=0; i<lengths.size(); ++i)
    for (size_t j=0; j<lengths.size(); ++j)
      sizes.push_back(gfx::Size(lengths[j], lengths[i]));

  for (std::vector<gfx::Size>::iterator sizes_it=sizes.begin(); sizes_it!=sizes.end(); ++sizes_it) {
    int w = sizes_it->w;
    int h = sizes_it->h;
    UniquePtr<Image> image(Image::create(ImageTraits::pixel_format, w, h));
    image->clear(0);
    
    for (int c=0; c<100; ++c) {
      int x = rand() % w;
      int y = rand() % h;
      int x2 = x + (rand() % (w-x));
      image->drawHLine(x, y, x2, rand() % ImageTraits::max_value);
    }
  }
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}