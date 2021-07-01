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

#include "UI-native.h"
#undef UI
#include "UI.h"

using namespace Win7BootUpdater;

using namespace System;

int UINative::Inc() { return UI::Inc(); }
//int UINative::Inc(int x) { return UI::Inc(x); }
//int UINative::Inc(LPCWSTR text) { return UI::Inc(gcnew String(text)); }
//void UINative::SetProgressText(LPCWSTR text) { UI::ProgressText = gcnew String(text); }
