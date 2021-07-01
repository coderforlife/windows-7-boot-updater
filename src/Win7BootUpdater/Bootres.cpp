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

#include "Bootres.h"

#include "Animation.h"
#include "ErrorCodes.h"
#include "UI.h"
#include "Winload.h"

#include "Bytes.h"
#include "Files.h"
#include "PEFile.h"
#include "PEFiles.h"
#include "WIM.h"

#undef GetTempPath

using namespace Win7BootUpdater;
using namespace Win7BootUpdater::Files;
using namespace Win7BootUpdater::PEFiles;

using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::IO;

string Bootres::def::get() {
	if (!_def)
		_def = Path::Combine(Path::GetDirectoryName(Winload::def), Bootres::name);
	return _def;
}

static PEFile *load(string path, uint *err, ushort *lang, bool readonly) {
	return LoadAndVerify(as_native(path), err, lang, RT_RCDATA, NULL, L"bootres", readonly);
}

uint Bootres::Check(string path) {
	path = Path::GetFullPath(path);
	ushort lang = 0;
	uint error = ERROR_SUCCESS;
	PEFile *bootres = load(path, &error, &lang, true);
	if (bootres) delete bootres;
	return (error == ERROR_SUCCESS) ? ERROR_SUCCESS : (error + ERROR_BOOTRES_BASE);
}

static string GetTempDir() {
	Random ^r = gcnew Random();
	string path;
	do {
		path = Path::Combine(Path::GetTempPath(), L"wim"+r->Next());
	} while (File::Exists(path) || Directory::Exists(path));
	try {
		Directory::CreateDirectory(path);
	} catch (Exception ^) { return nullptr; }
	return path;
}

static void DeleteDir(string s) {
	try {
		Directory::Delete(s, true);
	} catch (Exception ^) {}
}

Bitmap ^Bootres::GetAnimation(string path) {
	void *bootres = NULL, *data = NULL;
	try {
		bootres = ReadAll(as_native(path));
		if (bootres == NULL) { return nullptr; }
		size_t size = 0;
		void *wim = PEFile::GetResourceDirect(bootres, RT_RCDATA, MAKEINTRESOURCE(1), &size);
		if (wim == NULL) { return nullptr; }
		data = WIM::ExtractFile(wim, as_native(L"\\"+activity), &size);
		if (data == NULL) { return nullptr; }
		return gcnew Bitmap(gcnew UnmanagedMemoryStream((byte*)data, size));
	}
	finally {
		if (bootres)	free(bootres);
		if (data)		free(data);
	}
	return nullptr;
}

// 4 increments
static uint SaveActivityBMP(Image ^anim, string% path, string% activity, Color bgColor, Image ^bgImg) {
	if ((path = GetTempDir()) == nullptr) { return ERROR_GET_TEMP_DIR; }	UI::Inc();
	activity = Path::Combine(path, Bootres::activity);
	Image ^i = nullptr;
	try {
		i = Animation::ResolveTransparency(anim, Animation::Width, Animation::FullHeight, bgColor, bgImg);
		i->Save(activity, ImageFormat::Bmp);
		UI::Inc(3); // 3 increments
		return ERROR_SUCCESS;
	} catch (Exception ^) {
		DeleteDir(path);
		return ERROR_BOOTRES_ANIM_SAVE;
	} finally {
		if (i) delete i;
	}
}

// 8 increments
static uint CreateWIM(string path, Bytes &data, string activity) {
	UI::ProgressText = UI::GetMessage(Msg::CompressingAnimation);

	WCHAR wim[MAX_PATH] = {0};
	if (!GetTempFile(wim)) {
		return ERROR_GET_TEMP_FILE;
	}
	UI::Inc();

	uint error = ERROR_SUCCESS;

	// 6 increments + 1
	if (!WIM::Create(as_native(path), wim, as_native(activity)))	{ error = ERROR_BOOTRES_WIM_CAPTURE; }
	else if ((data = ReadAll(wim)) == NULL)							{ error = ERROR_BOOTRES_WIM_READ; }

	UI::Inc();

	DeleteFile(wim);
	return error;
}

uint Bootres::Update(Image ^anim, Color bgColor, Image ^bgImg, FileUpdater f) {
	uint error = ERROR_SUCCESS;
	PEFile *bootres;
	ushort lang = 0;
	string dest = nullptr, act = nullptr;
	Bytes data;

	// Compile activity.bmp (4 increments)
	if ((error = SaveActivityBMP(anim, dest, act, bgColor, bgImg)) != ERROR_SUCCESS)	{ return error; }

	// Load bootres (2 increments)
	else if ((bootres = load(f.path, &error, &lang, false)) == NULL)	{ error += ERROR_BOOTRES_BASE; }

	// Create the new WIM (8 increments)
	else if ((error = CreateWIM(dest, data, act)) != ERROR_SUCCESS)		{ /* error = error;*/ }

	// Modify WIM resource in bootres (2 increments)
	else { error = ModifyResAndSave(bootres, RT_RCDATA, MAKEINTRESOURCE(1), lang, data, ERROR_BOOTRES_BASE, false); }

	// Clean up (1 increment)
	DeleteDir(dest);
	if (data)		free(data);
	if (bootres)	delete bootres;
	UI::Inc();

	return error;
}
