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

#include "PDB.h"

#include "PEFile.h"
#include "Bytes.h"
#include "Utilities.h"

using namespace Win7BootUpdater;

using namespace System;
using namespace System::IO;
using namespace System::Net;
using namespace System::Text;
using namespace System::Runtime::InteropServices;

typedef byte* bytes;

static string Utf8ToString(const char* str) {
	return Encoding::UTF8->GetString(Utilities::GetManagedArray((const byte*)str, strlen(str)));
}

static bytes read_stream(const bytes data, const DWORD* blocks, DWORD nbytes,
						 DWORD num_blocks, DWORD block_size) {
	bytes out = (bytes)malloc(nbytes);
	if (!out) { return NULL; }
	DWORD n = (nbytes+block_size-1)/block_size;
	for (DWORD i = 0; i < n; i++) {
		if (blocks[i] > num_blocks) { return NULL; }
		memcpy(out+i*block_size, data+blocks[i]*block_size, (i == n-1) ? (nbytes % block_size) : block_size);
	}
	return out;
}

MSF::MSF(const void* _data, size_t len) : num(0), sizes(NULL), streams(NULL) {
	if (len < sizeof(SuperBlock)) { return; }
	const bytes data = (const bytes)_data;
	SuperBlock header = *(const SuperBlock*)data;
	DWORD bs = header.BlockSize;
	if (memcmp(header.FileMagic, MSF_Magic, sizeof(MSF_Magic)) != 0 ||
		header.NumBlocks*bs != len ||
		header.BlockMapAddr >= header.NumBlocks) { return; }
		
	// Read the stream directory
	DWORD num_entries = (header.NumDirectoryBytes+bs-1)/bs;
	if (num_entries*sizeof(DWORD) > bs) { return; }
	DWORD* dir = (DWORD*)read_stream(data, (const DWORD*)(data+header.BlockMapAddr*bs),
									 header.NumDirectoryBytes, header.NumBlocks, bs);
	if (!dir) { return; }
	
	// Parse the stream directory and read the streams
	num = dir[0];
	DWORD off = num+1, size = header.NumDirectoryBytes / sizeof(DWORD);
	if (off > size) { free(dir); dealloc(); return; }
	sizes = (DWORD*)malloc(num*sizeof(DWORD));
	streams = (void**)malloc(num*sizeof(void*));
	if (!sizes || !streams) { free(dir); dealloc(); return; }

	memcpy(sizes, dir+1, num*sizeof(DWORD));
	memset(streams, 0, num*sizeof(void*));
	for (DWORD i = 0; i < num; i++) {
		if (sizes[i] == 0) { continue; }
		streams[i] = read_stream(data, dir+off, sizes[i], header.NumBlocks, bs);
		off += (sizes[i]+bs-1)/bs;
		if (!streams[i] || off > size) { free(dir); dealloc(); return; }
	}
	free(dir);
}

void MSF::dealloc() {
	num = 0;
	free(sizes);
	sizes = NULL;
	if (streams) {
		for (DWORD i = 0; i < num; i++) { free(streams[i]); streams[i] = NULL; }
		free(streams);
		streams = NULL;
	}
}

PDB::PDB(const void* data, size_t len) : MSF(data, len), header(NULL), dbi(NULL), sects(NULL) {
	if (num < 4) { dealloc(); return; }
	
	DWORD sz;
	void* s = getStream(1, &sz);
	if (sz < sizeof(Header)) { dealloc(); return; }
	header = (Header*)s;
	if (header->Version != Version::VC70) { dealloc(); return; }

	s = getStream(3, &sz);
	if (sz < sizeof(DBIHeader)) { dealloc(); return; }
	dbi = (DBIHeader*)s;
	if (dbi->VersionHeader != DBIVersion::V70) { dealloc(); return; }
	if (dbi->PublicStreamIndex >= num || dbi->GlobalStreamIndex >= num || dbi->SymbolRecordStreamIndex >= num) { dealloc(); return; }
	DWORD dbi_len = sizeof(DBIHeader) + dbi->ModInfoSize + dbi->SectionContributionSize +
		dbi->SectionMapSize + dbi->SourceInfoSize + dbi->TypeServerSize + dbi->MFCTypeServerSize +
		dbi->ECSubstreamSize + dbi->DebugHeaderSize;
	if (dbi_len != sz) { dealloc(); return; }
	
	if (SECTION_HEADERS*sizeof(WORD) < dbi->DebugHeaderSize) {
		WORD strm = ((WORD*)((bytes)s + sz - dbi->DebugHeaderSize))[SECTION_HEADERS];
		if (strm < num) { sects = (IMAGE_SECTION_HEADER*)getStream(strm); }
	}
}

uint PDB::getFunctionVA(const char* name) const {
	// Leading _ and trailing @ are optional
	DWORD len = strlen(name);
	DWORD sz;
	const Bytes data((byte*)getStream(dbi->SymbolRecordStreamIndex, &sz), sz);
	const Bytes found = data.find((const bytes)name, len);
	if (found == NULL) { return 0; }
	uint name_off = (uint)(found - data);
	if (name_off+len >= sz) { return 0; }
	if (data[name_off+len] == '@') {
		len++;
		while (name_off+len < sz && isdigit(data[name_off+len])) { len++; }
	}
	if (name_off+len >= sz || data[name_off+len]) { return 0; } 
	while (name_off > 0 && data[name_off-1] == '_') { name_off--; len++; }
	if (name_off < 14) { return 0; }
	
	// Name goes from name_off to name_off+len (which is the null terminator)
	// Get the rest of the symbol
	FunctionSymbol* sym = (FunctionSymbol*)data(name_off-14);
	DWORD sym_len = 15 + len;
	if (sym_len % sizeof(DWORD)) { sym_len += sizeof(DWORD) - sym_len % sizeof(DWORD); }
	if (sym_len != sym->length+2u || sym->type != 0x110Eu) { return 0; }
	return sym->rva + ((sects == NULL || sym->sect_id == 0) ? 0 : sects[sym->sect_id-1].VirtualAddress);
}

const PDB::CODEVIEW_DEBUG_DIRECTORY_ENTRY* PDB::GetCVDebugDirectoryEntry(const PEFile* pe) {
	// Get the IMAGE_DEBUG_DIRECTORY which the refers to a CodeView Debug Diretory Entry
	if (IMAGE_DIRECTORY_ENTRY_DEBUG >= pe->getDataDirectoryCount()) { return NULL; }
	const IMAGE_DATA_DIRECTORY* dir = pe->getDataDirectory(IMAGE_DIRECTORY_ENTRY_DEBUG);
	if (dir->Size != sizeof(IMAGE_DEBUG_DIRECTORY)) { return NULL; }
	int i;
	const IMAGE_SECTION_HEADER* sect = pe->getSectionHeaderByRVA(dir->VirtualAddress, &i);
	if (sect == NULL) { return NULL; } // TODO: dir->VirtualAddress + dir->Size should be in sect as well
	DWORD data_off = dir->VirtualAddress - sect->VirtualAddress + sect->PointerToRawData;
	const IMAGE_DEBUG_DIRECTORY* dd = (IMAGE_DEBUG_DIRECTORY*)pe->get(data_off);
	if (dd->Type != IMAGE_DEBUG_TYPE_CODEVIEW || dd->SizeOfData < sizeof(CODEVIEW_DEBUG_DIRECTORY_ENTRY)) { return NULL; }

	// Get the CodeView Debug Directory Entry
	DWORD avail, loc = (dd->AddressOfRawData - sect->VirtualAddress) + sect->PointerToRawData; // can't use dd->PointerToRawData since it may have moved with other edits and not been updated, however this should work
	const CODEVIEW_DEBUG_DIRECTORY_ENTRY* cvdde = (CODEVIEW_DEBUG_DIRECTORY_ENTRY*)pe->get(loc, &avail);
	if (avail < dd->SizeOfData || cvdde->Signature != SIG_RSDS) { return NULL; }
	if (((const	byte*)cvdde)[dd->SizeOfData] != 0) { return NULL; }
	return cvdde;
}

static string FormatGUID(GUID guid) {
	return String::Format("{0:X8}{1:X4}{2:X4}{3:X2}{4:X2}{5:X2}{6:X2}{7:X2}{8:X2}{9:X2}{10:X2}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

static array<Byte>^ RawDownload(string path, string guid, DWORD age) {
	// Download the PDB file (this emaulates the Microsoft Symbol Server library)
	string url = String::Format("{0}/{1}/{2}{3:X}/{1}",
		L"http://msdl.microsoft.com/download/symbols", path, guid, age);
#ifdef _DEBUG
	Console::WriteLine(L"Downloading " + url);
#endif
	HttpWebRequest^ r = (HttpWebRequest^)WebRequest::Create(url);
	r->Method = WebRequestMethods::Http::Get;
	r->KeepAlive = true;
	r->UserAgent = L"Microsoft-Symbol-Server/6.3.9600.17095";
	r->AutomaticDecompression = DecompressionMethods::GZip;
	Stream^ stream = nullptr;
	MemoryStream^ memory;
	try {
		stream = r->GetResponse()->GetResponseStream();
		memory = gcnew MemoryStream();
		array<byte>^ buf = gcnew array<byte>(81920);
		int n;
		while ((n = stream->Read(buf, 0, 81920)) != 0) { memory->Write(buf, 0, n); }
#ifdef _DEBUG
	} catch (Exception^ exc) {
		Console::WriteLine("Exception while downloading:");
		Console::WriteLine(exc);
#else
	} catch (Exception^) {
#endif
		if (stream) { stream->Close(); }
		return nullptr;
	}
	return memory->ToArray();
}

static PDB* LoadPDB(array<Byte>^ data, GUID guid, DWORD age) {
	pin_ptr<Byte> ptr = &data[0];
	PDB* pdb = new PDB((bytes)ptr, data->Length);
	if (!pdb->isLoaded() || !pdb->matches(guid, age)) {
		delete pdb;
		return NULL;
	}
	return pdb;
}

array<Byte>^ PDB::Download(const PEFile* pe) {
	const CODEVIEW_DEBUG_DIRECTORY_ENTRY* cvdde = GetCVDebugDirectoryEntry(pe);
	return cvdde ? RawDownload(Utf8ToString(cvdde->Path), FormatGUID(cvdde->Guid), cvdde->Age) : nullptr;
}

string PDB::DownloadToDefault(const PEFile* pe) {
	const CODEVIEW_DEBUG_DIRECTORY_ENTRY* cvdde = GetCVDebugDirectoryEntry(pe);
	if (!cvdde) { return nullptr; }
	string path = Utf8ToString(cvdde->Path);
	if (path->Length == 0) { return nullptr; }
	string path_no_pdb = path->EndsWith(".pdb", StringComparison::InvariantCultureIgnoreCase) ? path->Remove(path->Length-4) : path;
	string guid = FormatGUID(cvdde->Guid);
	array<Byte>^ data = RawDownload(path, guid, cvdde->Age);
	if (data == nullptr) { return nullptr; }
	
	// Write the file
	string out = String::Format("{0}-{1}{2:X}.pdb", path_no_pdb, guid, cvdde->Age);
	try {
		File::WriteAllBytes(out, data);
	} catch (Exception^) { return nullptr; }
	return out;
}

PDB* PDB::Get(const PEFile* pe) {
	// Read the CodeView Debug Directory Entry
	const CODEVIEW_DEBUG_DIRECTORY_ENTRY* cvdde = GetCVDebugDirectoryEntry(pe);
	if (!cvdde) { return NULL; }
	string path = Utf8ToString(cvdde->Path);
	if (path->Length == 0) { return NULL; }
	string path_no_pdb = path->EndsWith(".pdb", StringComparison::InvariantCultureIgnoreCase) ? path->Remove(path->Length-4) : path;
	string guid = FormatGUID(cvdde->Guid);
	
	// See if the file is available locally
	// Several options, check for each either in the CWD or a relative to the apps directory
	array<string>^ files = gcnew array<string> {
		String::Format("{0}", path),
		String::Format("{0}-{1}{2:X}.pdb", path_no_pdb, guid, cvdde->Age),
		String::Format("{0}\\{1}{2:X}\\{0}", path, guid, cvdde->Age),
	};
	array<string>^ dirs = gcnew array<string> {
		Directory::GetCurrentDirectory(),
		Path::GetDirectoryName(System::Reflection::Assembly::GetExecutingAssembly()->Location),
	};
	for (int i = 0; i < files->Length; i++) {
		for (int j = 0; j < dirs->Length; j++) {
			string p = Path::Combine(dirs[j], files[i]);
			if (File::Exists(p)) {
				PDB* pdb = LoadPDB(File::ReadAllBytes(p), cvdde->Guid, cvdde->Age);
				if (pdb) { return pdb; }
			}
		}
	}

	// Download the PDB file
	array<Byte>^ data = RawDownload(path, guid, cvdde->Age);
	return data ? LoadPDB(data, cvdde->Guid, cvdde->Age) : NULL;
}
