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

#include "FileUpdater.h"

namespace Win7BootUpdater {
	/// <remarks>The static class for information about and checking the 'bootres.dll' file.</remarks>
	PUBLIC ref struct Bootres abstract sealed {
	private:
		static string _def = nullptr;

	public:
#pragma warning(push)
#pragma warning(disable:4693)
		/// <summary>The file name of the animation within the bootres.dll WIM file</summary>
		literal string activity = L"activity.bmp";
		
		/// <summary>Default name of the bootres.dll file</summary>
		literal string name = L"bootres.dll";

		/// <summary>Name of the alternative bootres.dll file used when winload.exe and winresume.exe use different animations</summary>
		literal string altName = L"bootrs2.dll";
		
		/// <summary>Path of the alternative bootres.dll file used when winload.exe and winresume.exe use different animations</summary>
		literal string altPath = L"\\system32\\bootrs2.dll";
#pragma warning(pop)

		/// <summary>Retrieves the animation image from the given bootres.dll</summary>
		/// <param name="path">The path of a bootres.dll file</param>
		/// <returns>The animation, or null if an error occurred</returns>
		static System::Drawing::Bitmap ^GetAnimation(System::String ^path);
		
		/// <summary>Default location of bootres.dll</summary>
		static property string def { string get(); };

		/// <summary>Checks to see if the file is really bootres. These checks are not completely thorough, but are fairly good.</summary>
		/// <param name="path">The path of the file to check</param>
		/// <returns>The error code of the check. If it is 0 there is no error, otherwise pass it to <see cref="UI::ShowError(string,string,uint,string)" /> to process it.</returns>
		static uint Check(string path);

	internal:
		// 17 increments
		static uint Update(System::Drawing::Image ^anim, System::Drawing::Color bgColor, System::Drawing::Image ^bgImg, FileUpdater bootres);
	};
}