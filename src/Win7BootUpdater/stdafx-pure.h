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

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Xml.dll>

#ifdef INTEGRATED
#define PUBLIC
#else
#define PUBLIC public
#endif

// C-Sharp shortcuts, so when I transition it is easier
#define _CS_TYPES_DEFINED_
typedef System::String ^string;
typedef System::Object ^object;

typedef System::Byte byte;
typedef System::UInt16 ushort;
typedef System::UInt32 uint;
typedef System::UInt64 ulong;

typedef System::SByte sbyte;
//typedef System::Int16 short; // same
//typedef System::Int32 int;   // same
typedef System::Int64 slong;   // supposed to be "long" but that conflicts with C++ long

//typedef System::Single float;  // same
//typedef System::Double double; // same
typedef System::Decimal decimal;

//typedef System::Char char; // in C++ this is wchar_t
//typedef System::Boolean bool;  // same
//typedef System::Void void;     // same


//inline string MakePath(... array<string> ^parts) { string path = parts[0]; for (int i = 1; i < parts->Length; ++i) path = System::IO::Path::Combine(path, parts[i]); return path; }
inline string GetFullPath(string path) { return path->StartsWith(L"\\\\?\\") ? path : System::IO::Path::GetFullPath(path); }
