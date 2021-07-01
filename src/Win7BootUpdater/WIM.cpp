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

#include "WIM.h"

#include "UI-native.h"
#include "Files.h"

#ifdef __cplusplus_cli
#pragma unmanaged
#endif

#include <wimgapi.h>
#pragma comment(lib, "wimgapi.lib")

using namespace Win7BootUpdater;

void *WIM::ExtractFile(void *_wim, LPCWSTR file, size_t *size)
{
	TCHAR tempPath[MAX_PATH], tempWimFile[MAX_PATH], tempFile[MAX_PATH];
	if (GetTempPath(MAX_PATH, tempPath) <= 0) return NULL;

	if (GetTempFileName(tempPath, L"wim", 0, tempWimFile) == 0) return NULL;
	if (!Files::WriteAll(tempWimFile, _wim, (uint)*size)) { DeleteFile(tempWimFile); return NULL; }

	HANDLE hWim = WIMCreateFile(tempWimFile, WIM_GENERIC_READ, WIM_OPEN_EXISTING, 0, 0, NULL);
	if (hWim == NULL) { DeleteFile(tempWimFile); return NULL; }
	WIMSetTemporaryPath(hWim, tempPath);

	HANDLE hImg = WIMLoadImage(hWim, 1);
	if (hImg == NULL) { WIMCloseHandle(hWim); DeleteFile(tempWimFile); return NULL; }
	
	bool success = (GetTempFileName(tempPath, L"wex", 0, tempFile) != 0) && (WIMExtractImagePath(hImg, file, tempFile, 0) != 0);
	WIMCloseHandle(hImg);
	WIMCloseHandle(hWim);
	DeleteFile(tempWimFile);
	if (!success) { return NULL; }

	Bytes data = Files::ReadAll(tempFile);
	DeleteFile(tempFile); // TODO: doesn't work (access denied), but explorer can delete the file when holding on this line of code...
	*size = *data;
	return data;
}

/////////////////// WIM Creation //////////////////////////////////////////////

typedef struct _WIMData {
	WCHAR activity[MAX_PATH];
	//UINT lastProgress;
} WIMData;

static DWORD WINAPI callback(DWORD msgId, WPARAM param1, LPARAM param2, PVOID pvUserData) {
	WIMData *data = (WIMData*)pvUserData;

	//First parameter: full file path for if WIM_MSG_PROCESS, message string for others
	TCHAR *message  = (TCHAR*) param1;
	TCHAR *filePath = (TCHAR*) param1;
	//UINT progress   = (UINT)param1;

	//Second parameter: message back to caller if WIM_MSG_PROCESS, error code for others
	DWORD errorCode = (DWORD)  param2;
	DWORD *msg_back = (DWORD*) param2;

	switch (msgId) {
		case WIM_MSG_PROCESS:
			//This message is sent for each file, capturing to see if callee intends to capture the file or not.
			if (_wcsicmp(data->activity, filePath) != 0) *msg_back = FALSE;
			break;

		case WIM_MSG_PROGRESS:
			//if (progress > data->lastProgress + 5) {
			//	UI::Inc(L"Compressing animation... ("+progress+L"%)");
			//	data->lastProgress = (data->lastProgress / 5) * 5;
			//}
			break;

		case WIM_MSG_ERROR:
			//This message is sent upon failure error case
			wprintf(L"ERROR: %s [err = %d]\n", message, errorCode);
			break;

		case WIM_MSG_RETRY:
			//This message is sent when the file is being reapplied because of
			//network timeout. Retry is done up to five times.
			//_tprintf(TEXT("RETRY: %s [err = %d]\n"), message, errorCode);
			break;

		case WIM_MSG_INFO:
			//This message is sent when informational message is available
			//_tprintf(TEXT("INFO: %s [err = %d]\n"), message, errorCode);
			break;

		case WIM_MSG_WARNING:
			//This message is sent when warning message is available
			wprintf(L"WARNING: %s [err = %d]\n", message, errorCode);
			break;
	}
	return WIM_MSG_SUCCESS;
}

#define END_IMAGE			L"</IMAGE>"
#define END_IMAGE_LEN		8
#define WIM_NAME			L"<NAME>Boot Resource WIM</NAME>"

static size_t removeExtraneousWS(LPWSTR str) {
	size_t len = wcslen(str);
	LPWSTR s = (LPWSTR)malloc(len*sizeof(WCHAR));
	size_t j = 0, i = 0;
	bool keep_spaces = false;
	for (; i < len; ++i) {
		if (str[i] == L'<') { keep_spaces = true; }
		else if (str[i] == L'>') { keep_spaces = false; }
		if (keep_spaces || !iswspace(str[i])) {
			s[j++] = str[i];
		}
	}
	wcsncpy(str, s, j);
	str[j] = 0;
	free(s);
	return j;
}

// 6 increments
bool WIM::Create(LPCWSTR src, LPCWSTR dest, LPCWSTR activity) {
	DWORD err = ERROR_SUCCESS;

	WIMData data;
	wcsncpy(data.activity, activity, MAX_PATH);
	//data.lastProgress = 0;

	HANDLE hWim = WIMCreateFile(dest, WIM_GENERIC_WRITE | WIM_GENERIC_READ, WIM_CREATE_ALWAYS, 0, WIM_COMPRESS_LZX, NULL);
	if (hWim == NULL) {
		return false;
	}
	UI::Inc();

	TCHAR tempPath[MAX_PATH];
	if (GetTempPath(MAX_PATH, tempPath) > 0)
		WIMSetTemporaryPath(hWim, tempPath);

	if (WIMRegisterMessageCallback(hWim, (FARPROC)&callback, &data) == INVALID_CALLBACK_VALUE) {
		WIMCloseHandle(hWim);
		return false;
	}
	UI::Inc();

	HANDLE hImg = WIMCaptureImage(hWim, src, 0);
	WIMUnregisterMessageCallback(hWim, (FARPROC)&callback);
	if (hImg == NULL) {
		WIMCloseHandle(hWim);
		return false;
	}
	UI::Inc();

	WCHAR *xml;
	DWORD xmlSize = 0;
	if (!WIMGetImageInformation(hImg, (LPVOID*)&xml, &xmlSize)) {
		err = GetLastError();
	} else {
		size_t xml_len = removeExtraneousWS(xml);
		UI::Inc();

		WCHAR *new_xml = (WCHAR*)malloc((xml_len+64)*sizeof(WCHAR)); // only really need 30 more, but it doesn't hurt
		wcsncpy(new_xml, xml, xml_len-END_IMAGE_LEN);
		new_xml[xml_len-END_IMAGE_LEN] = 0;
		wcscat(new_xml, WIM_NAME);
		wcscat(new_xml, END_IMAGE);
		if (!WIMSetImageInformation(hImg, new_xml, (DWORD)(sizeof(WCHAR)*(wcslen(new_xml)+1)))) {
			err = GetLastError();
		}
		free(new_xml);
		UI::Inc();
	}

	WIMCloseHandle(hImg);
	if (!WIMSetBootImage(hWim, 1)) {
		err = GetLastError();
	}
	WIMCloseHandle(hWim);
	UI::Inc();

	return err == ERROR_SUCCESS;
}
