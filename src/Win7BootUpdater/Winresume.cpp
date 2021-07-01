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

#include "Winresume.h"

#include "Bcd.h"
#include "UI.h"
#include "WinXXX.h"

using namespace Win7BootUpdater;

using namespace System;
using namespace System::IO;

uint Winresume::Check(string path) { return WinXXX::Check(path, true); }
uint Winresume::CheckMui(string path) { return WinXXX::CheckMui(path, true); }
string Winresume::GetDefaultResumeMsg() { return WinXXX::GetDefaultStartupMsg(true); }
string Winresume::def::get() {
	if (!_def) {
		try {
			_def = BCD::GetFilePath(BCD::GetBootLoader(BCD::GetGUID(BCD::Current, BcdOSLoaderObject_AssociatedResumeObject)), L"winresume.exe");
		} catch (Exception ^ex) {
			UI::ShowError(ex->Message, UI::GetMessage(Msg::ErrorWhileGettingPath, L"winresume.exe"));
			_def = defFallBack;
		}
	}
	return _def;
}
string Winresume::defMui::get() {
	if (!_defMui)
		_defMui = Path::Combine(Path::Combine(Path::GetDirectoryName(def), BCD::PreferredLocale), Path::GetFileName(def) + L".mui");
	return _defMui;
}
