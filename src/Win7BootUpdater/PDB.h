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

#include "PEFile.h"

namespace Win7BootUpdater {
	const byte MSF_Magic[32] = "Microsoft C/C++ MSF 7.00\x0D\x0A\x1A\x44\x53\x00\x00"; // one more null character automatically included

	class MSF {
	protected:
		struct SuperBlock {
			byte FileMagic[sizeof(MSF_Magic)];
			DWORD BlockSize;
			DWORD FreeBlockMapBlock;
			DWORD NumBlocks;
			DWORD NumDirectoryBytes;
			DWORD Unknown;
			DWORD BlockMapAddr;
		};
		DWORD num;
		DWORD* sizes;
		void** streams;
		void dealloc();
	public:
		MSF(const void* _data, size_t len);
		inline ~MSF() { dealloc(); }
		inline bool isLoaded() const { return num != 0; }
		inline DWORD streamCount() const { return num; }
		inline void* getStream(DWORD i, DWORD* sz=NULL) { if (sz) { *sz = sizes[i]; } return streams[i]; }
		inline const void* getStream(DWORD i, DWORD* sz=NULL) const { if (sz) { *sz = sizes[i]; } return streams[i]; }
	};
	class PDB : public MSF {
	public:
		enum class Version : DWORD {
			VC2 = 19941610,
			VC4 = 19950623,
			VC41 = 19950814,
			VC50 = 19960307,
			VC98 = 19970604,
			VC70Dep = 19990604,
			VC70 = 20000404,
			VC80 = 20030901,
			VC110 = 20091201,
			VC140 = 20140508,
		};
		struct Header {
			Version Version; // always V70
			DWORD Signature; // timestamp from time() [1-second resolution]
			DWORD Age;
			GUID Guid;
		};
		enum class DBIVersion : DWORD {
			VC41 = 930803,
			V50 = 19960307,
			V60 = 19970606,
			V70 = 19990903,
			V110 = 20091201
		};
		struct DBIFlags {
			WORD WasIncrementallyLinked : 1;
			WORD ArePrivateSymbolsStripped : 1;
			WORD HasConflictingTypes : 1;
			WORD Reserved : 13;
		};
		enum class Machine : WORD {
			x86 = 0x014C,
			x64 = 0x8664,
		};
		struct DBIHeader {
			DWORD VersionSignature; // always -1
			DBIVersion VersionHeader; // always V70
			DWORD Age; // same as Header.Age
			WORD GlobalStreamIndex;
			WORD BuildNumber; // toolchain version
			WORD PublicStreamIndex;
			WORD PdbDllVersion; // mspdb version
			WORD SymbolRecordStreamIndex;
			WORD PdbDllRbld; // unknown
			DWORD ModInfoSize;
			DWORD SectionContributionSize;
			DWORD SectionMapSize;
			DWORD SourceInfoSize;
			DWORD TypeServerSize;
			DWORD MFCTypeServerSize;
			DWORD DebugHeaderSize;
			DWORD ECSubstreamSize;
			DBIFlags Flags;
			Machine Machine;
			DWORD _Padding;
		};
		struct Symbol {
			WORD length;
			WORD type;
			byte* data;
		};
		struct FunctionSymbol {
			// Not sure about this format actually
			WORD length;
			WORD type; // always 0x110E
			DWORD unknown; // always 0x02
			DWORD rva;
			WORD sect_id;
			char name[1]; // null terminated
		};
		enum DEBUG_HEADER_INDEX {
			FPO = 0,
			EXC = 1,
			FIXUP = 2,
			OMAP_TO_SRC = 3,
			OMAP_FROM_SRC = 4,
			SECTION_HEADERS = 5,
			TOKEN_RID_MAP = 6,
			XDATA = 7,
			PDATA = 8,
			FPO_NEW = 9,
			ORIG_SECTION_HEADERS = 10,
		};
		// See https://github.com/dotnet/corefx/blob/master/src/System.Reflection.Metadata/specs/PE-COFF.md#codeview-debug-directory-entry-type-2
		typedef struct _CODEVIEW_DEBUG_DIRECTORY_ENTRY {
			DWORD Signature;
			GUID Guid;
			DWORD Age;
			char Path[1]; // null-terminated UTF-8 string
		} CODEVIEW_DEBUG_DIRECTORY_ENTRY;
		static const DWORD SIG_RSDS = 0x53445352;
	private:
		Header* header;
		//void* TPI;
		DBIHeader* dbi;
		IMAGE_SECTION_HEADER* sects;
	public:
		static const CODEVIEW_DEBUG_DIRECTORY_ENTRY* PDB::GetCVDebugDirectoryEntry(const PEFile* pe);
		static PDB* Get(const PEFile* pe);
		static array<byte>^ Download(const PEFile* pe);
		static string DownloadToDefault(const PEFile* pe);
		
		PDB(const void* data, size_t len);
		inline Header* getPDBHeader() { return header; }
		inline const Header* getPDBHeader() const { return header; }
		inline bool matches(GUID guid, DWORD age) const { return memcmp(&header->Guid, &guid, sizeof(GUID)) == 0 && dbi->Age == age; }
		inline bool hasSectionHeaders() const { return sects != NULL; }
		uint getFunctionVA(const char* name) const;
	};
}
