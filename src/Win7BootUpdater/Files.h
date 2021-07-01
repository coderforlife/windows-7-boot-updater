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

#include "Bytes.h"

namespace Win7BootUpdater { namespace Files {
	void GetNameAndExt(LPCWSTR path, LPWSTR name, LPWSTR ext);
	
	bool ShouldSave(bool exists, int overwrite);
	
	bool Exists(LPCWSTR lpFileName);
	bool GetFullPath(LPCWSTR lpFileName, LPWSTR lpBuffer, uint nBufferLen);
	bool GetTempFile(LPWSTR lpTempFileName, LPCWSTR lpPrefixString = L"w7b");
	bool Seek(HANDLE hFile, LONG lDistanceToMove, uint dwMoveMethod = FILE_BEGIN);
	bool Read(HANDLE hFile, LPVOID lpBuffer, uint dwSize);
	bool ReadAt(HANDLE hFile, LPVOID lpBuffer, uint dwSize, long lDistanceToMove, uint dwMoveMethod = FILE_BEGIN);
	bool Write(HANDLE hFile, LPVOID lpBuffer, uint dwSize);
	bool WriteAt(HANDLE hFile, LPVOID lpBuffer, uint dwSize, long lDistanceToMove, uint dwMoveMethod = FILE_BEGIN);
	
	Bytes ReadAll(LPCWSTR lpFileName); // data must be freed using free()
	bool WriteAll(LPCWSTR lpFileName, LPVOID data, uint size);

	int GetMaxBackupIndex(LPCWSTR path, LPWSTR backup);
	
	bool DeleteAfterExit(LPCWSTR path);

	bool CloseAllHandles(LPCWSTR path);
}}
