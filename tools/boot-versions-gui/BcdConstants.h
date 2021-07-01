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

// For more information on all the types, see: http://www.geoffchappell.com/viewer.htm?doc=notes/windows/boot/bcd/elements.htm
// For information about the BCD and EFI, see:
//   http://technet.microsoft.com/en-us/library/cc766223%28WS.10%29.aspx
//   http://www.computerperformance.co.uk/vista/vista_bcd.htm
// For more information on the 'DEVICE' objects, see: http://msdn.microsoft.com/en-us/library/aa362643%28VS.85%29.aspx

namespace Win7BootUpdater {
	ref struct Guids abstract sealed {
	public:
#pragma warning(push)
#pragma warning(disable:4693)
		literal string SystemStore = L"{9dea862c-5cdd-4e70-acc1-f32b344d4795}";
		literal string Current = L"{fa926493-6f1c-4193-a414-58f0b2456d1e}";
#pragma warning(pop)
	};

#define BCD_CLASS				0xF0000000
#define BCD_CLASS_LIBRARY		0x10000000
#define BCD_CLASS_APPLICATION	0x20000000
#define BCD_CLASS_DEVICE		0x30000000
#define BCD_CLASS_HIDDEN		0x40000000

#define BCD_FORMAT				0x0F000000
#define BCD_FORMAT_DEVICE		0x01000000 // name = "Device"	type = BcdDeviceData
#define BCD_FORMAT_STRING		0x02000000 // name = "String"	type = System.String
#define BCD_FORMAT_GUID			0x03000000 // name = "Id"		type = System.String
#define BCD_FORMAT_GUID_LIST	0x04000000 // name = "Ids"		type = System.String[] 
#define BCD_FORMAT_INTEGER		0x05000000 // name = "Integer"	type = System.String (can be parsed as System.UInt64)
#define BCD_FORMAT_BOOLEAN		0x06000000 // name = "Boolean"	type = System.Boolean or maybe bool
#define BCD_FORMAT_INTEGER_LIST	0x07000000 // name = "Integers"	type = System.String[] (each can be parsed as System.UInt64)

	enum BcdBootMgrElementTypes sealed {
		//BcdBootMgrObjectList_DisplayOrder        = 0x24000001,
		//BcdBootMgrObjectList_BootSequence        = 0x24000002,
		BcdBootMgrObject_DefaultObject           = 0x23000003,
		//BcdBootMgrInteger_Timeout                = 0x25000004,
		//BcdBootMgrBoolean_AttemptResume          = 0x26000005,
		BcdBootMgrObject_ResumeObject            = 0x23000006,
		//BcdBootMgrObjectList_ToolsDisplayOrder   = 0x24000010,
		//BcdBootMgrDevice_BcdDevice               = 0x21000022,
		//BcdBootMgrString_BcdFilePath             = 0x22000023 
	};

	/*enum BcdDeviceObjectElementTypes sealed {
		BcdDeviceObjectInteger_RamdiskImageOffset   = 0x35000001,
		BcdDeviceObjectInteger_TftpClientPort       = 0x35000002,
		BcdDeviceObjectInteger_SdiDevice            = 0x31000003,
		BcdDeviceObjectInteger_SdiPath              = 0x32000004,
		BcdDeviceObjectInteger_RamdiskImageLength   = 0x35000005 
	};*/

	enum BcdLibraryElementTypes sealed {
		BcdLibraryDevice_ApplicationDevice                   = 0x11000001,
		BcdLibraryString_ApplicationPath                     = 0x12000002,
		//BcdLibraryString_Description                         = 0x12000004,
		BcdLibraryString_PreferredLocale                     = 0x12000005,
		//BcdLibraryObjectList_InheritedObjects                = 0x14000006,
		//BcdLibraryInteger_TruncatePhysicalMemory             = 0x15000007,
		//BcdLibraryObjectList_RecoverySequence                = 0x14000008,
		//BcdLibraryBoolean_AutoRecoveryEnabled                = 0x16000009,
		//BcdLibraryIntegerList_BadMemoryList                  = 0x1700000a,
		//BcdLibraryBoolean_AllowBadMemoryAccess               = 0x1600000b,
		//BcdLibraryInteger_FirstMegabytePolicy                = 0x1500000c,
		//BcdLibraryBoolean_DebuggerEnabled                    = 0x16000010,
		//BcdLibraryInteger_DebuggerType                       = 0x15000011,
		//BcdLibraryInteger_SerialDebuggerPortAddress          = 0x15000012,
		//BcdLibraryInteger_SerialDebuggerPort                 = 0x15000013,
		//BcdLibraryInteger_SerialDebuggerBaudRate             = 0x15000014,
		//BcdLibraryInteger_1394DebuggerChannel                = 0x15000015,
		//BcdLibraryString_UsbDebuggerTargetName               = 0x12000016,
		//BcdLibraryBoolean_DebuggerIgnoreUsermodeExceptions   = 0x16000017,
		//BcdLibraryInteger_DebuggerStartPolicy                = 0x15000018,
		//BcdLibraryBoolean_EmsEnabled                         = 0x16000020,
		//BcdLibraryInteger_EmsPort                            = 0x15000022,
		//BcdLibraryInteger_EmsBaudRate                        = 0x15000023,
		//BcdLibraryString_LoadOptionsString                   = 0x12000030,
		//BcdLibraryBoolean_DisplayAdvancedOptions             = 0x16000040,
		//BcdLibraryBoolean_DisplayOptionsEdit                 = 0x16000041,
		//BcdLibraryBoolean_GraphicsModeDisabled               = 0x16000046,
		//BcdLibraryInteger_ConfigAccessPolicy                 = 0x15000047,
		BcdLibraryBoolean_DisableIntegrityChecks			 = 0x16000048,
		BcdLibraryBoolean_AllowPrereleaseSignatures          = 0x16000049 
	};

	/*enum BcdMemDiagElementTypes sealed {
		BcdMemDiagInteger_PassCount      = 0x25000001,
		BcdMemDiagInteger_FailureCount   = 0x25000003 
	};*/

	enum BcdOSLoaderElementTypes sealed {
		//BcdOSLoaderDevice_OSDevice                       = 0x21000001,
		//BcdOSLoaderString_SystemRoot                     = 0x22000002,
		BcdOSLoaderObject_AssociatedResumeObject         = 0x23000003,
		//BcdOSLoaderBoolean_DetectKernelAndHal            = 0x26000010,
		//BcdOSLoaderString_KernelPath                     = 0x22000011,
		//BcdOSLoaderString_HalPath                        = 0x22000012,
		//BcdOSLoaderString_DbgTransportPath               = 0x22000013,
		//BcdOSLoaderInteger_NxPolicy                      = 0x25000020,
		//BcdOSLoaderInteger_PAEPolicy                     = 0x25000021,
		//BcdOSLoaderBoolean_WinPEMode                     = 0x26000022,
		//BcdOSLoaderBoolean_DisableCrashAutoReboot        = 0x26000024,
		//BcdOSLoaderBoolean_UseLastGoodSettings           = 0x26000025,
		//BcdOSLoaderBoolean_AllowPrereleaseSignatures     = 0x26000027,
		//BcdOSLoaderBoolean_NoLowMemory                   = 0x26000030,
		//BcdOSLoaderInteger_RemoveMemory                  = 0x25000031,
		//BcdOSLoaderInteger_IncreaseUserVa                = 0x25000032,
		//BcdOSLoaderBoolean_UseVgaDriver                  = 0x26000040,
		//BcdOSLoaderBoolean_DisableBootDisplay            = 0x26000041,
		//BcdOSLoaderBoolean_DisableVesaBios               = 0x26000042,
		//BcdOSLoaderInteger_ClusterModeAddressing         = 0x25000050,
		//BcdOSLoaderBoolean_UsePhysicalDestination        = 0x26000051,
		//BcdOSLoaderInteger_RestrictApicCluster           = 0x25000052,
		//BcdOSLoaderBoolean_UseBootProcessorOnly          = 0x26000060,
		//BcdOSLoaderInteger_NumberOfProcessors            = 0x25000061,
		//BcdOSLoaderBoolean_ForceMaximumProcessors        = 0x26000062,
		//BcdOSLoaderBoolean_ProcessorConfigurationFlags   = 0x25000063,
		//BcdOSLoaderInteger_UseFirmwarePciSettings        = 0x26000070,
		//BcdOSLoaderInteger_MsiPolicy                     = 0x26000071,
		//BcdOSLoaderInteger_SafeBoot                      = 0x25000080,
		//BcdOSLoaderBoolean_SafeBootAlternateShell        = 0x26000081,
		//BcdOSLoaderBoolean_BootLogInitialization         = 0x26000090,
		//BcdOSLoaderBoolean_VerboseObjectLoadMode         = 0x26000091,
		//BcdOSLoaderBoolean_KernelDebuggerEnabled         = 0x260000a0,
		//BcdOSLoaderBoolean_DebuggerHalBreakpoint         = 0x260000a1,
		//BcdOSLoaderBoolean_EmsEnabled                    = 0x260000b0,
		//BcdOSLoaderInteger_DriverLoadFailurePolicy       = 0x250000c1,
		//BcdOSLoaderInteger_BootStatusPolicy              = 0x250000E0 
	};
	
	/*enum BcdLibrary_DebuggerType sealed {
		DebuggerSerial   = 0,
		Debugger1394     = 1,
		DebuggerUsb      = 2 
	};

	enum BcdLibrary_SafeBoot sealed {
		SafemodeMinimal    = 0,
		SafemodeNetwork    = 1,
		SafemodeDsRepair   = 2 
	};

	enum BcdOSLoader_NxPolicy sealed {
		NxPolicyOptIn       = 0,
		NxPolicyOptOut      = 1,
		NxPolicyAlwaysOff   = 2,
		NxPolicyAlwaysOn    = 3 
	};

	enum BcdOSLoader_PAEPolicy sealed {
		PaePolicyDefault        = 0,
		PaePolicyForceEnable    = 1,
		PaePolicyForceDisable   = 2 
	};*/
}