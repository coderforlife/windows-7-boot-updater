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

#include "stdafx-pure.h"
#include "stdafx-native.h"

// Interop
template<typename T> __forceinline T *as_native(array<T> ^a) { pin_ptr<T> p = &a[0]; return p; }
#define NATIVE(a) as_native(a), a->Length

#include <vcclr.h>
__forceinline const wchar_t *as_native(string s) { pin_ptr<const wchar_t> p = PtrToStringChars(s); return p; }
__forceinline string as_managed(wchar_t *s) { return System::Runtime::InteropServices::Marshal::PtrToStringUni((System::IntPtr)s); }
