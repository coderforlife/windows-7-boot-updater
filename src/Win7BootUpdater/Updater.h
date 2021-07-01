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

#include "BootSkin.h"

namespace Win7BootUpdater {
	/// <remarks>The static class that is the main gateway into the updating of system files.</remarks>
	PUBLIC ref class Updater abstract sealed {
	public:
		/// <summary>Checks if the current system is Windows Vista or newer</summary>
		/// <returns>True if the system is Windows Vista or newer, false otherwise</returns>
		static bool IsWindowsVistaOrNewer();
		/// <summary>Checks if the current system is Windows 7 or newer</summary>
		/// <returns>True if the system is Windows 7 or newer, false otherwise</returns>
		static bool IsWindows7OrNewer();
		/// <summary>Checks if the current system is 64 bit Windows, regardless of the binary format</summary>
		/// <returns>True if the system is 64-bit, false if it is 32-bit</returns>
		static bool Is64BitWindows();

		/// <summary>Gets the preferred locale, either from the BCD information or from the current Windows language</summary>
		/// <returns>The preferred locale name (e.g. en-US, de-DE, ...)</returns>
		static string GetPreferredLocale();

		/// <summary>Gets the version of a patch, possibly downloading a newer version from the Internet</summary>
		/// <param name="name">The name of the patch (bootmgr, winload, or winresume)</param>
		/// <returns>A string representing the version of the patch file</returns>
		static string GetPatchVersion(string name);

		/// <summary>Causes a patch to begin loading in a separate thread, possibly downloading a newer version from the Internet</summary>
		/// <param name="name">The name of the patch (bootmgr, winload, or winresume)</param>
		static void PreloadPatch(string name);

		/// <summary>Queries the Internet for the latest available version of the program</summary>
		/// <returns>null if the current version is as new as the latest available version, or a string representing the version of the newest version</returns>
		//static System::Version ^GetLatestAvailableVersion();

		/// <summary>Disables filesystem redirection for the current thread</summary>
		static void DisableFSRedirection();

		/// <summary>Reverts filesystem redirection for the current thread</summary>
		static void RevertFSRedirection();

		/// <summary>Sets a file to be deleted after the program exits</summary>
		/// <param name="path">The path of the file to delete after the program exists, typically the current program</param>
		/// <returns>Returns true if the delete was successfully setup, false otherwise</returns>
		static bool DeleteAfterExit(string path);

#pragma warning(push)
#pragma warning(disable:4693)
		/// <summary>The total amount of progress that updating uses</summary>
		literal int TotalProgress = 2*(18 + 7 + 13) + 7 + 7;
#pragma warning(pop)

		/// <summary>Initialize the updater library. This should be called when the program starts up.</summary>
		static void Init();
		/// <summary>Enables many privileges, including the take ownership (required to modify system files) and debug privileges. This should be called when the program starts up.</summary>
		/// <returns>True if enabled successfully, false otherwise. If it returns false, it may be because the process is not running as administrator.</returns>
		static bool EnablePrivileges();
		/// <summary>Cleans up the updater library. This should be called when the program is about to exit.</summary>
		static void Cleanup();

		/// <summary>Updates the system files according to the boot skin given</summary>
		/// <param name="bs">The boot skin</param>
		/// <param name="bootres">The path to bootres.dll</param>
		/// <param name="winload">The path to winload.exe</param>
		/// <param name="winloadMui">The path to winload.exe.mui</param>
		/// <param name="winresume">The path to winresume.exe</param>
		/// <param name="winresumeMui">The path to winresume.exe.mui</param>
		/// <param name="bootmgr">The path to bootmgr</param>
		/// <param name="backup">True if backups should be created before modifying the files</param>
		/// <returns>The error code. If it is 0 there is no error, otherwise pass it to <see cref="UI::ShowError(string,string,uint,string)" /> to process it.</returns>
		static uint Update(Win7BootUpdater::BootSkin ^bs, string bootres, string winload, string winloadMui, string winresume, string winresumeMui, string bootmgr, bool backup /*, array<string> ^%modifiedPaths*/);

		/// <summary>Restores modified files</summary>
		/// <param name="files">The list of full paths of files to restore</param>
		/// <returns>The list of full paths of the files that were restored</returns>
		static array<string> ^Restore(... array<string> ^files);

		/// <summary>Downloads the PDB file for the given executable</summary>
		/// <param name="file">The full path of executable to download PDB for (this does not support bootmgr at the moment)</param>
		/// <returns>The relative path of the PDB that is downloaded or null if it failed</returns>
		static string Updater::DownloadPDB(string file);
		
		/// <summary>Creates a boot skin installer from an installer base</summary>
		/// <param name="bs">The boot skin to embed into the installer</param>
		/// <param name="desc">The XML description to embed into the installer</param>
		/// <param name="image">The image data to embed into the installer</param>
		/// <param name="installerBase">The base installer into which the resources will be embedded into</param>
		/// <param name="outputFile">The file to save the resulting installer as</param>
		/// <returns>null if there was no error, otherwise it is string containing an error message</returns>
		static string CreateInstaller(Win7BootUpdater::BootSkin ^bs, System::Xml::XmlDocument ^desc, System::Drawing::Image ^image, array<byte> ^installerBase, string outputFile);
	};
}
