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

#include "Bcd.h"

#include "BcdConstants.h"
#include "BcdObject.h"
#include "WMI.h"

#ifndef InBootVersions
#include "UI.h"
#define Msg_UnableToLoadTheBCDStore()									UI::GetMessage(Msg::UnableToLoadTheBCDStore)
#define Msg_PathInTheBCDIsEitherInvalidOrNotYetSupported(n)				UI::GetMessage(Msg::PathInTheBCDIsEitherInvalidOrNotYetSupported, n)
#define Msg_PathInTheBCDReferencesAnEFIWhichIsCurrentlyNotSupported(n)	UI::GetMessage(Msg::PathInTheBCDReferencesAnEFIWhichIsCurrentlyNotSupported, n)
#define Msg_PathInTheBCDIsInvalid(n)									UI::GetMessage(Msg::PathInTheBCDIsInvalid, n)
#else
#define Msg_UnableToLoadTheBCDStore()									L"Unable to load the BCD store"
#define Msg_PathInTheBCDIsEitherInvalidOrNotYetSupported(n)				L"The "+n+L" path in the BCD is either invalid or is a not yet supported type"
#define Msg_PathInTheBCDReferencesAnEFIWhichIsCurrentlyNotSupported(n)	L"The "+n+L" path in the BCD references an EFI file which is currently not supported"
#define Msg_PathInTheBCDIsInvalid(n)									L"The "+n+L" path in the BCD is invalid"
#endif

#include "Files.h"

//#define USE_ACCURATE_GPT_LOOKUP_METHOD

#ifdef USE_ACCURATE_GPT_LOOKUP_METHOD
#include <WinIoCtl.h> // for drive layout stuff
#include <Objbase.h> // to convert string to GUID (IIDFromString)
#pragma comment(lib, "Ole32.lib") // for IIDFromString
#define MAX_PARTITIONS		128
#define MAX_DRIVES			64
#define DRIVE_LAYOUT_SIZE	sizeof(DRIVE_LAYOUT_INFORMATION_EX) + MAX_PARTITIONS*sizeof(PARTITION_INFORMATION_EX)
#endif

using namespace Win7BootUpdater;

using namespace ROOT::WMI;

using namespace System;
using namespace System::IO;
using namespace System::Runtime::InteropServices;

inline static string NicerPath(string vname) {
	// Get a nicer path (if possible)
	WCHAR paths[1024];
	DWORD count = 0;
	string path = nullptr;
	if (GetVolumePathNamesForVolumeName(as_native(vname), paths, ARRAYSIZE(paths), &count)) {
		size_t len = 0;
		for (LPWSTR p = paths; p[0] != 0; p += len + 1) {
			len = wcslen(p);
			if (path == nullptr || path->Length > (int)len)
				path = gcnew String(p);
		}
	}
	return path ? path : vname;
}

void BCD::Init() {
	if (initialized) return;

	ConnectionOptions ^opts = gcnew ConnectionOptions();
	opts->Impersonation = ImpersonationLevel::Impersonate;
	opts->EnablePrivileges = true;

	// Load the scope
	scope = gcnew ManagementScope(L"root\\WMI", opts);

	// Variable used for searching volumes
	HANDLE sh = NULL;
	WCHAR vname[MAX_PATH+1];
	size_t len;

	for (;;) {
		// Open the system store
		try {
			bootmgr = gcnew BcdObject(scope, Guids::SystemStore, storePath);
		} catch (Exception^) {}

		// if "bootmgr" is bad, then start to search
		if (bootmgr == nullptr || bootmgr->IsTypeNull) {
			// Try next volume
			if (sh == NULL)
				if ((sh = FindFirstVolume(vname, ARRAYSIZE(vname))) == INVALID_HANDLE_VALUE)	break;
			else
				if (!FindNextVolume(sh, vname, ARRAYSIZE(vname)))								break;
			
			// Make sure the volume path is valid
			len = wcslen(vname);
			if (len < 5 || wcsncmp(vname, L"\\\\?\\", 4) != 0 || vname[len-1] != L'\\')			continue; // will fail to open again and advance to the next volume

			storePath = NicerPath(gcnew String(vname)) + L"Boot\\BCD";

			// Make sure file exists?

			continue;
		}

		// Get the current OS loader
		try {
			current = gcnew BcdObject(scope, Guids::Current, storePath);
		} catch (Exception^) {}

		// if "current" is bad, then get default
		if (current == nullptr || current->IsTypeNull) {
			try {
				String ^def = GetString(bootmgr, BcdBootMgrObject_DefaultObject);
				if (def != nullptr) current = gcnew BcdObject(scope, def, storePath);
			} catch (Exception^) {}
		}

		// Success
		initialized = true;
		break;
	}

	if (sh) FindVolumeClose(sh);
	if (!initialized)
		throw gcnew Exception(Msg_UnableToLoadTheBCDStore());
}

string BCD::StorePath::get() { Init(); return storePath; }
BcdObject ^BCD::Bootmgr::get() { Init(); return bootmgr; }
BcdObject ^BCD::Current::get() { Init(); return current; }

/*
void BCD::Enum(BcdObject ^o) {
	array<ManagementBaseObject^> ^elems;
	array<uint> ^types;
	bool success_elems = o->EnumerateElements(elems);
	bool success_types = o->EnumerateElementTypes(types);
	for (int i = 0; i < elems->Length; ++i)
		System::Diagnostics::Debug::WriteLine(String::Format("0x{0:X8} {1}", types[i], elems[i]->GetText(TextFormat::Mof)));
}
*/

string BCD::PreferredLocale::get() {
	string locale = nullptr;
	try {
		string locale = GetString(Current, BcdLibraryString_PreferredLocale);
		if (!locale) locale = GetString(Bootmgr, BcdLibraryString_PreferredLocale);
	} catch (Exception^) {}
	return locale ? locale : System::Globalization::CultureInfo::InstalledUICulture->Name;
}

string BCD::GetString(BcdObject ^o, unsigned int t) {
	ManagementBaseObject ^mbo;
	return o->GetElement(t, mbo) ? (string)mbo[L"String"] : nullptr;
}

string BCD::GetGUID(BcdObject ^o, unsigned int t) {
	ManagementBaseObject ^mbo;
	return o->GetElement(t, mbo) ? (string)mbo[L"Id"] : nullptr;
}

/*inline static array<string> ^GetIds(BcdObject ^o, unsigned int t) {
	ManagementBaseObject ^mbo;
	return o->GetElement(t, mbo) ? (array<string>^)mbo[L"Ids"] : nullptr;
}

inline static bool Has(BcdObject ^o, unsigned int t) {
	ManagementBaseObject ^mbo;
	return o->GetElement(t, mbo);
}

inline static void Remove(BcdObject ^o, unsigned int t) {
	if (!o->DeleteElement(t))
		throw gcnew InvalidOperationException();
}*/

#ifdef USE_ACCURATE_GPT_LOOKUP_METHOD
static DRIVE_LAYOUT_INFORMATION_EX *GetDriveLayoutInfo(int i) { // must be freed
	// Get the drive name and open it
	HANDLE dev = CreateFile(as_native(L"\\\\.\\PhysicalDrive" + i), FILE_READ_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dev == INVALID_HANDLE_VALUE) { return NULL; } // check if that drive doesn't exist

	// Get the layout information for the drive
	DWORD size = DRIVE_LAYOUT_SIZE;
	DRIVE_LAYOUT_INFORMATION_EX *layout = (DRIVE_LAYOUT_INFORMATION_EX*)malloc(DRIVE_LAYOUT_SIZE);
	if (!DeviceIoControl(dev, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, layout, size, &size, NULL)) {
		free(layout);
		layout = NULL;
	}
	CloseHandle(dev);

	return layout;
}
#endif

static string GetWindowsDevicePath(const WCHAR *dos) {
	HANDLE sh;
	WCHAR vname[MAX_PATH+1], dname[MAX_PATH+1];
	size_t len;

	string path = nullptr;

	// Get the first volume (handle is used to iterate)
	sh = FindFirstVolume(vname, ARRAYSIZE(vname));
	if (sh == INVALID_HANDLE_VALUE)
		return nullptr;

	// Iterate through all volumes
	do {
		// Make sure the volume path is valid
		len = wcslen(vname);
		if (len < 5 || wcsncmp(vname, L"\\\\?\\", 4) != 0 || vname[len-1] != L'\\')
			continue;

		// Check the device name (which doesn't work with a trailing '\')
		vname[len-1] = 0;		
		if (QueryDosDeviceW(vname+4, dname, ARRAYSIZE(dname)) && wcscmp(dname, dos) == 0) {
	        vname[len-1] = L'\\';
			path = NicerPath(gcnew String(vname));
			break;
		}
	} while (FindNextVolume(sh, vname, ARRAYSIZE(vname)));

	FindVolumeClose(sh);
	return path;
}
static string GetWindowsDevicePath(string dos) { return GetWindowsDevicePath(as_native(dos)); }
static string GetWindowsDevicePathWithQuery(string dev) { WCHAR dos[MAX_PATH+1]; return QueryDosDeviceW(as_native(dev), dos, ARRAYSIZE(dos)) ? GetWindowsDevicePath(dos) : nullptr; }
static string GetWindowsDevicePath(uint32 diskIndex, uint32 partIndex) { return GetWindowsDevicePathWithQuery(L"Harddisk"+diskIndex+L"Partition"+partIndex); }

enum BcdDeviceTypes sealed : unsigned __int32 {
	BootDevice = 1,
	PartitionDevice = 2,
	FileDevice = 3,
	RamdiskDevice = 4,
	UnknownDevice = 5,
	QualifiedPartition = 6,
	LocateDevice = 7,
	LocateExDevice = 8
};

string BCD::GetDevicePath(BcdObject ^o, unsigned int t) {
	Init();

	ManagementBaseObject ^mbo;
	if (!o->GetElement(t, mbo))
		return nullptr;
	ManagementBaseObject ^device = (ManagementBaseObject^)mbo[L"Device"];
	// BcdDeviceData {
	//   DeviceType : BcdDeviceTypes
	//   AdditionalOptions : string
	// }
	BcdDeviceTypes type = (BcdDeviceTypes)(unsigned __int32)device[L"DeviceType"];

	string dev, signature, id;
	uint32 style;

	switch (type) {
	case PartitionDevice:
		// BcdDevicePartitionData : BcdDeviceData {
		//   Path : string
		// }
		dev = (string)device[L"Path"];
		return (dev && dev->ToLower()->StartsWith(L"\\device\\harddiskvolume")) ? GetWindowsDevicePath(dev) : dev;

	case QualifiedPartition:
		// BcdDeviceQualifiedPartitionData : BcdDeviceData {
		//   PartitionStyle : uint32
		//   DiskSignature : string 
		//   PartitionIdentifier : string 
		// }
		style = (uint32)device[L"PartitionStyle"];
		signature = (string)device[L"DiskSignature"];
		id = (string)device[L"PartitionIdentifier"];
		if (style == 0) { // MBR
			uint32 diskIndex = (uint32)WMI::QueryFirst(L"SELECT Index FROM Win32_DiskDrive WHERE Signature="+signature, L"Index");
			uint32 partIndex = (uint32)WMI::QueryFirst(L"SELECT Index FROM Win32_DiskPartition WHERE DiskIndex="+diskIndex+L" StartingOffset="+id, L"Index") + 1;
			return GetWindowsDevicePath(diskIndex, partIndex);
		} else if (style == 1) { // GPT
#ifdef USE_ACCURATE_GPT_LOOKUP_METHOD
			GUID gpt_signature, gpt_id;
			if (IIDFromString(as_native(signature), &gpt_signature) != S_OK || IIDFromString(as_native(id), &gpt_id) != S_OK)
				break;
			for (DWORD i = 0; i < MAX_DRIVES; ++i) {
				DRIVE_LAYOUT_INFORMATION_EX *layout = GetDriveLayoutInfo(i);
				if (layout && layout->PartitionStyle == PARTITION_STYLE_GPT && layout->Gpt.DiskId == gpt_signature) {
					for (DWORD j = 0; j < layout->PartitionCount; ++j) {
						PARTITION_INFORMATION_EX p = layout->PartitionEntry[j];
						if (p.PartitionStyle == PARTITION_STYLE_GPT && p.Gpt.PartitionType == gpt_id) {
							free(layout);
							return GetWindowsDevicePath(i, p.PartitionNumber);
						}
					}
				}
				free(layout);
			}
#else
			return NicerPath(L"\\\\?\\Volume{"+id+L"}\\");
#endif
		}
		break;
		
	case BootDevice:
		// The "AdditionalOptions" contains an ID as well?

		// Not accurate, this is the "boot partition" but really we want the "active partition":
		//return NicerPath((string)WMI::QueryFirst(L"SELECT DeviceID FROM Win32_Volume WHERE BootVolume=TRUE", L"DeviceID"));

		mbo = WMI::QueryFirst(L"SELECT DiskIndex, Index FROM Win32_DiskPartition WHERE BootPartition=TRUE");
		return GetWindowsDevicePath((uint32)mbo[L"DiskIndex"], (uint32)mbo[L"Index"]+1);

		// Or maybe:
		//dev = (string)WMI::QueryFirst(L"SELECT Description FROM Win32_BootConfiguration", L"Description"); // \Device\Harddisk0\Partition1
		//return GetWindowsDevicePathWithQuery(dev->Substring(8)->Replace(L"\\", L"")); // Harddisk0Partition1

	case LocateExDevice: // for dynamically found VHD files
		// BcdDeviceLocateData : BcdDeviceData {
		//   Type: uint32 { Element : 0, String : 1, ElementChild : 2 }
		// }

		//NOT SUPPORTED YET!
		break;
		
	case RamdiskDevice:
		//NOT SUPPORTED YET!
		break;

	case FileDevice: // no idea what this is actually for
		// BcdDeviceFileData : BcdDeviceData {
		//   Parent : BcdDeviceData
		//   Path : string
		// }

		//NOT SUPPORTED YET!
		break;

	case LocateDevice:
	case UnknownDevice:
		// BcdDeviceUnknownData : BcdDeviceData {
		//   Data : byte[]
		// }
	default:
		//NOT SUPPORTED!
		break;
	}
	return nullptr;
}

string BCD::GetFilePath(BcdObject ^o, string name) {
	Init();

	string path = nullptr;
	string device = BCD::GetDevicePath(o, BcdLibraryDevice_ApplicationDevice);
	if (device == nullptr) // Error, or not supported yet
		throw gcnew Exception(Msg_PathInTheBCDIsEitherInvalidOrNotYetSupported(name));

	path = BCD::GetString(o, BcdLibraryString_ApplicationPath);
	if (path == nullptr)
		path = name;
	else if (path->ToLower()->EndsWith(L"efi")) // NOT SUPPORTED YET!
		throw gcnew Exception(Msg_PathInTheBCDReferencesAnEFIWhichIsCurrentlyNotSupported(name));

	path = device->TrimEnd(L'\\') + L"\\" + path->TrimStart(L'\\');
	
	DISABLE_FS_REDIR();
	bool exists = Files::Exists(as_native(path));
	REVERT_FS_REDIR();

	if (!exists)
		throw gcnew Exception(Msg_PathInTheBCDIsInvalid(name));

	return path;
}

BcdObject ^BCD::GetBootLoader(string guid) { Init(); return gcnew BcdObject(scope, guid, storePath); }
