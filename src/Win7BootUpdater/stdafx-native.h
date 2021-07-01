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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

// We don't want TCHARs, we are going to only use WCHAR
#undef TCHAR
#undef TEXT
#undef _T

// Overwrite Flags
#define OVERWRITE_ALWAYS	0 //always adds the resource, even if it already exists
#define OVERWRITE_NEVER		1 //only adds a resource is it does not already exist
#define OVERWRITE_ONLY		2 //only adds a resource if it will overwrite another resource

// C-Sharp shortcuts, so when I transition it is easier
#ifndef _CS_TYPES_DEFINED_
#define _CS_TYPES_DEFINED_
//typedef System::String ^string; // unavailable in native
//typedef System::Object ^object; // unavailable in native

typedef unsigned char byte;
typedef unsigned __int16 ushort;
typedef unsigned __int32 uint;
typedef unsigned __int64 ulong;

typedef char sbyte;
//typedef __int16 short; // same
//typedef __int32 int;   // same
typedef __int64 slong; // supposed to be "long" but that conflicts with C++ long

//typedef _ float;   // same
//typedef _ double;  // same
//typedef System::Decimal decimal; // unavailable in native

//typedef wchar_t char; // in C++ this is wchar_t
//typedef _ bool;  // same
//typedef _ void;  // same
#endif

// Extra types, used by the decompression algorithms LZX and XPRESS
//typedef unsigned __int8 byte;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

#define thread_local __declspec(thread)

#ifndef _WIN64
#define DISABLE_FS_REDIR()	PVOID _old_state; BOOL _disabled = Wow64DisableWow64FsRedirection(&_old_state)
#define REVERT_FS_REDIR()	if (_disabled) Wow64RevertWow64FsRedirection(_old_state)
#else
#define DISABLE_FS_REDIR()
#define REVERT_FS_REDIR()
#endif
