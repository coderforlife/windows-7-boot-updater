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
	/// <remarks>The static class for information about and checking the 'bootmgr' file.</remarks>
	PUBLIC ref struct Bootmgr abstract sealed {
	private:
		static string _def = nullptr;

	public:
		/// <summary>The default path for bootmgr</summary>
		static property string def { string get(); };

		/// <summary>The default path for bootmgr in case the real bootmgr path cannot be found</summary>
		static initonly string defFallBack = System::IO::Path::Combine(System::IO::Path::GetPathRoot(System::Environment::GetFolderPath(System::Environment::SpecialFolder::System)), L"bootmgr");

		/// <summary>Checks if a particular path is on a hidden partition</summary>
		/// <param name="path">The path to check</param>
		/// <returns>True if the path is on a hidden partition, false otherwise</returns>
		static bool IsOnHiddenSystemPartition(string path);

		/// <summary>Checks if the default bootmgr path (<see cref="def"/>) is on a hidden partition</summary>
		/// <returns>True if the default path is on a hidden partition, false otherwise</returns>
		static bool DefaultIsOnHiddenSystemPartition();

		/// <summary>Checks to see if the file is really bootmgr. These checks are not completely thorough, but are fairly good.</summary>
		/// <param name="path">The path of the file to check</param>
		/// <returns>The error code of the check. If it is 0 there is no error, otherwise pass it to <see cref="UI::ShowError(string,string,uint,string)" /> to process it.</returns>
		static uint Check(string path);

	internal:
		// 6 increments
		static uint Update(FileUpdater bootmgr);
	};
}
