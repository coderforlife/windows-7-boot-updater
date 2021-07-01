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

#define VERSION_RELEASE	3
#define VERSION_RC		2
#define VERSION_BETA	1
#define VERSION_ALPHA	0

#define VERSION_MAJOR	0
#define VERSION_MINOR	0
#define VERSION_STATUS	VERSION_BETA
#define VERSION_REV		3

#define VERSION_RES		"0.0.1.3"
#define VERSION_FULL	L"0.0.1.3"
#define VERSION_ASM		L"0.0.1.3" // could also be 0.0.*
#define VERSION_DESC	L"v0 Beta 3"

namespace Win7BootUpdater {
	/// <remarks>Version information about the the Windows 7 Boot Updater.</remarks>
	PUBLIC ref struct Version abstract sealed {
#pragma warning(push)
#pragma warning(disable:4693)
		/// <summary>The assembly version number, could be something like 0.0.1.1 or 0.0.*</summary>
		literal string Asm = VERSION_ASM;
		/// <summary>The full version number, something like 0.0.1.1</summary>
		literal string Full = VERSION_FULL;
		/// <summary>The textual version number, something like v0 Beta 1</summary>
		literal string Desc = VERSION_DESC;
#pragma warning(pop)

		/// <summary>The version object representation of VERSION_FULL</summary>
		static initonly System::Version ^Ver = gcnew System::Version(VERSION_FULL); //System::Version::Parse(VERSION_FULL);
	};
}
