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

#include "Winload.h"

#include "Bcd.h"
#include "UI.h"
#include "WinXXX.h"

using namespace Win7BootUpdater;

using namespace System;
using namespace System::IO;

uint Winload::Check(string path) { return WinXXX::Check(path, false); }
uint Winload::CheckMui(string path) { return WinXXX::CheckMui(path, false); }
string Winload::GetDefaultStartupMsg() { return WinXXX::GetDefaultStartupMsg(false); }
string Winload::def::get() {
	if (!_def) {
		try {
			_def = BCD::GetFilePath(BCD::Current, L"winload.exe");
		} catch (Exception ^ex) {
			UI::ShowError(ex->Message, UI::GetMessage(Msg::ErrorWhileGettingPath, L"winload.exe"));
			_def = defFallBack;
		}
	}
	return _def;
}
string Winload::defMui::get() {
	if (!_defMui)
		_defMui = Path::Combine(Path::Combine(Path::GetDirectoryName(def), BCD::PreferredLocale), Path::GetFileName(def) + L".mui");
	return _defMui;
}
