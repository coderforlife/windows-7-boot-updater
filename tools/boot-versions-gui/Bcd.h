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

#include "BcdConstants.h"
#include "BcdObject.h"

namespace Win7BootUpdater {
	ref struct BCD abstract sealed {
	private:
		static string storePath = L"";
		static System::Management::ManagementScope ^scope;
		static ROOT::WMI::BcdObject ^bootmgr, ^current;
		static string preferredLocale = nullptr;
		static bool initialized = false;
		static void Init();
		static string GetDevicePath(ROOT::WMI::BcdObject ^o, unsigned int t);
		static string GetString(ROOT::WMI::BcdObject ^o, unsigned int t);

	public:
		static property string StorePath { string get(); }								// can throw exception
		static property string PreferredLocale { string get(); }						// does not throw exceptions
		static string GetGUID(ROOT::WMI::BcdObject ^o, unsigned int t);					// can throw exception
		static string GetFilePath(ROOT::WMI::BcdObject ^o, string name);				// can throw exception
		static ROOT::WMI::BcdObject ^GetBootLoader(string guid);						// can throw exception
		static property ROOT::WMI::BcdObject ^Bootmgr { ROOT::WMI::BcdObject ^get(); }	// can throw exception
		static property ROOT::WMI::BcdObject ^Current { ROOT::WMI::BcdObject ^get(); }  // can throw exception, fallback to Default if current is not available
	};
}
