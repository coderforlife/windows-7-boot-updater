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

#include "updater.h"

#include "Animation.h"
#include "Bcd.h"
#include "Bootres.h"
#include "Bootmgr.h"
#include "ErrorCodes.h"
#include "FileUpdater.h"
#include "UI.h"
#include "Resources.h"
#include "WinXXX.h"

#include "PDB.h"
#include "Utilities.h"

#include "Files.h"
#include "PEFile.h"
#include "FileSecurity.h"

using namespace Win7BootUpdater;
using namespace Win7BootUpdater::Files;

using namespace System;
using namespace System::IO;
using namespace System::Threading;
using namespace System::Xml;

//native
void Updater::Init() {
	// We need to load these libraries ahead of time for WOW64 because later on we disable file path redirection
	LoadLibrary(L"Version.dll");
	LoadLibrary(L"Advapi32.dll");
	LoadLibrary(L"wimgapi.dll");
	LoadLibrary(L"Shlwapi.dll");
	LoadLibrary(L"security.dll");
	LoadLibrary(L"Secur32.dll");
	LoadLibrary(L"SspiCli.dll");
	FileSecurity::Init(); // loads the delay-loaded security libraries
}
//native
static bool EnablePriv(LPCWSTR name) {
	TOKEN_PRIVILEGES priv = {1, {0, 0, SE_PRIVILEGE_ENABLED}};
	HANDLE hToken;
	bool retval = false;
	if (LookupPrivilegeValue(NULL, name, &priv.Privileges[0].Luid) && OpenProcessToken(GetCurrentProcess(), (TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY), &hToken)) {
		retval = AdjustTokenPrivileges(hToken, FALSE, &priv, sizeof(priv), NULL, NULL)==TRUE;
		CloseHandle(hToken);
	}
	return retval;
}
//native
// Enable the SeDebugPrivilege, SeTakeOwnership, SeRestorePrivilege, and SeBackupPrivilege privileges
// SeDebugPrivilege allows for closing any handles
// SeTakeOwnership allows a file to become under the ownership of the current user
// SeRestorePrivilege allows a file to be put under the ownership of a different user
// SeBackupPrivilege for completeness
#define TEXT(x) L##x // needed for SE_XXX_NAME
bool Updater::EnablePrivileges() { return EnablePriv(SE_TAKE_OWNERSHIP_NAME) && EnablePriv(SE_RESTORE_NAME) && EnablePriv(SE_BACKUP_NAME) && EnablePriv(SE_DEBUG_NAME); }
#undef TEXT
//native
void Updater::Cleanup() { FileSecurity::Cleanup(); }

//pure
static string GetAltBootresPath(BootSkinFile ^bs, string path) {
	if (!bs->AnimIsNotSet()) {
		path = GetFullPath(path);
		string alt = Path::Combine(Path::GetDirectoryName(path), Bootres::altName);
		// Do I need to allow access to the directory?
		File::Copy(path, alt, true);
		return alt;
	}
	return nullptr;
}

// 18 increments
//pure
static uint UpdateBootres(BootSkinFile ^bs, string path, FileUpdater %file, bool backup, uint error) {
	if (bs->AnimIsNotSet()) {
		UI::Inc(18);
	} else {
		if ((error = file.Init(ERROR_BOOTRES_BASE, path, backup)) == ERROR_SUCCESS) { // 1 increment
			return Bootres::Update(bs->Anim, bs->BackColor, bs->UsesBackgroundImage() ? bs->Background : nullptr, file); // 17 increments
		}
	}
	return error;
}

//20 increments
//pure
static uint UpdateWinXXX(BootSkinFile ^bs, string path, string muiPath, FileUpdater %file, FileUpdater %fileMui, bool backup, uint error) {
	// Winload (13 increments)
	if (error == ERROR_SUCCESS && (error = file.Init(ERROR_WINLOAD_BASE, path, backup)) == ERROR_SUCCESS) { // 1 increment
		bool alt = bs->IsWinresume() && !bs->AnimIsNotSet();
		if (bs->UsesBackgroundImage()) {
			error = WinXXX::UpdateRes(bs->Background, bs->BackColor, file, bs->IsWinresume()); // 6 increments
			if (error == ERROR_SUCCESS) {
				error = WinXXX::Update(alt, file, bs->IsWinresume()); // 6 increments
			}
		} else {
			error = WinXXX::UpdateRes(bs->Message[1], bs->BackColor, file, bs->IsWinresume()); // 6 increments
			if (error == ERROR_SUCCESS) {
				error = WinXXX::Update(bs->MessageCount, bs->MessageBackColor, bs->Message[0], bs->TextSizes, bs->Positions, bs->TextColors, alt, file, bs->IsWinresume()); // 6 increments
			}
		}
	}

	// Winload (MUI) Resources (7 increments)
	if (error == ERROR_SUCCESS && (error = fileMui.Init(ERROR_WINLOAD_MUI_BASE, muiPath, backup)) == ERROR_SUCCESS) { // 1 increment
		if (bs->UsesBackgroundImage()) {
			error = WinXXX::UpdateRes(bs->Background, bs->BackColor, fileMui, bs->IsWinresume()); // 6 increments
		} else {
			error = WinXXX::UpdateRes(bs->Message[1], bs->BackColor, fileMui, bs->IsWinresume()); // 6 increments
		}
	}

	return error;
}

//pure
static uint IncOrFinishUp(bool b, FileUpdater %f, uint error) { if (b) UI::Inc(); return b ? error : f.FinishUp(error); }

//mixed
uint Updater::Update(BootSkin ^bs, string bootresPath, string winloadPath, string winloadMuiPath, string winresumePath, string winresumeMuiPath, string bootmgrPath, bool backup /*, array<string> ^%modifiedPaths*/) {
	uint error = ERROR_SUCCESS;

	FileUpdater bootres, bootresAlt, winload, winloadMui, winresume, winresumeMui, bootmgr;
	BootSkinFile ^bsWinload = bs->Winload, ^bsWinresume = bs->Winresume;

	DISABLE_FS_REDIR();

	string altBootresPath = GetAltBootresPath(bsWinresume, bootresPath);

	try {

		error = UpdateBootres(bsWinload, bootresPath, bootres, backup, error);
		error = UpdateBootres(bsWinresume, altBootresPath, bootresAlt, backup, error);
		error = UpdateWinXXX(bsWinload, winloadPath, winloadMuiPath, winload, winloadMui, backup, error);
		error = UpdateWinXXX(bsWinresume, winresumePath, winresumeMuiPath, winresume, winresumeMui, backup, error);

		// Bootmgr (7 increments)
		if (error == ERROR_SUCCESS && (error = bootmgr.Init(ERROR_BOOTMGR_BASE, bootmgrPath, backup)) == ERROR_SUCCESS) { // 1 increment
			error = Bootmgr::Update(bootmgr); // 6 increments
		}
	} catch (Exception ^) {
		error = ERROR_THROWN;
		throw;
	} finally {
		REVERT_FS_REDIR();
		
		if (error == ERROR_SUCCESS) {
			UI::ProgressText = UI::GetMessage(Msg::FinishingUp);
		}

		// 7 increments
		error = IncOrFinishUp(bsWinload->AnimIsNotSet(), bootres, error);
		error = IncOrFinishUp(bsWinresume->AnimIsNotSet(), bootresAlt, error);
		error = winload.FinishUp(error);
		error = winloadMui.FinishUp(error);
		error = winresume.FinishUp(error);
		error = winresumeMui.FinishUp(error);
		error = bootmgr.FinishUp(error);
	}
	//if (error == 0)
	//	modifiedPaths = gcnew array<string>{bootres.Backup, bootresAlt.Backup, winload.Backup, winloadMui.Backup, winresume.Backup, winresumeMui.Backup, bootmgr.Backup};
	return error;
}

//mixed
array<string> ^Updater::Restore(... array<string> ^files) {
	array<string> ^results = gcnew array<string>(files->Length);
	DISABLE_FS_REDIR();
	for (int i = 0; i < files->Length; ++i) {
		string file = files[i];
		const wchar_t *f = as_native(file);
		WCHAR backup[MAX_PATH];
		if (GetMaxBackupIndex(f, backup) > 0) {
			FileSecurity::CurrentUserFullAccess(f);
			if (!CopyFile(backup, f, false))
				continue;
			FileSecurity::CurrentUserFullAccess(backup);
			DeleteFile(backup);
			results[i] = gcnew String(backup);
		}
	}
	REVERT_FS_REDIR();
	return results;
}

//mixed
string Updater::DownloadPDB(string file) {
	DISABLE_FS_REDIR();
	PEFile* pe = new PEFile(as_native(file), true);
	string dest = pe->isLoaded() ? PDB::DownloadToDefault(pe) : nullptr;
	delete pe;
	REVERT_FS_REDIR();
	return dest;
}

delegate void Saver(Stream ^s);

inline static array<byte> ^SaveIt(Saver ^save) {
	MemoryStream ^s = nullptr;
	try { save(s = gcnew MemoryStream()); }
	catch (Exception^) { return nullptr; }
	finally { if (s) s->Close(); }
	return s->ToArray();
}

//mixed
string Updater::CreateInstaller(BootSkin ^bs, XmlDocument ^desc, System::Drawing::Image ^image, array<byte> ^installerBase, string file) {
	// Get boot skin data
	array<byte> ^bs_data = SaveIt(gcnew Saver(bs, &BootSkin::Save));
	if (!bs_data) { return UI::GetMessage(Msg::FailedToGetBootSkinData); }

	// Get desc data
	desc->InsertBefore(desc->CreateXmlDeclaration(L"1.0", L"utf-8", nullptr), desc->FirstChild);
	desc->PreserveWhitespace = true; // this actually removes extra formatting whitespace added by the save function (by preserving the whitespace exactly, and not adding any extra).
	array<byte> ^desc_data = SaveIt(gcnew Saver(desc, &XmlDocument::Save));

	// Get the image data
	array<byte> ^img_data = Animation::GetPngData(image);

	Msg err = Msg::NO_MESSAGE;

	// Modify the installer
	// Cannot use NATIVE(installerBase) since that memory cannot be realloc-ed
	PEFile *f = new PEFile(Utilities::GetNativeArray(installerBase), installerBase->Length);
	if (!f->isLoaded())																{ err = Msg::FileCouldNotBeReadAsPE; }
	else if (!f->addResource(RT_RCDATA, MAKEINTRESOURCE(1), 0, NATIVE(bs_data)))	{ err = Msg::FailedToUpdateTheResources; }
	else if (!f->addResource(RT_RCDATA, MAKEINTRESOURCE(2), 0, NATIVE(desc_data)))	{ err = Msg::FailedToUpdateTheResources; }
	else if (!f->addResource(RT_RCDATA, MAKEINTRESOURCE(3), 0, NATIVE(img_data)))	{ err = Msg::FailedToUpdateTheResources; }
	else if (!f->save() || !WriteAll(as_native(file), f->get(), f->getSize()))		{ err = Msg::FailedToSaveTheUpdated; }
	delete f;

	return err == Msg::NO_MESSAGE ? nullptr : UI::GetMessage(err, UI::GetMessage(Msg::Installer));
}

//native
#ifndef _WIN64
static __declspec(thread) BOOL _fs_disabled = FALSE;
static __declspec(thread) PVOID _fs_old_state = NULL;
void Updater::DisableFSRedirection() { if (!_fs_disabled) { _fs_disabled = Wow64DisableWow64FsRedirection(&_fs_old_state); } }
void Updater::RevertFSRedirection() { if (_fs_disabled) { Wow64RevertWow64FsRedirection(_fs_old_state); _fs_disabled = FALSE; } }
#else
void Updater::DisableFSRedirection() {}
void Updater::RevertFSRedirection() {}
#endif

//mixed
bool Updater::DeleteAfterExit(string path) { return Files::DeleteAfterExit(as_native(path)); }

//pure
string Updater::GetPatchVersion(string name) { return Res::GetPatch(name)->Version; }

//pure
static void LoadPatch(object name) { Res::GetPatch((string)name); }
void Updater::PreloadPatch(string name) {
	Thread ^t = gcnew Thread(gcnew ParameterizedThreadStart(LoadPatch));
	t->Name = L"Patch Preloader: "+name;
	t->Start(name);
}

//pure
//System::Version ^Updater::GetLatestAvailableVersion() { return Res::GetLatestAvailableVersion(); }

//pure
string Updater::GetPreferredLocale() { return BCD::PreferredLocale; }

//pure
bool Updater::IsWindowsVistaOrNewer() { return Environment::OSVersion->Platform == PlatformID::Win32NT && Environment::OSVersion->Version >= gcnew System::Version(6, 0, 6000); }

//pure
bool Updater::IsWindows7OrNewer() { return Environment::OSVersion->Platform == PlatformID::Win32NT && Environment::OSVersion->Version >= gcnew System::Version(6, 1, 7600); }

//native
bool Updater::Is64BitWindows() {
#ifdef _WIN64
	return true;
#else
	//BOOL is32in64;
	//return IsWow64Process(GetCurrentProcess(), &is32in64) && is32in64;

	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	return si.dwProcessorType == PROCESSOR_ARCHITECTURE_AMD64;
#endif
}
