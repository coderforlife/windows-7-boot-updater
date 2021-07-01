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

#include "Bootmgr.h"

#include "Bcd.h"
#include "ErrorCodes.h"
#include "Resources.h"
#include "UI.h"

#include "Patch.h"

#include "bmzip.h"
#include "Bytes.h"
#include "Files.h"
#include "PEFile.h"
#include "PEFiles.h"

using namespace Win7BootUpdater;
using namespace Win7BootUpdater::Files;
using namespace Win7BootUpdater::Patches;
using namespace Win7BootUpdater::PEFiles;

using namespace System;
using namespace System::IO;

/////////////////// Path Functions ////////////////////////////////////////////

string Bootmgr::def::get() {
	if (!_def) {
		try {
			_def = BCD::GetFilePath(BCD::Bootmgr, L"bootmgr");
		} catch (Exception ^ex) {
			UI::ShowError(ex->Message, UI::GetMessage(Msg::ErrorWhileGettingPath, L"bootmgr"));
			_def = defFallBack;
		}
	}
	return _def;
}
bool Bootmgr::IsOnHiddenSystemPartition(string path) { return path->StartsWith(L"\\\\?\\Volume{"); }
bool Bootmgr::DefaultIsOnHiddenSystemPartition() { return IsOnHiddenSystemPartition(def); }


/////////////////// General Hacking Routines //////////////////////////////////

const static unsigned char program_id[4] = {0x4D, 0x5A, 0x90, 0x00}; // MZ 0x0090

// 3 increments
static PEFile *load(string path, uint *err, ushort *lang, Bytes &data, Bytes &program1, Bytes &program2, Bytes &decomp, bool readonly) {
	*err = ERROR_BOOTMGR_LOAD;

	data = ReadAll(as_native(path));
	if (data == NULL) { return NULL; }

	program1 = data.find(program_id, ARRAYSIZE(program_id));
	if (program1 == NULL) { free(data); data = NULL; return NULL; }

	program2 = (program1+1).find(program_id, ARRAYSIZE(program_id))-3; // need to back up 3 bytes for compression header
	if (program2 == NULL) { free(data); data = NULL; return NULL; }

	data.setCount(program2-data); // set the size of the stub to be up to program 2

	decomp = bmzip_decompress(program2);
	if (decomp == NULL) { free(data); data = NULL; return NULL; }

	UI::Inc();

	// 2 increments
	PEFile *f = LoadAndVerify(decomp, err, lang, RT_MESSAGETABLE, L"BOOTMGR.XSL", L"bootmgr.exe", readonly);
	if (f == NULL) { free(data); data = NULL; return NULL; }

	*err = ERROR_SUCCESS;
	return f;
}

uint Bootmgr::Check(string path) {
	path = GetFullPath(path);
	ushort lang = 0;
	uint error = ERROR_SUCCESS;
	Bytes data, p1, p2, decomp;
	PEFile *bootmgr = load(path, &error, &lang, data, p1, p2, decomp, true);
	if (data) { free(data); }
	if (bootmgr) {
		delete bootmgr;
		return error;
	}
	return error + ERROR_BOOTMGR_BASE;
}

uint Bootmgr::Update(FileUpdater fu) {
	ushort lang = 0;
	uint error = ERROR_SUCCESS;
	Bytes data, prog1, prog2, decomp;

	// 3 increments
	PEFile *bm = load(fu.path, &error, &lang, data, prog1, prog2, decomp, false);
	if (bm && bm->isAlreadyModified()) {
		delete bm;
		bm = NULL;
		UI::Inc(3);
	}
	if (!bm) {
		if (data)	free(data);
		return error;
	}

	Bytes comp;
	HANDLE f = INVALID_HANDLE_VALUE;

	PatchFile ^patch = Res::GetPatch(L"bootmgr");

	// 2 increments
	if (!patch->Apply(bm) || !UI::Inc())											{ error = ERROR_BOOTMGR_HACK; }
	else if ((error = Save(bm, ERROR_BOOTMGR_BASE)) != ERROR_SUCCESS)				{ /* error = error; */ }
	else if (!decomp.setCount(bm->getSize()) || !(comp = bmzip_compress(decomp)))	{ error = ERROR_BOOTMGR_COMPRESS; }

	// 1 increment
	else if ((f = CreateFile(as_native(fu.path), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM, NULL)) == INVALID_HANDLE_VALUE)	{ error = ERROR_BOOTMGR_SAVE; }
	else if (!Write(f, data, (uint)*data) || !Write(f, comp, (uint)*comp))			{ error = ERROR_BOOTMGR_SAVE; }
	UI::Inc();

	if (f != INVALID_HANDLE_VALUE) CloseHandle(f);
	if (bm)		delete bm;
	if (comp)	free(comp);
	if (data)	free(data);

	return error;
}
