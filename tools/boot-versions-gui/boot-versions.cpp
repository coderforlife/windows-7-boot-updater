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

#include "stdafx.h"
#include "boot-versions.h"

#include "BCD.h"

#define BOOT_VERSIONS_IS_WRAPPED
#include "boot-versions.c"

using namespace Win7BootUpdater;

using namespace System;
using namespace System::IO;
using namespace System::Text;
using namespace System::Windows::Forms;

#define BUFFER_SIZE		10240
#define MAX_BIG_PATH	32768

static void AppendError(DWORD err, string msg, StringBuilder ^output) {
	output->Append(L"! Error While ")->Append(msg)->Append(L": ")->Append((gcnew System::ComponentModel::Win32Exception(err))->Message)->Append(L" (")->Append((uint32)err)->Append(L")\n");
}

static void GetSysInfo(StringBuilder ^output) {
	OSVERSIONINFOEX os;
	SYSTEM_INFO sys;
	DWORD product;
	string type, platform, prod;

	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO*)&os);
	GetNativeSystemInfo(&sys);
	GetProductInfo(6, 1, 0, 0, &product);

	switch (os.wProductType) {
		case VER_NT_WORKSTATION:		type = L"Workstation"; break;
		case VER_NT_SERVER:				type = L"Server"; break;
		case VER_NT_DOMAIN_CONTROLLER:	type = L"Domain Controller"; break;
		default: type = L"Unknown";
	}

	switch (sys.wProcessorArchitecture) {
		case PROCESSOR_ARCHITECTURE_INTEL:	platform = L"x86"; break;
		case PROCESSOR_ARCHITECTURE_AMD64:	platform = L"x64"; break;
		case PROCESSOR_ARCHITECTURE_IA64:	platform = L"Intel Itanium"; break;
		default: platform = L"Unknown";
	}

	switch (product) {
		case PRODUCT_STARTER:					prod = L"Starter"; break;
		case PRODUCT_STARTER_E:					prod = L"Starter E"; break; // Not supported
		case PRODUCT_STARTER_N:					prod = L"Starter N"; break;
		case PRODUCT_HOME_BASIC:				prod = L"Home Basic"; break;
		case PRODUCT_HOME_BASIC_E:				prod = L"Home Basic E"; break; // Not supported
		case PRODUCT_HOME_BASIC_N:				prod = L"Home Basic N"; break;
		case PRODUCT_HOME_PREMIUM:				prod = L"Home Premium"; break;
		case PRODUCT_HOME_PREMIUM_E:			prod = L"Home Premium E"; break; // Not supported
		case PRODUCT_HOME_PREMIUM_N:			prod = L"Home Premium N"; break;
		case PRODUCT_BUSINESS:					prod = L"Business"; break;
		case PRODUCT_BUSINESS_N:				prod = L"Business N"; break;
		case PRODUCT_PROFESSIONAL:				prod = L"Professional"; break;
		case PRODUCT_PROFESSIONAL_E:			prod = L"Professional E"; break; // Not supported
		case PRODUCT_PROFESSIONAL_N:			prod = L"Professional N"; break;
		case PRODUCT_ULTIMATE:					prod = L"Ultimate"; break;
		case PRODUCT_ULTIMATE_E:				prod = L"Ultimate E"; break; // Not supported
		case PRODUCT_ULTIMATE_N:				prod = L"Ultimate N"; break;
		case PRODUCT_ENTERPRISE:				prod = L"Enterprise"; break;
		case PRODUCT_ENTERPRISE_E:				prod = L"Enterprise E"; break; // Not supported
		case PRODUCT_ENTERPRISE_N:				prod = L"Enterprise N"; break;

		case PRODUCT_CLUSTER_SERVER:			prod = L"HPC Edition"; break;
		case PRODUCT_DATACENTER_SERVER:			prod = L"Server Datacenter (full)"; break;
		case PRODUCT_DATACENTER_SERVER_CORE:	prod = L"Server Datacenter (core)"; break;
		case PRODUCT_DATACENTER_SERVER_CORE_V:	prod = L"Server Datacenter without Hyper-V (core)"; break;
		case PRODUCT_DATACENTER_SERVER_V:		prod = L"Server Datacenter without Hyper-V (full)"; break;
		case PRODUCT_ENTERPRISE_SERVER:			prod = L"Server Enterprise (full)"; break;
		case PRODUCT_ENTERPRISE_SERVER_CORE:	prod = L"Server Enterprise (core)"; break;
		case PRODUCT_ENTERPRISE_SERVER_CORE_V:	prod = L"Server Enterprise without Hyper-V (core)"; break;
		case PRODUCT_ENTERPRISE_SERVER_IA64:	prod = L"Server Enterprise for Itanium-based Systems"; break;
		case PRODUCT_ENTERPRISE_SERVER_V:		prod = L"Server Enterprise without Hyper-V (full)"; break;
		case PRODUCT_HYPERV:					prod = L"Microsoft Hyper-V Server"; break;
		case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:	prod = L"Windows Essential Business Server Management Server"; break;
		case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:	prod = L"Windows Essential Business Server Messaging Server"; break;
		case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:	prod = L"Windows Essential Business Server Security Server"; break;
		case PRODUCT_SERVER_FOR_SMALLBUSINESS:	prod = L"Windows Server 2008 for Windows Essential Server Solutions"; break;
		case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:prod = L"Windows Server 2008 without Hyper-V for Windows Essential Server Solutions"; break;
		case PRODUCT_SERVER_FOUNDATION:			prod = L"Server Foundation"; break;
		case PRODUCT_SMALLBUSINESS_SERVER:		prod = L"Windows Small Business Server"; break;
		case PRODUCT_SOLUTION_EMBEDDEDSERVER:	prod = L"Windows MultiPoint Server"; break;
		case PRODUCT_STANDARD_SERVER:			prod = L"Server Standard (full)"; break;
		case PRODUCT_STANDARD_SERVER_CORE:		prod = L"Server Standard (core)"; break;
		case PRODUCT_STANDARD_SERVER_CORE_V:	prod = L"Server Standard without Hyper-V (core)"; break;
		case PRODUCT_STANDARD_SERVER_V:			prod = L"Server Standard without Hyper-V (full)"; break;
		case PRODUCT_STORAGE_ENTERPRISE_SERVER:	prod = L"Storage Server Enterprise"; break;
		case PRODUCT_STORAGE_EXPRESS_SERVER:	prod = L"Storage Server Express"; break;
		case PRODUCT_STORAGE_STANDARD_SERVER:	prod = L"Storage Server Standard"; break;
		case PRODUCT_STORAGE_WORKGROUP_SERVER:	prod = L"Storage Server Workgroup"; break;
		case PRODUCT_WEB_SERVER:				prod = L"Web Server (full)"; break;
		case PRODUCT_WEB_SERVER_CORE:			prod = L"Web Server (core)"; break;

		case PRODUCT_UNLICENSED:				prod = L"Unlicensed"; break;
		case PRODUCT_UNDEFINED:					prod = L"Undefined"; break;
		default: prod = L"Unknown";
	}

/*
PRODUCT_SB_SOLUTION_SERVER
PRODUCT_SB_SOLUTION_SERVER_EM
PRODUCT_SERVER_FOR_SB_SOLUTIONS
PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM
PRODUCT_STANDARD_SERVER_SOLUTIONS
PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE
PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT
PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL
PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC
PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC
*/

	output->AppendFormat(L"Windows Version: {0}.{1}.{2}\n", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
	output->AppendFormat(L"Service Pack:    {0}.{1}\n", os.wServicePackMajor, os.wServicePackMinor);
	output->AppendFormat(L"Product / Type:  {0} / {1}\n", prod, type);
	output->AppendFormat(L"Platform:        {0}\n\n", platform);
}

/* To work with printf statements:
	// Prepare the arguments to pass to the underlying code
	char buffer[BUFFER_SIZE];

	// Add a huge buffer to stdout so we can gather the output
	ZeroMemory(buffer, BUFFER_SIZE);
	setvbuf(stdout, buffer, _IOFBF, BUFFER_SIZE);

	DisplayVersion(as_native(name), get_ver, as_native(path));

	// Reset stdout buffering
	setbuf(stdout, NULL);

	output->Append(gcnew String(buffer));
*/

static void GetVersionInfo(string name, string error, string path, bool bootmgr, StringBuilder ^output) {
	output->Append(name)->Append(L"\n--------------------\n");
	if (error != nullptr)
		output->Append(error);

	ULONGLONG file, prod;
	WORD platform;
	BOOL success = (bootmgr ? &GetBootmgrVersion : &GetWinloadVersion)(as_native(path), &file, &prod, &platform);
	output->AppendFormat(L"Path:            {0}\n", path);
	if (!success) {
		output->Append(L"Failed to get information\n");
	} else {
		WORD f_major = (file >> 48) & 0xFFFF, f_minor = (file >> 32) & 0xFFFF, f_build = (file >> 16) & 0xFFFF, f_revis = file & 0xFFFF;
		WORD p_major = (prod >> 48) & 0xFFFF, p_minor = (prod >> 32) & 0xFFFF, p_build = (prod >> 16) & 0xFFFF, p_revis = prod & 0xFFFF;
		string p;
		switch (platform) {
			case IMAGE_FILE_MACHINE_I386:	p = L"x86"; break;
			case IMAGE_FILE_MACHINE_AMD64:	p = L"x64"; break;
			case IMAGE_FILE_MACHINE_IA64:	p = L"Intel Itanium"; break;
			default: p = L"Unknown";
		}
		output->AppendFormat(L"File Version:    {0}.{1}.{2}.{3}\n", f_major, f_minor, f_build, f_revis);
		output->AppendFormat(L"Product Version: {0}.{1}.{2}.{3}\n", p_major, p_minor, p_build, p_revis);
		output->AppendFormat(L"Platform:        {0}\n", p);
	}
	output->Append(L'\n');
}

static bool GetProgramOutput(string cmdLine, StringBuilder ^output) {
	bool retval = false;

	SECURITY_ATTRIBUTES sa;

	HANDLE hStdInRd, hStdInWr;
	HANDLE hStdOutRd, hStdOutWr;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	wchar_t szCmdLine[MAX_BIG_PATH] = { 0 };

	// Set the bInheritHandle flag so pipe handles are inherited.
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT / STDERR and STDIN
	if (!CreatePipe(&hStdOutRd, &hStdOutWr, &sa, BUFFER_SIZE) || !SetHandleInformation(hStdOutRd, HANDLE_FLAG_INHERIT, 0) ||
		!CreatePipe(&hStdInRd, &hStdInWr, &sa, BUFFER_SIZE) || !SetHandleInformation(hStdInWr, HANDLE_FLAG_INHERIT, 0))
	{
		AppendError(GetLastError(), L"Creating Pipes", output);
	} else {
		// Create the child process
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		si.hStdError = hStdOutWr;
		si.hStdOutput = hStdOutWr;
		si.hStdInput = hStdInRd;
		wcsncpy(szCmdLine, as_native(cmdLine), ARRAYSIZE(szCmdLine));
	
		if (!CreateProcess(NULL, szCmdLine, NULL, NULL, TRUE /*FALSE*/, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
			AppendError(GetLastError(), L"Creating Process", output);
		} else if (!CloseHandle(hStdInWr)) { // Close the pipe handle so the child process stops reading
			AppendError(GetLastError(), L"Ending Input", output);
		} else {
			bool active;
			DWORD dwExit = 0, dwRead = 0;
			CHAR buffer[BUFFER_SIZE];
			retval = true;

			// Read from the process until it exits
			do {
				active = GetExitCodeProcess(pi.hProcess, &dwExit) && dwExit == STILL_ACTIVE;
				if (!active) // The process has exited, close out write end so that ReadFile will not block
					CloseHandle(hStdOutWr);
				if (ReadFile(hStdOutRd, buffer, sizeof(buffer), &dwRead, NULL) && dwRead > 0)
					output->Append(gcnew String(buffer, 0, dwRead));
				SwitchToThread();
			} while (active);
		}
		
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	CloseHandle(hStdInWr);
	CloseHandle(hStdInRd);
	CloseHandle(hStdOutWr);
	CloseHandle(hStdOutRd);

	return retval;
}

string bootversionsgui::GetBootVersionInfo() {
	StringBuilder ^sb = gcnew StringBuilder();

	// Basic System Folder Paths
	string SysFolder = Environment::GetFolderPath(Environment::SpecialFolder::System);
	string WinFolder = Path::GetDirectoryName(SysFolder);
#ifndef _WIN64
	BOOL is32in64;
	if (IsWow64Process(GetCurrentProcess(), &is32in64) && is32in64)
		SysFolder = Path::Combine(WinFolder, L"sysnative"); // correct for WOW64 issues
#endif

	// Get Basic System Information
	sb->Append(L"System ===========================================\n");
	GetSysInfo(sb);

	// Get File Version Information
	string file, error = nullptr;
	sb->Append(L"Files ============================================\n\n");

	// Bootmgr
	try {
		error = nullptr;
		file = BCD::GetFilePath(BCD::Bootmgr, L"bootmgr");
	} catch (Exception ^ex) {
		error = L"Using fallback path ("+ex->Message+L")";
		file = Path::Combine(Path::GetPathRoot(SysFolder), L"bootmgr");
	}
	GetVersionInfo(L"bootmgr", error, file, true, sb);

	// Winload
	try {
		error = nullptr;
		file = BCD::GetFilePath(BCD::Current, L"winload.exe");
	} catch (Exception ^ex) {
		error = L"Using fallback path ("+ex->Message+L")\n";
		file = Path::Combine(SysFolder, L"winload.exe");
	}
	GetVersionInfo(L"winload", error, file, false, sb);

	// Winresume
	try {
		error = nullptr;
		file = BCD::GetFilePath(BCD::GetBootLoader(BCD::GetGUID(BCD::Current, BcdOSLoaderObject_AssociatedResumeObject)), L"winresume.exe");
	} catch (Exception ^ex) {
		error = L"Using fallback path ("+ex->Message+L")\n";
		file = Path::Combine(SysFolder, L"winresume.exe");
	}
	GetVersionInfo(L"winresume", error, file, false, sb);

	// Get the output of "bcdedit -enum all"
	sb->Append(L"Boot Configuration Data ==========================\n");
	try {
		string bcdstore = BCD::StorePath, bcdedit_cmd = Path::Combine(SysFolder, L"bcdedit.exe");
		bool def = String::IsNullOrEmpty(bcdstore);
		if (!def) bcdedit_cmd += L" /store "+bcdstore;
		bcdedit_cmd += " -enum all";

		sb->Append(L"BCD Store Path: ");
		sb->Append(def ? L"<system default>" : bcdstore)->Append(L'\n');

		if (!GetProgramOutput(bcdedit_cmd, sb))
			sb->Append("Unable to run 'bcdedit -enum all' command");
	} catch (Exception^) {
		sb->Append("Unable to load the BCD store");
	}

	return sb->Replace(L"\n", L"\r\n")->ToString();
}
