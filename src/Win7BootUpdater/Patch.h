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

#ifndef IMAGE_SIZEOF_SHORT_NAME
#define IMAGE_SIZEOF_SHORT_NAME 8
#endif

#define PATCH_MAGIC	0x7C9A

#ifndef _M_CEE_SAFE
#include "PEFile.h"
#endif

namespace Win7BootUpdater { namespace Patches { 
	enum class Platforms sealed : ushort { I386 = 0x014C, AMD64 = 0x8664 };
	enum class Compressions sealed : ushort { None = 0, GZip = 1, Deflate = 2 };

	///// Known Sections ///////////////////////////
	ref struct Sections abstract sealed {
	public:
		static initonly array<char> ^Blank = gcnew array<char>(IMAGE_SIZEOF_SHORT_NAME){};
		static initonly array<char> ^text = gcnew array<char>(IMAGE_SIZEOF_SHORT_NAME){'.','t','e','x','t'};
		static initonly array<char> ^rdata = gcnew array<char>(IMAGE_SIZEOF_SHORT_NAME){'.','r','d','a','t','a'};
		static initonly array<array<char>^> ^All = gcnew array<array<char>^>{ Blank, text, rdata };
	};
	
	ref class PatchFile;

	///// Basic Patch File Structure ////////////
	ref class Patch abstract {
	public:
		property ushort Type { ushort get(); };
	
		///// Help load patches in classes that are pure
		static PatchFile ^Load(System::IO::Stream ^s);
		static PatchFile ^Load(System::IO::Stream ^s, PatchFile ^min);
		static PatchFile ^Load(System::IO::Stream ^s, ushort min_major, ushort min_minor);
	};

	ref class PatchVersion sealed {
		ulong min, max; // the format from VS_FIXEDFILEINFO: 64-bits, 8 bits for each of: major, minor, ?, revision
		// max can be all 0s which means no max
		Patch ^patch;
	public:
		PatchVersion(System::IO::BinaryReader ^b);
		property ulong Min { ulong get(); }
		property ulong Max { ulong get(); }
		property bool NoMax { bool get(); }
		Patch ^Get();
	};

	ref class PatchPlatform sealed {
		ushort platform; // IMAGE_FILE_MACHINE_I386 or IMAGE_FILE_MACHINE_AMD64
		array<PatchVersion^> ^versions;
	public:
		PatchPlatform(System::IO::BinaryReader ^b);
		property ushort Type { ushort get(); }
		array<PatchVersion^> ^Get(ulong version);
		array<Patch^> ^GetPatches(ulong version);
	};

	ref class PatchEntry sealed {
		ushort id;
		array<PatchPlatform^> ^platforms;
	public:
		PatchEntry(System::IO::BinaryReader ^b);
		property ushort Id { ushort get(); }
		PatchPlatform ^Get(ushort platform);
	};

#ifndef _M_CEE_SAFE

	// We can define PatchFile and Types only when unsafe
	
	ref class PatchFile sealed : public System::IComparable<PatchFile^> {
		ushort format_major, format_minor;
		ushort file_major, file_minor;
		ushort compression;
		array<PatchEntry^> ^entries;
		void PatchFile::Init1(System::IO::Stream ^s);
		void PatchFile::Init2(System::IO::Stream ^s);
	public:
		PatchFile(System::IO::Stream ^s);
		PatchFile(System::IO::Stream ^s, ushort min_major, ushort min_minor);
		array<PatchEntry^> ^Get(ushort id);
		array<Patch^> ^Get(ushort id, ushort platform, ulong version);
		array<Patch^> ^Get(PEFile *f, ushort id);
		virtual int CompareTo(PatchFile ^other);
		property ushort Major { ushort get(); }
		property ushort Minor { ushort get(); }
		property string Version { string get(); }

		// Shortcut Apply functions
		bool Apply(PEFile *f); // all Direct at once
		bool ApplyIgnoringApplied(PEFile *f); // all Direct at once, ignoring ones already applied
		bool Apply(PEFile *f, ushort id); // Direct or AddFunction
		bool Apply(PEFile *f, ushort id, array<uint> ^values); // Dwords or AddFunction
		bool Apply(PEFile *f, ushort id, uint value); // Dwords or AddFunction
		bool Apply(PEFile *f, ushort id, string value); // String

		// Shortcut Revert functions
		bool Revert(PEFile *f, ushort id); // AddFunction

		// Shortcut Retrieve Functions (note: with multiple id mappings the GetValue(s) functions will return the first found)
		bool IsApplied(PEFile *f, ushort id); // Direct, Dwords, String, or AddFunction
		array<uint> ^GetValues(PEFile *f, ushort id); // Dwords or AddFunction
		string GetValue(PEFile *f, ushort id); // String
		bool Get(PEFile *f, ushort id, uint %val);
		bool Get(PEFile *f, ushort id, string %s);
	};

	///// Patch Types //////////////////////////////
	namespace Types {
		ref class Direct sealed : public Patch {
			array<char> ^section;
			byte wildcard;
			array<byte> ^target, ^value;
			array<bool> ^already_changed;
		public:
			static const ushort Type = 0x0001;
			Direct(System::IO::BinaryReader ^b);
			bool Apply(PEFile *f);
			bool IsApplied(PEFile *f);
		};

		ref class Dwords sealed : public Patch {
			array<char> ^section;
			array<ushort> ^pos;
			byte wildcard;
			array<byte> ^target;
			array<bool> ^already_changed;
		public:
			static const ushort Type = 0x0002;
			Dwords(System::IO::BinaryReader ^b);
			ushort Count();
			bool Apply(PEFile *f, ... array<uint> ^values);
			array<uint> ^GetValues(PEFile *f);
		};

		ref class String sealed : public Patch {
			array<char> ^section;
			ushort pos;
			byte wildcard;
			array<byte> ^target;
			bool FindTarget(PEFile *f, array<byte> ^%target, uint *pos, int *data_i, uint *off, string %value);
			bool DoInPlacePatch(PEFile *f, string value, uint off, uint max);
			bool DoMovePatch(PEFile *f, string value, array<byte> ^target, uint pos, int data_i);
		public:
			static const ushort Type = 0x0003;
			String(System::IO::BinaryReader ^b);
			bool Apply(PEFile *f, string value);
			string GetValue(PEFile *f);
		};

		ref class AddFunction sealed : public Patch {
			array<char> ^section;
			ushort callPos;
			byte wildcard, call_wildcard, func_wildcard;
			array<byte> ^target, ^call, ^func, ^w_call, ^w_func;
			array<ushort> ^patchPos, ^funcPos, ^allPos;
            array<array<byte>^> ^funcNames;
			ushort Id();
		public:
			static const ushort Type = 0x0004;
			AddFunction(System::IO::BinaryReader ^b);
			bool Apply(PEFile *f, ... array<uint> ^values);
			bool Revert(PEFile *f);
			array<uint> ^GetValues(PEFile *f);
		};
	}

#endif
} }
