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

#include "WinXXX.h"

#include "Animation.h"
#include "Bootres.h"
#include "BootSkin.h"
#include "ErrorCodes.h"
#include "Resources.h"
#include "UI.h"

#include "MessageTable.h"
#include "Patch.h"

#include "Bytes.h"
#include "Files.h"
#include "PEFile.h"
#include "PEFiles.h"

using namespace Win7BootUpdater;
using namespace Win7BootUpdater::Files;
using namespace Win7BootUpdater::Patches;
using namespace Win7BootUpdater::PEFiles;

using namespace System;
using namespace System::Drawing;
using namespace System::IO;

#define OFFSCREEN_POSITION			10000

#define PATCH_BOOTRES_UNSIGNED		1 // direct
#define PATCH_BOOTRES_MUI_UNSIGNED	2 // direct, not present in winresume
#define PATCH_WINX_MUI_UNSIGNED		3 // direct
#define PATCH_MSG_BG				4
#define PATCH_TEXT_1				5
#define PATCH_PROP_1				6
#define PATCH_YPOS_2				7
#define PATCH_COLOR_2				8
#define PATCH_SIZE_2				9
#define PATCH_BG_IMAGE				10
#define PATCH_BOOTRES_PATH			11 // not present in winload

#define ERROR_WINX(n) (winresume ? ERROR_WINRESUME_##n : ERROR_WINLOAD_##n)
#define ERROR_WINX_MUI(n) (winresume ? ERROR_WINRESUME_MUI_##n : ERROR_WINLOAD_MUI_##n)

#define XSL_NAME (winresume ? L"RESUME.XSL" : L"OSLOADER.XSL")
#define WINX_NAME (winresume ? L"winresume" : L"winload")
#define WINX_NAME_EXE (winresume ? L"winresume.exe" : L"winload.exe")


///////////////////////////////////////////////////////////////////////////////
///// Loading and Checking Functions
///////////////////////////////////////////////////////////////////////////////
static PEFile *load(string path, uint *err, ushort *lang, bool winresume, bool readonly) {
	return LoadAndVerify(as_native(path), err, lang, RT_MESSAGETABLE, XSL_NAME, winresume ? L"hiberrsm.exe" : L"osloader.exe", readonly);
}

uint WinXXX::Check(string path, bool winresume) {
	path = Path::GetFullPath(path);
	ushort lang = 0;
	uint error = ERROR_SUCCESS;
	PEFile *winload = load(path, &error, &lang, winresume, true);
	if (winload) {
		if (winload->getSectionHeaderCount() < 2) {
			error = ERROR_WINX(IS_MUI);
		}
		delete winload;
		return error;
	}
	return error + ERROR_WINX(BASE);
}

uint WinXXX::CheckMui(string path, bool winresume) {
	path = Path::GetFullPath(path);
	ushort lang = 0;
	uint error = 0;
	PEFile *winload = load(path, &error, &lang, winresume, true);
	if (winload) {
		if (winload->getSectionHeaderCount() != 1) {
			error = ERROR_WINX_MUI(IS_NOT_MUI);
		}
		delete winload;
		return error;
	}
	return error + ERROR_WINX_MUI(BASE);
}



///////////////////////////////////////////////////////////////////////////////
///// Message and Color Functions
///////////////////////////////////////////////////////////////////////////////
#define WINLOAD_STARTUP_MSG_ID		9016
#define WINRESUME_STARTUP_MSG_ID	9003
#define STARTUP_MSG_ID (winresume ? WINRESUME_STARTUP_MSG_ID : WINLOAD_STARTUP_MSG_ID)

static string GetMsgLocalized(LPCWSTR file, uint msgId) {
	string str = nullptr;
	HMODULE hMod = LoadLibraryW(file);
	if (!hMod) { return nullptr; }
	LPWSTR buf = NULL;
	if (FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, hMod, msgId, 0, (LPWSTR)&buf, 1, NULL)) {
		str = gcnew System::String(buf);
		LocalFree(buf);
	}
	FreeLibrary(hMod);
	return str;
}

string WinXXX::GetDefaultStartupMsg(bool winresume) {
	string def = GetMsgLocalized(WINX_NAME_EXE, STARTUP_MSG_ID);
	return (def == nullptr) ? (winresume ? L"Resuming Windows" : L"Starting Windows") : def;
}

#define R(c)	(((c) >> 16) & 0xFF)
#define G(c)	(((c) >>  8) & 0xFF)
#define B(c)	(((c)      ) & 0xFF)
//#define C_TO_RGB(c)	(0xFF000000 | ((c.R & 0xFF) << 16) | ((c.G & 0xFF) << 8) | (c.B & 0xFF))
#define C_TO_RGB(c)		(0xFF000000 | (c).ToArgb())
#define RGB_TO_C(c)		Color::FromArgb(0xFF000000 | (c))
#define sq(x)	((x)*(x))
static int Equal(Color c1, unsigned int c2) {
	return c1.R == R(c2) && c1.G == G(c2) && c1.B == B(c2);
}
static double Difference(Color c1, unsigned int c2) {
	double r = (c1.R+R(c2)) / 2.0;
	return Math::Sqrt((2+r/256)*sq(R(c2)-c1.R) + 4*sq(G(c2)-c1.G) + (2+(255-r)/256)*sq(B(c2)-c1.B));
}

int WinXXX::GetClosestBGColorIndex(Color c) {
	if (Equal(c, BackgroundColors[0]))
		return 0;
	double minDist = Difference(c, BackgroundColors[0]), dist;
	int minIndex = 0;
	for (int i = 1; i < BackgroundColors->Length; ++i) {
		if (Equal(c, BackgroundColors[i]))
			return i;
		dist = Difference(c, BackgroundColors[i]);
		if (dist < minDist) {
			minDist = dist;
			minIndex = i;
		}
	}
	return minIndex;
}

Color WinXXX::GetClosestBGColor(Color c) { return RGB_TO_C(BackgroundColors[GetClosestBGColorIndex(c)]); }
string WinXXX::GetXMLColor(Color c) { return ColorXMLs[GetClosestBGColorIndex(c)]; }
Color WinXXX::GetColorFromXml(string xml) {
	for (int i = 0; i < WinXXX::ColorXMLs->Length; ++i)
		if (ColorXMLs[i] == xml)
			return RGB_TO_C(BackgroundColors[i]);
	return Color::Empty;
}


///////////////////////////////////////////////////////////////////////////////
///// Updating Functions
///////////////////////////////////////////////////////////////////////////////
#define APPLY_(id, x)	patch->Apply(f, PATCH_##id, x)
#define APPLY(id, x)	(APPLY_(id, x) && UI::Inc())
#define REVERT(id)		patch->Revert(f, PATCH_##id)

uint WinXXX::Update(int msgCount, Color bg, string text, array<int> ^textSize, array<int> ^textPos, array<Color> ^textColor, bool altBootres, FileUpdater fu, bool winresume) {
	LONG error = ERROR_SUCCESS;
	PEFile *f;
	if (msgCount < 2) { textPos[msgCount] = OFFSCREEN_POSITION; } // moves that message and all subsequent messages off the screen
	array<uint> ^text1prop = gcnew array<uint>{textPos[0], C_TO_RGB(textColor[0]), textSize[0]};
	altBootres &= winresume;

	// Retrieve a PEFileHandle for the file (1 increment)
	if ((f = new PEFile(as_native(fu.path))) == NULL || !f->isLoaded() || !UI::Inc())					{ if (f) delete f; return ERROR_WINX(LOAD); }

	PatchFile ^patch = Res::GetPatch(WINX_NAME);

	// 0 increments
	if (!patch->ApplyIgnoringApplied(f))							{ error = ERROR_WINX(HACK); }
	else if (!(REVERT(BG_IMAGE) && REVERT(COLOR_2)))				{ error = ERROR_WINX(PROP); }

	// 5 increments
	else if (!APPLY(MSG_BG,	C_TO_RGB(bg)))							{ error = ERROR_WINX(PROP); }
	else if (!APPLY(TEXT_1,	text))									{ error = ERROR_WINX(COPYRIGHT); }
	else if (!APPLY(PROP_1, text1prop) || !APPLY(YPOS_2, textPos[1]) || !APPLY(SIZE_2, textSize[1]))	{ error = ERROR_WINX(PROP); }
	else if (!textColor[1].Equals(Color::White) && !APPLY_(COLOR_2, C_TO_RGB(textColor[1])))			{ error = ERROR_WINX(PROP); }
	else if (altBootres && !APPLY_(BOOTRES_PATH, Bootres::altPath))	{ error = ERROR_WINX(PROP); }
	else if (!f->updatePEChkSum())									{ error = ERROR_WINX(PROP); }

	// Cleanup
	if (f) delete f;
	return error;
}

uint WinXXX::Update(bool altBootres, FileUpdater fu, bool winresume) {
	LONG error = ERROR_SUCCESS;
	PEFile *f;
	array<uint> ^text1prop = gcnew array<uint>{OFFSCREEN_POSITION, 0, 0};
	altBootres &= winresume;

	// Retrieve a PEFileHandle for the file (1 increment)
	if ((f = new PEFile(as_native(fu.path))) == NULL || !f->isLoaded() || !UI::Inc())	{ if (f) delete f; return ERROR_WINX(LOAD); }

	PatchFile ^patch = Res::GetPatch(WINX_NAME);

	// 0 increments
	if (!patch->ApplyIgnoringApplied(f))							{ error = ERROR_WINX(HACK); }
	else if (!(REVERT(BG_IMAGE) && REVERT(COLOR_2)))				{ error = ERROR_WINX(PROP); }

	// 1 increment
	else if (!APPLY(PROP_1,	text1prop))								{ error = ERROR_WINX(PROP); }
	else if (altBootres && !APPLY_(BOOTRES_PATH, Bootres::altPath))	{ error = ERROR_WINX(PROP); }

	// 4 increments
	else if (!patch->Apply(f, PATCH_BG_IMAGE) || !UI::Inc(4))		{ error = ERROR_WINX(PROP); }
	else if (!f->updatePEChkSum())									{ error = ERROR_WINX(PROP); }

	// Cleanup
	if (f) delete f;
	return error;
}

///////////////////////////////////////////////////////////////////////////////
///// Updating Resources Functions
///////////////////////////////////////////////////////////////////////////////
// 2 increments
static uint UpdateMessageTable(PEFile *f, string text, ushort lang, bool winresume) {
	Bytes data;

	if ((data = (BYTE*)f->getResource(RT_MESSAGETABLE, MAKEINTRESOURCE(1), lang, &data)) == NULL) { return ERROR_WINX(INVALID_RES); }

	MessageTable ^msgTbl = gcnew MessageTable(~data, *data);
	free(data);
	data = NULL;

	if (!msgTbl->ContainsId(STARTUP_MSG_ID))	{ return ERROR_WINX(MSG_TBL); }
	msgTbl[STARTUP_MSG_ID] = text;
	data = msgTbl->compile(&data);
	if (data == NULL)							{ return ERROR_WINX(MSG_TBL); }
	UI::Inc();

	// Modify message table resource in winload (1 increment)
	uint error = ModifyRes(f, RT_MESSAGETABLE, MAKEINTRESOURCE(1), lang, data, ERROR_WINX(BASE), false);
	free(data);
	return error;
}

// 2 increments
static uint AddBackgroundImage(PEFile *f, Image ^bg, Color bgColor, ushort lang, bool winresume) {
	MemoryStream ^s = nullptr;
	Image ^i = nullptr;
	try {
		i = Animation::ResolveTransparency(bg, Animation::ScreenWidth, Animation::ScreenHeight, bgColor, nullptr);
		i->Save(s = gcnew MemoryStream(), Imaging::ImageFormat::Bmp);
	} catch (Exception ^) {
		return ERROR_WINX(SAVE);
	} finally {
		if (i) delete i;
	}

	Bytes data = Bytes::copy(as_native(s->GetBuffer()), s->Length);
	UI::Inc();

	// Add resource to winload (1 increment)
	uint error = ModifyRes(f, RT_RCDATA, MAKEINTRESOURCE(1), lang, data, ERROR_WINX(BASE), true);
	free(data);
	return error;
}

static uint RemoveBackgroundImage(PEFile *f, ushort lang, bool winresume) {
	UNREFERENCED_PARAMETER(winresume);
	f->removeResource(RT_RCDATA, MAKEINTRESOURCE(1), lang);
	return ERROR_SUCCESS;
}

static LPWSTR FindBackgroundColor(void *data, bool winresume) {
	LPWSTR str = wcsstr((wchar_t*)data, winresume ? L" match=\"graphical-progress-bar\"" : L" match=\"osload-graphics\"");
	return (str == NULL || (str = wcsstr(str, L" background-color=\"")) == NULL || (str = wcschr(str, L'"')) == NULL) ? NULL : str+1;
}

// 2 increments
static uint UpdateBackgroundColor(PEFile *f, Color color, ushort lang, bool winresume) {
	// Get the resource
	Bytes data;
	if ((data = (BYTE*)f->getResource(RT_HTML, XSL_NAME, lang, &data)) == NULL) { return ERROR_WINX(INVALID_RES); }

	// Find the color string
	LPWSTR str = FindBackgroundColor(~data, winresume);
	if (str == NULL)	{ free(data); return ERROR_WINX(XSL); }

	// Copy the new color
	string c = WinXXX::GetXMLColor(color);
	for (int i = 0; i < 4; ++i)
		str[i] = c[i];

	// Modify the xml resource in winload (2 increments)
	uint error = ModifyResAndSave(f, RT_HTML, XSL_NAME, lang, data, ERROR_WINX(BASE), false);
	free(data);
	return error;
}

uint WinXXX::UpdateRes(string text, Color color, FileUpdater fu, bool winresume) {
	uint error = ERROR_SUCCESS;
	PEFile *f = NULL;
	ushort lang = 0;

	// Load winload (2 increments)
	if ((f = load(fu.path, &error, &lang, winresume, false)) == NULL)						{ error += ERROR_WINX(BASE); }

	// Remove the background image (if it exists)
	else if ((error = RemoveBackgroundImage(f, lang, winresume)) != ERROR_SUCCESS)			{ /* error = error; */ }

	// Update message table (2 increments)
	else if ((error = UpdateMessageTable(f, text, lang, winresume)) != ERROR_SUCCESS)		{ /* error = error; */ }

	// Update background color (2 increments)
	else if ((error = UpdateBackgroundColor(f, color, lang, winresume)) != ERROR_SUCCESS)	{ /* error = error; */ }

	// Clean up
	if (f)	delete f;
	return error;
}

uint WinXXX::UpdateRes(Image ^bg, Color color, FileUpdater fu, bool winresume) {
	uint error = ERROR_SUCCESS;
	PEFile *f = NULL;
	ushort lang = 0;

	// Load winload (2 increments)
	if ((f = load(fu.path, &error, &lang, winresume, false)) == NULL)						{ error += ERROR_WINX(BASE); }

	// Update message table (2 increments)
	else if ((error = AddBackgroundImage(f, bg, color, lang, winresume)) != ERROR_SUCCESS)	{ /* error = error; */ }

	// Update background color (2 increments)
	else if ((error = UpdateBackgroundColor(f, color, lang, winresume)) != ERROR_SUCCESS)	{ /* error = error; */ }

	// Clean up
	if (f)	delete f;
	return error;
}


///////////////////////////////////////////////////////////////////////////////
///// Retrieving Functions
///////////////////////////////////////////////////////////////////////////////
string GetStartupMessage(PEFile *f, ushort lang, bool winresume) {
	void *data;
	size_t size;
	if ((data = f->getResource(RT_MESSAGETABLE, MAKEINTRESOURCE(1), lang, &size)) != NULL) {
		MessageTable ^msgTbl = gcnew MessageTable((byte*)data, size);
		free(data);
		if (msgTbl->ContainsId(STARTUP_MSG_ID))
			return msgTbl[STARTUP_MSG_ID];
	}
	return winresume ? L"Resuming Windows" : L"Starting Windows"; // localize?
}
static Image ^GetBackgroundImage(PEFile *f, ushort lang, bool winresume) {
	UNREFERENCED_PARAMETER(winresume);

	// Get the resource
	Bytes data;
	if ((data = (BYTE*)f->getResource(RT_RCDATA, MAKEINTRESOURCE(1), lang, &data)) == NULL) { return nullptr; }

	// Create the bitmap
	try {
		return gcnew Bitmap(gcnew UnmanagedMemoryStream(~data, *data));
	} finally { if (data) free(data); }
	return nullptr;
}
static Color GetBackgroundColor(PEFile *f, ushort lang, bool winresume) {
	// Get the resource
	Bytes data;
	if ((data = (BYTE*)f->getResource(RT_HTML, XSL_NAME, lang, &data)) == NULL) { return Color::Empty; }

	// Find the color string
	LPWSTR str = FindBackgroundColor(~data, winresume);
	if (str == NULL)	{ free(data); return Color::Empty; }

	// Copy the old color
	string xml = gcnew String(str, 0, 4);
	free(data);

	// Get the actual color
	return WinXXX::GetColorFromXml(xml);
}

void WinXXX::GetProperties(string path, BootSkinFile ^bs) {
	uint error = ERROR_SUCCESS;
	PEFile *f = NULL;
	ushort lang = 0;

	bool winresume = bs->IsWinresume();

	// dummy variables
	string str;
	uint val;
	array<uint> ^vals;

	// Load winload
	if ((f = load(path, &error, &lang, winresume, false)) == NULL) { return; }

	PatchFile ^patch = Res::GetPatch(WINX_NAME);

	// Get basic message properties
	bs->MessageCount = 2;
	if (patch->Get(f, PATCH_MSG_BG, val))	bs->MessageBackColor = RGB_TO_C(val);

	// Get message 2 properties
	bs->Message[1] = GetStartupMessage(f, lang, winresume);
	if (patch->Get(f, PATCH_YPOS_2, val)) {
		if (val > OFFSCREEN_POSITION)		bs->MessageCount = 1;
		else								bs->Position[1] = val;
	}
	if (patch->Get(f, PATCH_SIZE_2, val))	bs->TextSize[1] = val;
	if (patch->Get(f, PATCH_COLOR_2, val))	bs->TextColor[1] = RGB_TO_C(val);

	// Get message 1 properties
	if (patch->Get(f, PATCH_TEXT_1, str))	bs->Message[0] = str;
	if ((vals = patch->GetValues(f, PATCH_PROP_1)) != nullptr && vals->Length > 2) {
		if (vals[0] == OFFSCREEN_POSITION)	bs->MessageCount = 0;
		else								bs->Position[0] = vals[0];
		bs->TextColor[0] = RGB_TO_C(vals[1]);
		bs->TextSize[0] = vals[2];
	}

	// Get the background properties
	Color bg = GetBackgroundColor(f, lang, winresume);
	if (bg != Color::Empty)					bs->BackColor = bg;
	if (patch->IsApplied(f, PATCH_BG_IMAGE)) {
		Image ^i = GetBackgroundImage(f, lang, winresume);
		if (i)
			bs->Background = i;
	}

	// Get the animation properties
	//bool alt = winresume && patch->Get(f, PATCH_BOOTRES_PATH, str) && str == Bootres::altPath;
	//if (winresume && !alt)  // Use the winload animation
	//	bs->UseWinloadAnim();
	//else {
	//	/*string file = Path::Combine(Path::GetDirectoryName(file), alt ? Bootres::name : Bootres::altName);

	//	// TODO: Find out if it is using the default animation
	//	bool def = false;

	//	if (def)	// Use the default animation
	//		bs->UseDefaultAnim();
	//	else		// Get the animation
	//		bs->Anim = Bootres::GetAnimation(file);*/
	//}

	if (f) delete f;
}
