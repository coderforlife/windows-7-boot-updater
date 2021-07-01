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

// These functions are the remainder of the functions that have not been converted to C++/CLI

#pragma unmanaged

/////////////////// Utilities /////////////////////////////////////////////////

static LPBYTE find(LPBYTE data, size_t data_len, const unsigned char *target, size_t target_len, size_t *out_len) {
	LPBYTE d = data, orig = data;
	size_t d_len = data_len;
	do {
		// Find the first byte of target in data
		d = (unsigned char*)memchr(data, (int)target[0], d_len);

		// Could not find start
		if (d == NULL) { d_len = 0; break; }

		// Move the next search position one ahead so that we skip the current byte
		d_len = data_len-(d-orig);
		data = d+1;

		// Compare the current data to target
	} while (memcmp(d, target, min(target_len, d_len)) != 0);
	if (out_len)
		*out_len = d_len;
	return d;
}
BOOL Read(HANDLE hFile, LPVOID lpBuffer, DWORD dwSize) {
	DWORD dwRead;
	return ReadFile(hFile, lpBuffer, dwSize, &dwRead, NULL) ? TRUE : (dwRead == dwSize);
}
LPBYTE ReadAll(LPCWSTR lpFileName, size_t *len) {
	HANDLE h;
	LPBYTE b;
	DWORD size;

#ifndef _WIN64
	PVOID old_state;
	BOOL disabled = Wow64DisableWow64FsRedirection(&old_state);
#endif

	h = CreateFile(lpFileName, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) return NULL;

	size = GetFileSize(h, NULL); // assume not super-huge
	if (size != INVALID_FILE_SIZE) {
		b = (LPBYTE)malloc(size);
		if (!Read(h, b, size)) { free(b); b = NULL; }
		else if (len) { *len = size; }
	}
	CloseHandle(h);

#ifndef _WIN64
	if (disabled) Wow64RevertWow64FsRedirection(old_state);
#endif

	return b;
}


/////////////////// General Hacking Routines //////////////////////////////////

static BYTE program_id[] = {0x4D, 0x5A, 0x90, 0x00}; // MZ 0x0090

static BOOL GetFileVersion(LPVOID ver, ULONGLONG *fileVersion, ULONGLONG *prodVersion) {
	VS_FIXEDFILEINFO *v = NULL;
	UINT count;
	if (VerQueryValueW(ver, L"\\", (LPVOID*)&v, &count)) {
		*fileVersion = (((ULONGLONG)v->dwFileVersionMS << 32) | v->dwFileVersionLS);
		*prodVersion = (((ULONGLONG)v->dwProductVersionMS << 32) | v->dwProductVersionLS);
		return TRUE;
	}
	return FALSE;
}

static IMAGE_RESOURCE_DIRECTORY_ENTRY *GetEntries(LPBYTE data, size_t size, size_t offset, DWORD *nEntries) {
	IMAGE_RESOURCE_DIRECTORY *dir = (IMAGE_RESOURCE_DIRECTORY*)(data+offset);
	offset += sizeof(IMAGE_RESOURCE_DIRECTORY);
	if (offset > size) { return NULL; }
	*nEntries = dir->NumberOfIdEntries+dir->NumberOfNamedEntries;
	if (offset + *nEntries*sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) > size) { return NULL; }
	return (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(data+offset);
}

static IMAGE_RESOURCE_DIRECTORY_ENTRY *GetFirstEntry(LPBYTE data, size_t size, size_t offset) {
	IMAGE_RESOURCE_DIRECTORY *dir = (IMAGE_RESOURCE_DIRECTORY*)(data+offset);
	DWORD nEntries;
	offset += sizeof(IMAGE_RESOURCE_DIRECTORY);
	if (offset > size) { return NULL; }
	nEntries = dir->NumberOfIdEntries+dir->NumberOfNamedEntries;
	if (nEntries < 1 || (offset + nEntries*sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY)) > size) { return NULL; }
	return (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(data+offset);
}

static LPBYTE GetVersionResource(LPBYTE d, size_t len, size_t *out_len) {
	DWORD start, nEntries, i;
	IMAGE_SECTION_HEADER *section;
	IMAGE_RESOURCE_DIRECTORY_ENTRY *entries, *name, *lang;
	IMAGE_RESOURCE_DATA_ENTRY *entry;

	if ((section = (IMAGE_SECTION_HEADER*)find(d, len, (unsigned char*)".rsrc\0\0\0", 8, NULL)) == NULL) return NULL;
	start = section->PointerToRawData;

	if ((entries = GetEntries(d, len, start, &nEntries)) == NULL) return NULL;
	for (i = 0; i < nEntries; i++) {
		// Check to see if its the version resources
		if (!entries[i].NameIsString && MAKEINTRESOURCE(entries[i].Id) == RT_VERSION) {
			// Take the first name / lang entries
			if ((name = GetFirstEntry(d, len, start+entries[i].OffsetToDirectory)) == NULL) { break; }
			if ((lang = GetFirstEntry(d, len, start+name->OffsetToDirectory)) == NULL) { break; }
			
			// Get the data entry
			if (start+lang->OffsetToData+sizeof(IMAGE_RESOURCE_DATA_ENTRY) > len) { break; }
			entry = (IMAGE_RESOURCE_DATA_ENTRY*)(d+start+lang->OffsetToData);

			// Get the data
			if (start+entry->OffsetToData-section->VirtualAddress+entry->Size > len) { break; }
			if (out_len)
				*out_len = entry->Size;
			return d+start+entry->OffsetToData-section->VirtualAddress;
		}
	}
	return NULL;
}

static WORD GetPlatform(LPBYTE d) {
	IMAGE_DOS_HEADER *dosh = (IMAGE_DOS_HEADER*)d;
	if (dosh->e_magic != IMAGE_DOS_SIGNATURE)						{ SetLastError(ERROR_INVALID_DATA); return 0; }
	return ((IMAGE_FILE_HEADER*)(d+dosh->e_lfanew+sizeof(DWORD)))->Machine;
}

static BOOL GetInfo(LPBYTE data, size_t len, ULONGLONG *fileVersion, ULONGLONG *prodVersion, WORD *platform) {
	BOOL success = FALSE;
	*platform = GetPlatform(data);
	if (*platform != 0) {
		LPBYTE ver = GetVersionResource(data, len, NULL);
		if (ver)
			success = GetFileVersion(ver, fileVersion, prodVersion);
	}
	return success;
}

static BOOL GetBootmgrVersion(LPCWSTR path, ULONGLONG *fileVersion, ULONGLONG *prodVersion, WORD *platform) {
	BOOL success = FALSE;
	size_t len, p1_len, p2_len, d_len;
	LPBYTE data = ReadAll(path, &len), program1, program2, decomp;
	if (data) {
		program1 = find(data, len, program_id, ARRAYSIZE(program_id), &p1_len);
		if (program1) {
			program2 = find(program1+1, p1_len-1, program_id, ARRAYSIZE(program_id), &p2_len);
			if (program2) {
				program2 -= 3;
				p2_len += 3;
				decomp = (LPBYTE)malloc(p2_len*2); // assume double is as big as uncompressed data is
				if ((d_len = Decompress(program2, p2_len, decomp, p2_len*2)) > 0) {
					success = GetInfo(decomp, d_len, fileVersion, prodVersion, platform);
				}
				free(decomp);
			}
		}
		free(data);
	}
	return success;
}

static BOOL GetWinloadVersion(LPCWSTR path, ULONGLONG *fileVersion, ULONGLONG *prodVersion, WORD *platform) {
	BOOL success = FALSE;
	size_t len;
	LPBYTE data = ReadAll(path, &len);
	if (data) {
		success = GetInfo(data, len, fileVersion, prodVersion, platform);
		free(data);
	}
	return success;
}

typedef BOOL (*GetVer) (LPCWSTR path, ULONGLONG *fileVersion, ULONGLONG *prodVersion, WORD *platform);

#pragma managed
