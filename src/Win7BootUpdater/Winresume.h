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

namespace Win7BootUpdater {
	/// <remarks>The static class for information about and checking the 'winresume.exe' file.</remarks>
	PUBLIC ref struct Winresume abstract sealed {
	private:
		static string _def = nullptr, _defMui = nullptr;

	public:
		/// <summary>Default location of winresume.exe</summary>
		static property string def { string get(); };
		/// <summary>Default location of winresume.exe.mui</summary>
		static property string defMui { string get(); };

		/// <summary>The default path for winresume.exe in case the real winresume.exe path cannot be found</summary>
		static initonly string defFallBack = System::IO::Path::Combine(System::Environment::GetFolderPath(System::Environment::SpecialFolder::System), L"winresume.exe");

		/// <summary>
		/// Checks to see if the file is really winresume.exe. These checks are not completely thorough, but are fairly good.
		/// A shorthand for <see cref="WinXXX::Check" /> with true.
		/// </summary>
		/// <param name="path">The path of the file to check</param>
		/// <returns>The error code of the check. If it is 0 there is no error, otherwise pass it to <see cref="UI::ShowError(string,string,uint,string)" /> to process it.</returns>
		static uint Check(string path);
		/// <summary>
		/// Checks to see if the file is really winresume.exe.mui. These checks are not completely thorough, but are fairly good.
		/// A shorthand for <see cref="WinXXX::CheckMui" /> with true.
		/// </summary>
		/// <param name="path">The path of the file to check</param>
		/// <returns>The error code of the check. If it is 0 there is no error, otherwise pass it to <see cref="UI::ShowError(string,string,uint,string)" /> to process it.</returns>
		static uint CheckMui(string path);

		/// <summary>
		/// Gets the default resume message ("Resuming Windows") in the system winresume.
		/// A shorthand for <see cref="WinXXX::GetDefaultStartupMsg" /> with true.
		/// </summary>
		/// <returns>The default resume message.</returns>
		static string GetDefaultResumeMsg();
	};
}
