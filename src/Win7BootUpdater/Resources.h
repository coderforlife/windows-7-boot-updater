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

#include "Version.h"
#include "Patch.h"

namespace Win7BootUpdater {
	ref class Res abstract sealed {
	private:
/*#pragma warning(push)
#pragma warning(disable:4693)
		literal string server = L"http://coderforlife.com/projects/win7boot/updates.php?";
		literal string referer = L"http://coderforlife.com/projects/win7boot/RESOURCE_UPDATER/" VERSION_FULL;
#pragma warning(pop)
		static initonly object lock = gcnew System::Object();
		static string unique_id = nullptr;
		static bool first_request = true;
		static initonly System::Collections::Generic::Dictionary<string,Patches::PatchFile^> ^patches = gcnew System::Collections::Generic::Dictionary<string,Patches::PatchFile^>();*/
		static initonly System::Resources::ResourceManager ^resources = gcnew System::Resources::ResourceManager(L"Win7BootUpdater", System::Reflection::Assembly::GetExecutingAssembly());
		//static System::IO::Stream ^GetUpdate(string file);

	public:
		//static string GetUniqueID(); // a 32 character long hex string
		
		//static object Get(string name);
		//static array<byte> ^GetData(string name);
		//static string GetString(string name);
		
		static System::IO::Stream ^GetStream(string name);
		static System::IO::Stream ^GetCompressedStream(string name);
		
		static Patches::PatchFile ^GetPatch(string name);
		//static System::Version ^GetLatestAvailableVersion();

		static array<string> ^GetAvailableLanguages();
		static System::IO::Stream ^GetLanguageStream(string lang);
	};
}
