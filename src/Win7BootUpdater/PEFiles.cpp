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

#include "PEFiles.h"

#include "ErrorCodes.h"
#include "UI.h"

using namespace Win7BootUpdater;

using namespace System;

#pragma unmanaged

typedef struct _LANGANDCODEPAGE {
  ushort wLanguage;
  ushort wCodePage;
} LANGANDCODEPAGE;

static bool verStrMatches(LPVOID ver, LPCWSTR key, LPCWSTR value) {
	LANGANDCODEPAGE *trans;
	WCHAR subBlock[96], *str;
	UINT transCount = 0, count = 0;
	unsigned int i;
	if (!PEFile::GetVersionInfo(ver, L"\\VarFileInfo\\Translation", (LPVOID*)&trans, &transCount)) {
		return false;
	}
	transCount /= sizeof(LANGANDCODEPAGE);
	for (i = 0; i < transCount; i++) {
		_snwprintf(subBlock, 96, L"\\StringFileInfo\\%04x%04x\\%s", trans[i].wLanguage, trans[i].wCodePage, key);
		if (PEFile::GetVersionInfo(ver, subBlock, (LPVOID*)&str, &count) && wcscmp(str, value) == 0) {
			return true;
		}
	}
	return false;
}

static uint CheckFile(PEFile *f, ushort *lang, LPWSTR typeTest, LPWSTR htmlName, LPCWSTR internalName) {
	if (f->isLoaded()) {
		size_t size = 0; // second RESUME.XSL
		LPVOID ver;
		if (f->resourceExists(typeTest, MAKEINTRESOURCE(1), lang) &&
			(htmlName == NULL || f->resourceExists(RT_HTML, htmlName, lang)) &&
			(ver = f->getResource(RT_VERSION, MAKEINTRESOURCE(1), *lang, &size)) != NULL) {
			bool matches = verStrMatches(ver, L"InternalName", internalName);
			free(ver);
			return matches ? ERROR_SUCCESS : GEN_ERR_PEFILE_INVALID_VER;
		} else {
			return GEN_ERR_PEFILE_INVALID_RES; 
		}
	} else {
		return GEN_ERR_PEFILE_LOAD;
	}
}

#pragma managed

static PEFile *CheckFile(PEFile *f, uint *err, ushort *lang, LPWSTR typeTest, LPWSTR htmlName, LPCWSTR internalName) {
	UI::Inc();
	*err = CheckFile(f, lang, typeTest, htmlName, internalName);
	if (*err != ERROR_SUCCESS) {
		delete f;
		return NULL;
	}
	UI::Inc();
	return f;
}

// 2 increments
PEFile *PEFiles::LoadAndVerify(LPCWSTR path, uint *err, ushort *lang, LPWSTR typeTest, LPWSTR htmlName, LPCWSTR internalName, bool readonly) {
	DISABLE_FS_REDIR();
	PEFile *f = new PEFile(path, readonly);
	REVERT_FS_REDIR();
	return CheckFile(f, err, lang, typeTest, htmlName, internalName);
}

// 2 increments
PEFile *PEFiles::LoadAndVerify(Bytes data, uint *err, ushort *lang, LPWSTR typeTest, LPWSTR htmlName, LPCWSTR internalName, bool readonly) {
	return CheckFile(new PEFile(data, *data, readonly), err, lang, typeTest, htmlName, internalName);
}

// 1 increment
uint PEFiles::Save(PEFile *f, uint errBase) {
	if (!f->clearCertificateTable())				{ return GEN_ERR_PEFILE_CLEAR_CERT+errBase; }
	else if (!f->setModifiedFlag() || !f->save())	{ return GEN_ERR_PEFILE_SAVE+errBase; }
	UI::Inc();
	return ERROR_SUCCESS;
}

// 1 increment
uint PEFiles::ModifyRes(PEFile *f, LPWSTR type, LPWSTR name, ushort lang, Bytes data, uint errBase, bool add) {
	//UI::ProgressText = UI::GetMessage(Msg::ModifyingResources);
	if (!f->addResource(type, name, lang, data, *data, add ? OVERWRITE_ALWAYS : OVERWRITE_ONLY))	{ return GEN_ERR_PEFILE_MOD_RES+errBase; } // or when add, OVERWRITE_NEVER
	UI::Inc();
	return ERROR_SUCCESS;
}

// 2 increments
uint PEFiles::ModifyResAndSave(PEFile *f, LPWSTR type, LPWSTR name, ushort lang, Bytes data, uint errBase, bool add) {
	uint error = ModifyRes(f, type, name, lang, data, errBase, add);
	return error == ERROR_SUCCESS ? Save(f, errBase) : error;
}
