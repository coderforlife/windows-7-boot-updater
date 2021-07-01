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

#include "stdafx.h"

using namespace System;
using namespace System::IO;
using namespace System::IO::Compression;

#define PATCH_MAGIC	0x7C9A

enum class Compressions sealed : unsigned __int16 { None = 0, GZip = 1, Deflate = 2 };

void DecompressPatch(Stream ^s) {
	BinaryReader ^b = gcnew BinaryReader(s);
	if (b->ReadUInt16() != PATCH_MAGIC) { throw gcnew Exception(L"Loading patch failed"); }
	unsigned __int16 format_major = b->ReadUInt16();
	unsigned __int16 format_minor = b->ReadUInt16();
	if (format_major != 0 || format_minor > 1) { throw gcnew Exception(L"The patch version is newer than this program can understand"); }
	unsigned __int16 file_major = b->ReadUInt16();
	unsigned __int16 file_minor = b->ReadUInt16();
	unsigned __int16 compression = b->ReadUInt16();
	switch (compression) {
		case Compressions::None: break;
		case Compressions::GZip:	s = gcnew GZipStream   (s, CompressionMode::Decompress); break;
		case Compressions::Deflate:	s = gcnew DeflateStream(s, CompressionMode::Decompress); break;
		default: throw gcnew Exception(L"Unknown compression format");
	}
	array<Byte> ^bytes = gcnew array<Byte>(1024);
	int count;
	Stream ^out = File::Open(L"patch-uncompressed", FileMode::Create, FileAccess::Write);
	BinaryWriter ^w = gcnew BinaryWriter(out);
	w->Write((unsigned __int16)PATCH_MAGIC);
	w->Write(format_major);
	w->Write(format_minor);
	w->Write(file_major);
	w->Write(file_minor);
	w->Write((unsigned __int16)Compressions::None);
	while ((count = s->Read(bytes, 0, 1024)) > 0) {
		out->Write(bytes, 0, count);
	}
}

int main(array<String^> ^args)
{
	DecompressPatch(File::Open(L"patch", FileMode::Open, FileAccess::Read));
    return 0;
}
