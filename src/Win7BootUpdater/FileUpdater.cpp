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

#include "FileUpdater.h"

#include "ErrorCodes.h"
#include "UI.h"

#include "Bytes.h"

#include "Files.h"
#include "FileSecurity.h"
#include "PEFile.h"

using namespace Win7BootUpdater;

using namespace System;
using namespace System::IO;

#pragma unmanaged

static LPWSTR GetFolder(LPCWSTR path, LPWSTR folder) {
	wcscpy(folder, path); // copy the path
	*wcsrchr(folder, L'\\') = 0; // remove the file name from the path
	return folder;
}

static LPWSTR createBackup(LPCWSTR path) {
	LPWSTR backup = (LPWSTR)malloc(MAX_PATH*sizeof(WCHAR));
	WCHAR name[MAX_PATH], ext[MAX_PATH];
	DWORD i = 0;
	Files::GetNameAndExt(path, name, ext);
	do {
		_snwprintf(backup, MAX_PATH, L"%s~%d%s", name, ++i, ext);
	} while (Files::Exists(backup));
	return CopyFile(path, backup, true) ? backup : NULL;
}

#pragma managed

static HANDLE OpenFileForAttr(LPCWSTR path, bool write) {
	HANDLE h = CreateFile(path, write ? FILE_WRITE_ATTRIBUTES : FILE_READ_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE)
		throw gcnew FileNotFoundException(UI::GetMessage(Msg::CreateFileFailed, GetLastError()), gcnew String(path));
	// Don't change the access file time
	/*FILETIME x = {0xFFFFFFFF, 0xFFFFFFFF};
	if (!SetFileTime(h, NULL, &x, NULL)) {
		CloseHandle(h);
		throw gcnew InvalidOperationException();
	}*/
	return h;
}

string FileUpdater::Backup::get() {
	return backup;
}

inline static DateTime ConvertTime(FILETIME f) { return DateTime::FromFileTimeUtc((((__int64)f.dwHighDateTime)<<32) | f.dwLowDateTime); }
inline static FILETIME ConvertTime(DateTime d) { __int64 t = d.ToFileTimeUtc(); FILETIME f = {(uint)t, ((uint)(t>>32))}; return f; }

static void GetFileTimes(/*string path,*/ LPCWSTR path, DateTime %atime, DateTime %mtime, DateTime %ctime) {
	// It would be nice to do it this way
	// But the paths to the hidden volume (\\?\Volume{...}) do not work with the .NET functions
	//atime = File::GetLastAccessTimeUtc(path);
	//mtime = File::GetLastWriteTimeUtc(path);
	//ctime = File::GetCreationTimeUtc(path);

	HANDLE h = OpenFileForAttr(path, false);
	FILETIME a, m, c;
	BOOL res = GetFileTime(h, &c, &a, &m);
	CloseHandle(h);
	if (!res) throw gcnew InvalidOperationException();
	atime = ConvertTime(a); mtime = ConvertTime(m); ctime = ConvertTime(c);
}
static void SetFileTimes(/*string path,*/ LPCWSTR path, DateTime atime, DateTime mtime, DateTime ctime) {
	// It would be nice to do it this way
	// But the paths to the hidden volume (\\?\Volume{...}) do not work with the .NET functions
	//File::SetCreationTimeUtc(path, ctime);
	//File::SetLastWriteTimeUtc(path, mtime);
	//File::SetLastAccessTimeUtc(path, atime);

	HANDLE h = OpenFileForAttr(path, true);
	FILETIME a = ConvertTime(atime), m = ConvertTime(mtime), c = ConvertTime(ctime);
	BOOL res = SetFileTime(h, &c, &a, &m);
	CloseHandle(h);
	if (!res) throw gcnew InvalidOperationException();
}

/*static string createBackup(string path) {
	// It would be nice to do it this way
	// But the paths to the hidden volume (\\?\Volume{...}) do not work with the .NET functions
	string name = Path::Combine(Path::GetDirectoryName(path), Path::GetFileNameWithoutExtension(path));
	string ext = Path::GetExtension(path);
	string backup;
	uint i = 0;
	do {
		backup = name + L"~" + (++i) + ext;
	} while (File::Exists(backup) || Directory::Exists(backup));
	try {
		File::Copy(path, backup);
	} catch (Exception ^) { backup = nullptr; }
	return backup;
}*/

// 1 increment
uint FileUpdater::Init(uint error_base, string path, bool backup) {
	if (initialized) return ERROR_SUCCESS;

	path = GetFullPath(path); // make sure its a full path!

	UI::ProgressText = UI::GetMessage(Msg::Updating, Path::GetFileName(path));

	this->error_base = error_base;
	this->path = path;

	const wchar_t *c_path = as_native(path);

	// Grab timestamps of the file
	GetFileTimes(c_path, atime, mtime, ctime);

	initialized = true;

	// Need to make sure the user has the ability to create files in the parent folder
	WCHAR folder[MAX_PATH];
	FileSecurity::AddCurrentUserAccess(GetFolder(c_path, folder)); // add the current user to the security

	// Create a backup
	if (backup) {
		const wchar_t *c_backup = createBackup(c_path);
		if (c_backup == NULL) {
			return FinishUp(GEN_ERR_CREATE_BACKUP + error_base);
		}
		this->backup = gcnew String(c_backup);
		try {
			SetFileTimes(c_backup, atime, mtime, ctime);
		} catch (Exception ^) {}
	}

	// Remove any protective security on the file
	sec = (IntPtr)FileSecurity::Get(c_path);
	if (sec == IntPtr::Zero || !FileSecurity::CurrentUserFullAccess(c_path)) {
		return FinishUp(GEN_ERR_DEACTIVE_SEC + error_base);
	}
	UI::Inc();

	return ERROR_SUCCESS;
}

// 1 increment
uint FileUpdater::FinishUp(uint error) {
	if (!initialized) return error;

	DISABLE_FS_REDIR();

	const wchar_t *c_path = as_native(path);
	const wchar_t *c_backup = backup ? as_native(backup) : NULL;

	// Make sure that all file handles are completely closed (they have a way of staying open when errors happen)
	PEFile::UnmapAllViewsOfFile(c_path);
	Files::CloseAllHandles(c_path);
	if (c_backup)
		Files::CloseAllHandles(c_backup);

	// Restore the backup in case of an error
	if (c_backup && error != ERROR_SUCCESS && CopyFile(c_backup, c_path, false))
		DeleteFile(c_backup);

	// Restore timestamps to the file
	try {
		SetFileTimes(c_path, atime, mtime, ctime);
	} catch (Exception ^) {}

	// Restore security to the files
	if (sec != IntPtr::Zero) {
		if (!FileSecurity::Restore(c_path, sec.ToPointer()) && error == ERROR_SUCCESS) { error = GEN_ERR_RESTORE_SEC + error_base; }
		if (c_backup && !FileSecurity::Restore(c_backup, sec.ToPointer()) && error == ERROR_SUCCESS) { error = GEN_ERR_RESTORE_SEC + error_base; }
	}
	UI::Inc();

	REVERT_FS_REDIR();

	initialized = false;
	return error;
}
FileUpdater::FileUpdater() : initialized(false) {}
FileUpdater::FileUpdater(FileUpdater% x) : initialized(x.initialized), error_base(x.error_base), path(x.path), backup(x.backup), ctime(x.ctime), mtime(x.mtime), atime(x.atime) {
	sec = (IntPtr)FileSecurity::DuplicateData(x.sec.ToPointer());
}
FileUpdater::~FileUpdater() {
	if (sec != IntPtr::Zero) FileSecurity::FreeData(sec.ToPointer());
}
