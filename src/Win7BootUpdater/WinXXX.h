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
#include "BootSkin.h"

namespace Win7BootUpdater {
	/// <remarks>The static class for information about and checking the 'winload.exe' or 'winresume.exe' files.</remarks>
	PUBLIC ref struct WinXXX abstract sealed {
	public:
#pragma warning(push)
#pragma warning(disable:4693)
		/// <summary>Default copyright message in both winload and winresume</summary>
		literal string CopyrightDefault = L"� Microsoft Corporation";
#pragma warning(pop)
		
		/// <summary>Gets the closest possible background color for a given color.</summary>
		/// <param name="c">The color to analyze</param>
		/// <returns>The color in <see cref="BackgroundColors" /> that is closest to the given color.</returns>
		static System::Drawing::Color GetClosestBGColor(System::Drawing::Color c);

		/// <summary>The possible colors for the solid background of the boot screen.</summary>
		static initonly array<int> ^BackgroundColors = gcnew array<int>{
			0xFFFFFF, 0xFFFF55, 0xFF55FF, 0xFF5555, 0x55FFFF, 0x55FF55, 0x5555FF, 0x555555,
			0xAAAAAA, 0xAA5500, 0xAA00AA, 0xAA0000, 0x00AAAA, 0x00AA00, 0x0000AA, 0x000000
		};

		/// <summary>Checks to see if the file is really winload.exe or winresume.exe. These checks are not completely thorough, but are fairly good.</summary>
		/// <param name="path">The path of the file to check</param>
		/// <param name="winresume">If true, checks if the file is winresume.exe, otherwise checks if it is winload.exe</param>
		/// <returns>The error code of the check. If it is 0 there is no error, otherwise pass it to <see cref="UI::ShowError(string,string,uint,string)" /> to process it.</returns>
		static uint Check(string path, bool winresume);

		/// <summary>Checks to see if the file is really winload.exe.mui or winresume.exe.mui. These checks are not completely thorough, but are fairly good.</summary>
		/// <param name="path">The path of the file to check</param>
		/// <param name="winresume">If true, checks if the file is winresume.exe.mui, otherwise checks if it is winload.exe.mui</param>
		/// <returns>The error code of the check. If it is 0 there is no error, otherwise pass it to <see cref="UI::ShowError(string,string,uint,string)" /> to process it.</returns>
		static uint CheckMui(string path, bool winresume);
		
		/// <summary>Gets the default startup message ("Starting Windows" or "Resuming Windows") in the system winload or winresume.</summary>
		/// <param name="winresume">If true, looks for the message in winresume.exe.mui, otherwise looks in winload.exe.mui</param>
		/// <returns>The default startup message.</returns>
		static string GetDefaultStartupMsg(bool winresume);

	internal:
		static int GetClosestBGColorIndex(System::Drawing::Color c);
		static string GetXMLColor(System::Drawing::Color c);
		static System::Drawing::Color GetColorFromXml(string xml);
		static initonly array<string> ^ColorXMLs = gcnew array<string>{
			L"RGBI", L"RGXI", L"RXBI", L"RXXI", L"XGBI", L"XGXI", L"XXBI", L"XXXI",
			L"RGBX", L"RGXX", L"RXBX", L"RXXX", L"XGBX", L"XGXX", L"XXBX", L"XXXX"
		};

		static void GetProperties(string path, BootSkinFile ^bs);

		// 6 increments
		static uint Update(int msgCount, System::Drawing::Color bg, string text, array<int> ^textSize, array<int> ^textPos, array<System::Drawing::Color> ^textColor, bool altBootres, FileUpdater winload, bool winresume); // text messages
		static uint Update(bool altBootres, FileUpdater winload, bool winresume); // background image

		// 6 increments
		static uint UpdateRes(string text, System::Drawing::Color color, FileUpdater winload, bool winresume); // text messages
		static uint UpdateRes(System::Drawing::Image ^bg, System::Drawing::Color color, FileUpdater winload, bool winresume); // background
	};
}
