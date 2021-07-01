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

#include "FileSecurity.h"

#ifdef __cplusplus_cli
#pragma unmanaged
#endif

#include <Aclapi.h>
#pragma comment(lib, "Advapi32.lib")

//#include <Sddl.h>		// for ConvertSidToStringSid (debugging)
//#define MAX_NAME 256  // (debugging)

using namespace Win7BootUpdater;

PSID sid = NULL;
PACL acl = NULL;

typedef struct _SEC_INFO {
	bool readOnly;
	//PSID owner;
	//PACL dacl;
	DWORD size;
	PSECURITY_DESCRIPTOR sec;
} SEC_INFO;

void FileSecurity::Init() {
	// Dummy function call forces dependencies to load for WOW64 stuff (this depends on many delayed imports)
	GetNamedSecurityInfo(L".", SE_FILE_OBJECT, 0, NULL, NULL, NULL, NULL, NULL);
}

/*static void sidString(PSID sid) {
	LPWSTR sidStr = NULL;
	ConvertSidToStringSid(sid, &sidStr);
	LocalFree(sidStr);
}*/

/*static bool isGroupSid(PSID sid) {
	bool retval = false;
	SID_NAME_USE use;
	DWORD size = MAX_NAME;
	TCHAR name[MAX_NAME], domain[MAX_NAME];
	if (LookupAccountSid(NULL, sid, name, &size, domain, &size, &use) || GetLastError() == ERROR_NONE_MAPPED) {
		retval = use == SidTypeGroup || use == SidTypeWellKnownGroup;
	}
	return retval;
}*/

/*static bool isTrustedInstaller(PSID sid) {
	bool retval = false;
	SID_NAME_USE use;
	DWORD size = MAX_NAME; // longest possible SID byte string is 68 long, so fits in MAX_NAME
	PSID tiSid = LocalAlloc(LMEM_ZEROINIT, size);
	TCHAR domain[MAX_NAME];
	if (LookupAccountName(NULL, TEXT("NT SERVICE\\TrustedInstaller"), tiSid, &size, domain, &size, &use) || GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		retval = EqualSid(sid, tiSid) == TRUE;
	}
	LocalFree(tiSid);
	return retval;
}*/

void *FileSecurity::Get(LPCWSTR path) {
	DWORD size;
	if (!GetFileSecurity(path, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, NULL, 0, &size) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) { return NULL; }
	SEC_INFO *si = (SEC_INFO*)LocalAlloc(LMEM_ZEROINIT, sizeof(SEC_INFO));
	si->sec = LocalAlloc(LMEM_ZEROINIT, si->size = size);
	if (!GetFileSecurity(path, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, si->sec, size, &size)) {
	//if (GetNamedSecurityInfo(path, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, &si->owner, NULL, &si->dacl, NULL, &si->sec) != ERROR_SUCCESS) {
		si = (SEC_INFO*)FreeData(si);
	} else {
		DWORD attrib = GetFileAttributes(path);
		si->readOnly = attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_READONLY);
	}
	return si;
}

bool FileSecurity::Restore(LPCWSTR path, void *sec) {
	SEC_INFO *si = (SEC_INFO*)sec;

	DWORD attrib = GetFileAttributes(path);
	if (attrib != INVALID_FILE_ATTRIBUTES) {
		SetFileAttributes(path, si->readOnly ? (attrib|FILE_ATTRIBUTE_READONLY) : (attrib&!FILE_ATTRIBUTE_READONLY));
	}

	//SetNamedSecurityInfo(path, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, si->owner, NULL, si->dacl, NULL) == ERROR_SUCCESS
	return SetFileSecurity(path, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, si->sec) == TRUE;
}

void *FileSecurity::DuplicateData(void *sec) {
	SEC_INFO *si = (SEC_INFO*)sec, *x = NULL;
	if (si) {
		x = (SEC_INFO*)LocalAlloc(LMEM_ZEROINIT, sizeof(SEC_INFO));
		x->sec = LocalAlloc(LMEM_ZEROINIT, x->size = si->size);
		memcpy(x->sec, si->sec, si->size);
		x->readOnly = si->readOnly;
	}
	return x;
}

void *FileSecurity::FreeData(void *sec) {
	SEC_INFO *si = (SEC_INFO*)sec;
	if (si) {
		if (si->sec) LocalFree(si->sec);
		LocalFree(si);
	}
	return NULL;
}

// Get the SID of the user running this program (not necessarily the logged on user)
// Returns NULL on error or the PSID on success
static PSID GetCurrentSID() {
	HANDLE token;
	PSID sid = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
		DWORD size = 0;
		// Get size of the TOKEN_USER data
		GetTokenInformation(token, TokenUser, NULL, 0, &size);
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			TOKEN_USER *user = (TOKEN_USER*)LocalAlloc(0, size);
			if (GetTokenInformation(token, TokenUser, user, size, &size)) {
				// We need to copy the SID because the TOKEN_USER needs to be freed
				size = GetLengthSid(user->User.Sid);
				sid = (PSID)LocalAlloc(LMEM_ZEROINIT, size);
				if (!CopySid(size, sid, user->User.Sid)) {
					sid = LocalFree(sid);
				}
			}
			LocalFree(user);
		}
		CloseHandle(token);
	}
	return sid;
}

// Enables the current user to have full access to a file (including being the owner)
bool FileSecurity::CurrentUserFullAccess(LPCWSTR path) {
	if (!sid) {
		sid = GetCurrentSID();
		if (!sid) return false;
	}
	
	DWORD err;
	if (!acl) {
		// Explicit Access: All Access
		EXPLICIT_ACCESS ea[1];
		ZeroMemory(&ea, 1*sizeof(EXPLICIT_ACCESS));
		ea[0].grfAccessPermissions = GENERIC_ALL;
		ea[0].grfAccessMode = SET_ACCESS;
		ea[0].grfInheritance = SUB_OBJECTS_ONLY_INHERIT | NO_PROPAGATE_INHERIT_ACE;
		ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
		ea[0].Trustee.ptstrName = (LPTSTR)sid;

		// Create the ACL
		if ((err = SetEntriesInAcl(1, ea, NULL, &acl)) != ERROR_SUCCESS) {
			return false;
		}
	}

	// Set the security
	// We cannot set both properties at the same time since if we don't own it we may not be able to change the DACL
	if (SetNamedSecurityInfo((LPWSTR)path, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, sid, NULL, NULL, NULL) == ERROR_SUCCESS &&
		SetNamedSecurityInfo((LPWSTR)path, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, acl,  NULL) == ERROR_SUCCESS) {
			// Remove the read-only attribute
			DWORD attrib = GetFileAttributes(path);
			if (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_READONLY)) {
				SetFileAttributes(path, attrib&!FILE_ATTRIBUTE_READONLY);
			}
			return true;
	}

	return false;
}

// Adds the current user to have full access to a file (including being the owner)
bool FileSecurity::AddCurrentUserAccess(LPCWSTR path) {
	if (!sid) {
		sid = GetCurrentSID();
		if (!sid) {
			return false;
		}
	}

	DWORD err;
	PACL old_acl = NULL, acl = NULL;
	PSECURITY_DESCRIPTOR sec = NULL;
	bool retval = false;

	// Get the old ACL
	if ((err = GetNamedSecurityInfo(path, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &old_acl, NULL, &sec)) != ERROR_SUCCESS) {
		return false;
	}

	// Explicit Access: All Access
	EXPLICIT_ACCESS ea[1];
	ZeroMemory(&ea, 1*sizeof(EXPLICIT_ACCESS));
	ea[0].grfAccessPermissions = FILE_GENERIC_WRITE | FILE_DELETE_CHILD; //GENERIC_ALL; // STANDARD_RIGHTS_ALL
	ea[0].grfAccessMode = GRANT_ACCESS;
	ea[0].grfInheritance = 0;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
	ea[0].Trustee.ptstrName = L"CURRENT_USER";

	// Change the ACL
	if (SetEntriesInAcl(1, ea, old_acl, &acl) != ERROR_SUCCESS) {
		LocalFree(sec);
		return false;
	}

	// Set the security
	// We cannot set both properties at the same time since if we don't own it we may not be able to change the DACL
	if (SetNamedSecurityInfo((LPWSTR)path, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, sid, NULL, NULL, NULL) == ERROR_SUCCESS) {
		if (SetNamedSecurityInfo((LPWSTR)path, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, acl,  NULL) == ERROR_SUCCESS) {
			// Remove the read-only attribute
			DWORD attrib = GetFileAttributes(path);
			if (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_READONLY)) {
				SetFileAttributes(path, attrib&!FILE_ATTRIBUTE_READONLY);
			}
			retval = true;
		}
	}

	LocalFree(sec);
	LocalFree(acl);
	return retval;
}

void FileSecurity::Cleanup() {
	sid = LocalFree(sid);
	acl = (PACL)LocalFree(acl);
}
