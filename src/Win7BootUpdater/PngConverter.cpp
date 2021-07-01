/*
 * Windows 7 Boot Updater (github.com/coderforlife/windows-7-boot-updater)
 * Copyright (C) 2021  Jeffrey Bush - Coder for Life
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "PngConverter.h"

#include "Utilities.h"

#include "libpng\png.h"
#include "libpng\pngstruct.h"
#include "libpng\pnginfo.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;

using namespace Win7BootUpdater;

#define COMPRESSION_LEVEL	9

#pragma unmanaged
typedef struct _write_png_io {
	size_t size;
	size_t pos; 
	unsigned char *x;
} write_png_io;

static void write_png(png_structp png, png_bytep data, png_size_t len) {
	write_png_io *out = (write_png_io*)png->io_ptr;
	while (out->pos + len > out->size)
		out->x = (unsigned char *)realloc(out->x, (out->size <<= 1));
	memcpy(out->x+out->pos, data, len);
	out->pos += len;
}

#pragma managed
static unsigned char *GetPngBytesWithAlpha(Bitmap ^b, size_t *size) { // return must be freed
	unsigned long w = b->Width, h = b->Height, W = w*4;

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) { return NULL; }

	png_infop info = png_create_info_struct(png);
	if (!info) { png_destroy_write_struct(&png, NULL); return NULL; }

	write_png_io out;
	out.pos = 0;
	out.size = w*h*4/10; // assume that the compression will be about 10% of the raw data size
	out.x = (unsigned char *)malloc(out.size);

	png_set_write_fn(png, &out, &write_png, NULL);
	png_set_compression_level(png, COMPRESSION_LEVEL);
	png_set_IHDR(png, info, w, h, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);
	
	System::Drawing::Rectangle r = System::Drawing::Rectangle(0, 0, w, h);
	BitmapData ^bmp_data = b->LockBits(r, ImageLockMode::ReadOnly, PixelFormat::Format32bppArgb);

	unsigned long s = bmp_data->Stride;
	unsigned char *data = (unsigned char *)bmp_data->Scan0.ToPointer();
	unsigned char *stride = (unsigned char *)malloc(s);

	for (unsigned long y = 0; y < h; ++y) {
		for (unsigned long x = 0; x < W; x += 4) {
			stride[x  ] = data[x+2];
			stride[x+1] = data[x+1];
			stride[x+2] = data[x  ];
			stride[x+3] = data[x+3];
		}
		png_write_row(png, stride);
		data += s;
	}

	b->UnlockBits(bmp_data);

	png_write_end(png, NULL);
	png_destroy_write_struct(&png, &info);

	*size = out.pos;
	return out.x;
}

static unsigned char *GetPngBytesWithoutAlpha(Bitmap ^b, size_t *size) { // return must be freed
	unsigned long w = b->Width, h = b->Height, W = w*3;

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) { return NULL; }

	png_infop info = png_create_info_struct(png);
	if (!info) { png_destroy_write_struct(&png, NULL); return NULL; }

	write_png_io out;
	out.pos = 0;
	out.size = w*h*3/10; // assume that the compression will be about 10% of the raw data size
	out.x = (unsigned char *)malloc(out.size);

	png_set_write_fn(png, &out, &write_png, NULL);
	png_set_compression_level(png, COMPRESSION_LEVEL);
	png_set_IHDR(png, info, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);
	
	System::Drawing::Rectangle r = System::Drawing::Rectangle(0, 0, w, h);
	BitmapData ^bmp_data = b->LockBits(r, ImageLockMode::ReadOnly, PixelFormat::Format24bppRgb);

	unsigned long s = bmp_data->Stride;
	unsigned char *data = (unsigned char *)bmp_data->Scan0.ToPointer();
	unsigned char *stride = (unsigned char *)malloc(s);

	for (unsigned long y = 0; y < h; ++y) {
		for (unsigned long x = 0; x < W; x += 3) {
			stride[x  ] = data[x+2];
			stride[x+1] = data[x+1];
			stride[x+2] = data[x  ];
		}
		png_write_row(png, stride);
		data += s;
	}

	b->UnlockBits(bmp_data);

	png_write_end(png, NULL);
	png_destroy_write_struct(&png, &info);

	*size = out.pos;
	return out.x;
}

array<System::Byte> ^PngConverter::GetBytes(Bitmap ^b) {
	size_t size;
	unsigned char *data =
		(b->PixelFormat != PixelFormat::Format24bppRgb && b->PixelFormat != PixelFormat::Format16bppGrayScale &&
		b->PixelFormat != PixelFormat::Format16bppRgb555 && b->PixelFormat != PixelFormat::Format16bppRgb565 &&
		b->PixelFormat != PixelFormat::Format32bppRgb && b->PixelFormat != PixelFormat::Format48bppRgb) ?
		GetPngBytesWithAlpha(b, &size) : GetPngBytesWithoutAlpha(b, &size);
	array<System::Byte> ^bytes = Utilities::GetManagedArray(data, size);
	free(data);
	return bytes;
}
