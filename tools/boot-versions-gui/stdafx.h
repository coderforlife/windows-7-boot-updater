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

#define InBootVersions // causes changes to files that are direct copies from the complete Windows 7 Boot Updater

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#pragma comment(lib, "Version.lib")		// for VerQueryValue/GetFileVersionInfoSize/GetFileVersionInfo

// For PathFileExists
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include "product-types.h"

typedef System::String ^string;
typedef System::Object ^object;
typedef System::Byte byte;
typedef System::UInt16 ushort;
typedef System::UInt32 uint;
typedef System::UInt64 ulong;
typedef System::SByte sbyte;
typedef System::Decimal decimal;

typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

#include <vcclr.h>
__forceinline const wchar_t *as_native(string s) { pin_ptr<const wchar_t> p = PtrToStringChars(s); return p; }

#ifndef _WIN64
#define DISABLE_FS_REDIR()	PVOID _old_state; BOOL _disabled = Wow64DisableWow64FsRedirection(&_old_state)
#define REVERT_FS_REDIR()	if (_disabled) Wow64RevertWow64FsRedirection(_old_state)
#else
#define DISABLE_FS_REDIR()
#define REVERT_FS_REDIR()
#endif
