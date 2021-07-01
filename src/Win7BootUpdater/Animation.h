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

#pragma once

#include "UI.h"

namespace Win7BootUpdater {
	/// <remarks>The static class for loading and creating the animation of the boot process</remarks>
	PUBLIC ref struct Animation abstract sealed {
	public:
#pragma warning(push)
#pragma warning(disable:4693)
		/// <summary>The width of a single frame of the animation</summary>
		literal int Width = 200;
		/// <summary>The height of a single frame of the animation</summary>
		literal int Height = 200;
		/// <summary>The total number of frames in the animation</summary>
		literal int Frames = 105;
		/// <summary>The full height of the animation file, including all frames</summary>
		literal int FullHeight = 21000;
		/// <summary>The frame at which the animation loops</summary>
		literal int LoopFrame = 61;
		/// <summary>The width of the boot screen</summary>
		literal int ScreenWidth = 1024;
		/// <summary>The height of the boot screen</summary>
		literal int ScreenHeight = 768;
		/// <summary>The X position of the animation on the screen</summary>
		literal int X = (ScreenWidth - Width) / 2;
		/// <summary>The Y position of the animation on the screen</summary>
		literal int Y = (ScreenHeight - Height) / 2;
#pragma warning(pop)

		/// <summary>The description for a directory that contains a valid animation</summary>
		static property string DirDesc { string get(); };

		/// <summary>Create an animation from a single static image file</summary>
		/// <param name="file">The image file to use</param>
		/// <returns>An image representing the full animation with the static image repeated <see cref="Frames" /> times</returns>
		static System::Drawing::Image ^CreateFromSingle(string file);

		/// <summary>Create an animation from a single static image</summary>
		/// <param name="src">The source bitmap to use</param>
		/// <returns>An image representing the full animation with the static image repeated <see cref="Frames" /> times</returns>
		static System::Drawing::Image ^CreateFromSingle(System::Drawing::Bitmap ^src);

		/// <summary>Create an animation from a folder of images</summary>
		/// <param name="folder">The folder to use</param>
		/// <returns>An image representing the full animation</returns>
		static System::Drawing::Image ^CreateFromFolder(string folder);

	internal:
		static initonly System::Drawing::Color Transparent = System::Drawing::Color::FromArgb(0x00000000);
		static initonly array<string> ^ext = gcnew array<string>{L"bmp",L"png",L"gif",L"jpg",L"jpeg",L"tif"}; //tiff is automatically included due to weird rules
		static void CreateFromSingle(System::Drawing::Image ^src, System::Drawing::Rectangle srcRect, System::Drawing::Bitmap ^b, System::Drawing::Graphics ^g);
		static System::Drawing::Image ^ResolveTransparency(System::Drawing::Image ^img, int width, int height, System::Drawing::Color bg, System::Drawing::Image ^animBgImg);
		static System::Drawing::Image ^CreateFromData(array<byte> ^data);

		/*
		/// <summary>Saves the image as a PNG and then gets the bytes of that file (not real files, all in memory)</summary>
		/// <param name="img">The image to get the bytes of</param>
		/// <returns>The bytes representing the PNG format of the image</returns>
		*/
		static array<byte> ^GetPngData(System::Drawing::Image ^img);
	};
}
