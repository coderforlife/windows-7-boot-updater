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

#include "Files.h"

#ifdef __cplusplus_cli
#pragma unmanaged
#endif

// For PathFileExists and PathCombine
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

// For Zw/Nt function in CloseAllHandles
#include "ntdll.h"

using namespace Win7BootUpdater;

void Files::GetNameAndExt(LPCWSTR path, LPWSTR name, LPWSTR ext) {
	LPCWSTR period = wcsrchr(path, L'.'), slash = wcsrchr(path, L'\\');
	wcscpy(name, path);
	if (period != NULL && slash < period) {
		name[period - path] = 0; // does not include period
		wcscpy(ext, period); // includes the period
	} else {
		ext[0] = 0; // no extension
	}
}
bool Files::ShouldSave(bool exists, int overwrite) {
	return (overwrite == OVERWRITE_ALWAYS) || (exists  && overwrite == OVERWRITE_ONLY) || (!exists && overwrite == OVERWRITE_NEVER);
}
bool Files::Exists(LPCWSTR lpFileName) { return PathFileExists(lpFileName) == TRUE; }
bool Files::GetFullPath(LPCWSTR lpFileName, LPWSTR lpBuffer, uint nBufferLen) {
	uint retval = GetFullPathName(lpFileName, nBufferLen, lpBuffer, NULL);
	return retval != 0 && retval < nBufferLen;
}
bool Files::GetTempFile(LPWSTR lpTempFileName, LPCWSTR lpPrefixString) {
	uint retval;
	TCHAR path[MAX_PATH];

	retval = GetTempPath(MAX_PATH, path);
	if (retval == 0 || retval >= MAX_PATH)
		return false;
	retval = GetTempFileName(path, lpPrefixString, 0, lpTempFileName);
	return retval != 0 && retval != ERROR_BUFFER_OVERFLOW;
}
bool Files::Seek(HANDLE hFile, LONG lDistanceToMove, uint dwMoveMethod) {
	return SetFilePointer(hFile, lDistanceToMove, NULL, dwMoveMethod) != INVALID_SET_FILE_POINTER;
}
bool Files::Read(HANDLE hFile, LPVOID lpBuffer, uint dwSize) {
	DWORD dwRead;
	return ReadFile(hFile, lpBuffer, dwSize, &dwRead, NULL) ? true : (dwRead == dwSize);
}
bool Files::ReadAt(HANDLE hFile, LPVOID lpBuffer, uint dwSize, LONG lDistanceToMove, uint dwMoveMethod) {
	return Seek(hFile, lDistanceToMove, dwMoveMethod) && Read(hFile, lpBuffer, dwSize);
}
bool Files::Write(HANDLE hFile, LPVOID lpBuffer, uint dwSize) {
	DWORD dwWrite;
	return WriteFile(hFile, lpBuffer, dwSize, &dwWrite, NULL) ? true : (dwWrite == dwSize);
}
bool Files::WriteAt(HANDLE hFile, LPVOID lpBuffer, uint dwSize, LONG lDistanceToMove, uint dwMoveMethod) {
	return Seek(hFile, lDistanceToMove, dwMoveMethod) && Write(hFile, lpBuffer, dwSize);
}
Bytes Files::ReadAll(LPCWSTR lpFileName) {
	Bytes b;
	
	DISABLE_FS_REDIR();
	HANDLE h = CreateFile(lpFileName, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	REVERT_FS_REDIR();
	if (h == INVALID_HANDLE_VALUE) return b;

	uint size = GetFileSize(h, NULL); // assume not super-huge
	if (size != INVALID_FILE_SIZE) {
		b = Bytes::alloc(size);
		if (!Read(h, b, size)) { free(b); b = Bytes::Null; }
	}
	CloseHandle(h);
	return b;
}
bool Files::WriteAll(LPCWSTR lpFileName, LPVOID data, uint size) {
	DISABLE_FS_REDIR();
	HANDLE h = CreateFile(lpFileName, FILE_WRITE_DATA, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	REVERT_FS_REDIR();
	if (h == INVALID_HANDLE_VALUE) return false;
	bool retval = Write(h, data, size);
	CloseHandle(h);
	return retval;
}


///////////////////////////////////////////////////////////////////////////////
///// Get Max Backup Index
///////////////////////////////////////////////////////////////////////////////
static int GetMaxBackupIndex_Core(int max, WCHAR *backup, size_t num_offset, size_t after_num) {
	LPWSTR x;
	WIN32_FIND_DATA file;
	HANDLE h = FindFirstFile(backup, &file);
	if (h != INVALID_HANDLE_VALUE) {
		do {
			if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { continue; }
			x = file.cFileName+num_offset; // now *%s or *)%s
			int i, n = 0;
			for (i = 0; isdigit(x[i]); ++i) { n = (x[i]-L'0') + 10*n; } // find the number of digits (i) and the number itself
			if (wcslen(x+i) != after_num) { continue; }
			//int n = _wtoi(x);
			if (n > max) max = n;
		} while (FindNextFile(h, &file));
		if (GetLastError() != ERROR_NO_MORE_FILES) { /* error! */ }
		FindClose(h);
	}
	return max;
}

static int GetMaxBackupIndex_Old(LPCWSTR p, LPWSTR b) {
	WCHAR path[MAX_PATH];
	WCHAR backup[MAX_PATH];
	WCHAR name[MAX_PATH], ext[MAX_PATH];

	if (!Files::GetFullPath(p, path, MAX_PATH)) return -1;

	Files::GetNameAndExt(path, name, ext);

	// Check the base name
	_snwprintf(backup, MAX_PATH, L"%s - Backup%s", name, ext);
	int max = PathFileExists(backup) ? 1 : 0;

	// Check for all of the additional backups
	_snwprintf(backup, MAX_PATH, L"%s - Backup (*)%s", name, ext);
	size_t num_offset = wcslen(wcsrchr(name, L'\\')+1)+11, after_num = wcslen(ext)+1;
	max = GetMaxBackupIndex_Core(max, backup, num_offset, after_num);
	if (max == 0)
		b[0] = 0;
	else if (max == 1)
		_snwprintf(b, MAX_PATH, L"%s - Backup%s", name, ext);
	else
		_snwprintf(b, MAX_PATH, L"%s - Backup (%d)%s", name, max, ext);
	return max;
}

static int GetMaxBackupIndex_New(LPCWSTR p, LPWSTR b) {
	WCHAR path[MAX_PATH];
	WCHAR backup[MAX_PATH];
	WCHAR name[MAX_PATH], ext[MAX_PATH];

	if (!Files::GetFullPath(p, path, MAX_PATH)) return -1;

	Files::GetNameAndExt(path, name, ext);

	_snwprintf(backup, MAX_PATH, L"%s~*%s", name, ext);
	size_t num_offset = wcslen(wcsrchr(name, L'\\')+1)+1, after_num = wcslen(ext);
	int max = GetMaxBackupIndex_Core(0, backup, num_offset, after_num);
	if (max == 0)
		b[0] = 0;
	else
		_snwprintf(b, MAX_PATH, L"%s~%d%s", name, max, ext);
	return max;
}

int Files::GetMaxBackupIndex(LPCWSTR p, LPWSTR b) {
	int val = GetMaxBackupIndex_New(p, b);
	return (val > 0) ? val : GetMaxBackupIndex_Old(p, b);
}


///////////////////////////////////////////////////////////////////////////////
///// Delete After Exit (essentially for deleting self)
///////////////////////////////////////////////////////////////////////////////
typedef UINT (WINAPI *WAIT_PROC)  (HANDLE, DWORD);	// WaitForSingleObject
typedef BOOL (WINAPI *CLOSE_PROC) (HANDLE);			// CloseHandle
typedef BOOL (WINAPI *DELETE_PROC)(LPCTSTR);		// DeleteFile
typedef VOID (WINAPI *EXIT_PROC)  (DWORD);			// ExitProcess

typedef struct _INJECT {
	WAIT_PROC	fnWaitForSingleObject;
	CLOSE_PROC	fnCloseHandle;
	DELETE_PROC	fnDeleteFile;
	EXIT_PROC	fnExitProcess;

	HANDLE		hProcess;
	WCHAR		szFileName[MAX_PATH];
} INJECT;

#pragma optimize("gsy", off)
#pragma check_stack(off)		// doesn't work :-(

#define FUNC_SIZE 128

DWORD WINAPI RemoteThread(INJECT *r) {
	r->fnWaitForSingleObject(r->hProcess, INFINITE);
	r->fnCloseHandle(r->hProcess);
	r->fnDeleteFile(r->szFileName);
	r->fnExitProcess(0);
	return 0;
}

#pragma optimize("gsy", on)
#pragma check_stack

static HANDLE GetRemoteProcess() {
	WCHAR cmd[MAX_PATH+1] = L"cmd.exe";
	STARTUPINFO			si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION	pi;
	if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, CREATE_SUSPENDED|CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		CloseHandle(pi.hThread);
		return pi.hProcess;
	}
	return NULL;
}

__inline static PVOID GetFunctionAddr(PVOID func) {
#ifdef _DEBUG
	// get address of function from the JMP <relative> instruction
	DWORD *offset = (DWORD*)((BYTE*)func + 1);
	return (PVOID)(*offset + (BYTE *)func + 5);
#else
	return func;
#endif
}

bool Files::DeleteAfterExit(LPCWSTR path) {
	INJECT	local, *remote;
	LPTHREAD_START_ROUTINE func;
	LPVOID	mem;
	HMODULE	hKernel32;
	HANDLE	hRemoteProcess = GetRemoteProcess(), hCurProc = GetCurrentProcess(), hThread = NULL;
	DWORD	dwThreadId;

	if (hRemoteProcess == NULL)						{ return false; }
	
	// allocate memory in remote process
	mem = VirtualAllocEx(hRemoteProcess, NULL, sizeof(INJECT) + FUNC_SIZE, MEM_RESERVE|MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (mem == NULL)								{ CloseHandle(hRemoteProcess); return false; }
	func = (LPTHREAD_START_ROUTINE)mem;
	remote = (INJECT *)((BYTE*)mem + FUNC_SIZE);

	// setup remote structure
	hKernel32 = GetModuleHandle(L"kernel32.dll");
	local.fnWaitForSingleObject	= (WAIT_PROC)	GetProcAddress(hKernel32, "WaitForSingleObject");
	local.fnCloseHandle			= (CLOSE_PROC)	GetProcAddress(hKernel32, "CloseHandle");
	local.fnExitProcess			= (EXIT_PROC)	GetProcAddress(hKernel32, "ExitProcess");
	local.fnDeleteFile			= (DELETE_PROC)	GetProcAddress(hKernel32, "DeleteFileW");

	// duplicate our own process handle for remote process to wait on
	DuplicateHandle(hCurProc, hCurProc, hRemoteProcess, &local.hProcess, 0, FALSE, DUPLICATE_SAME_ACCESS);
	
	// find name of current executable
	//GetModuleFileName(NULL, local.szFileName, MAX_PATH);
	wcsncpy(local.szFileName, path, MAX_PATH);

	// write in code to execute, and the remote structure
	WriteProcessMemory(hRemoteProcess, func,   GetFunctionAddr(RemoteThread), FUNC_SIZE, NULL);
	WriteProcessMemory(hRemoteProcess, remote, &local, sizeof(local), NULL);

	// execute the code in remote process
	if ((hThread = CreateRemoteThread(hRemoteProcess, NULL, 0, func, remote, 0, &dwThreadId)) != NULL)
		CloseHandle(hThread);
	
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///// Close all file handles
///////////////////////////////////////////////////////////////////////////////
#define OTI_BUF_SIZE 0x100 // max is ~0x88
#define ONI_BUF_SIZE (MAX_PATH+2)*sizeof(WCHAR) // 0x20C
bool Files::CloseAllHandles(LPCWSTR file) {
	// Get the \Device\ style path
	WCHAR drv[3] = { file[0], L':', 0 };
	WCHAR path[MAX_PATH+1];
	if ((file[1] != L':' || file[2] != L'\\' || ((file[0] < L'A' || file[0] > L'Z') && (file[0] < L'a' || file[0] > L'z'))) ||
		QueryDosDeviceW(drv, path, MAX_PATH) == 0 || wcsncat(path, file + 2, MAX_PATH) == 0) return false;

	// Get the system handle information
	DWORD n = 0x100000, *p = (DWORD*)malloc(n);
	while (ZwQuerySystemInformation(SystemHandleInformation, p, n, &n) == STATUS_INFO_LENGTH_MISMATCH)
		p = (DWORD*)realloc(p, n += 0x1000); // some extra just in case
	
	// Setup a bunch of random variables
	DWORD pid = GetCurrentProcessId();
	SYSTEM_HANDLE_INFORMATION *h = (SYSTEM_HANDLE_INFORMATION*)(p + 1);

	BYTE oti_buf[OTI_BUF_SIZE], oni_buf[ONI_BUF_SIZE];
	OBJECT_BASIC_INFORMATION obi;
	PUBLIC_OBJECT_TYPE_INFORMATION *oti = (PUBLIC_OBJECT_TYPE_INFORMATION*)oti_buf;
	OBJECT_NAME_INFORMATION *oni = (OBJECT_NAME_INFORMATION*)oni_buf;

	// Iterate through all handles
	bool retval = true;
	for (DWORD i = 0; i < *p; i++) {
		HANDLE handle = (HANDLE)h[i].Handle;
		if (h[i].dwProcessId == pid && h[i].bFlags != SYSTEM_HANDLE_FLAGS_PROTECT_FROM_CLOSE && // make sure it is for this process and can be closed
			NT_SUCCESS(ZwQueryObject(handle, ObjectBasicInformation, &obi, sizeof(obi), &n)) && // get the information about the handle, making sure it is a file and matches the path
			NT_SUCCESS(ZwQueryObject(handle, ObjectTypeInformation, oti, n = OTI_BUF_SIZE, &n)) && wcsncmp(L"File", oti[0].TypeName.Buffer, 5) == 0 &&
			NT_SUCCESS(ZwQueryObject(handle, ObjectNameInformation, oni, n = ONI_BUF_SIZE, &n)) && wcsncmp(path, oni[0].Name.Buffer, MAX_PATH) == 0) {
				if (!CloseHandle(handle)) {
					retval = false;
					break;
				}
		}
	}

	free(p);
	return retval;
}
