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

using System;
using System.IO;

using Microsoft.Win32;

namespace Win7BootUpdater.Installer
{
    internal static class Registry
    {
        private static RegistryKey UninstallKey = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(@"Software\Microsoft\Windows\CurrentVersion\Uninstall", true);
        private const string ProductCode = "Windows 7 Boot Skin";
        private const string Win7BootUpdaterLink = "http://coderforlife.com/projects/win7boot/";
        private const string ModifiedFiles = "ModifiedFiles", SkinName = "SkinName", SkinAuthor = "SkinAuthor";

        public static string QuotePath(string p) { p = p.Trim(); return p.Contains(" ") ? '"' + p + '"' : p; }
        public static string UnquotePath(string p) { p = p.Trim(); return (p.StartsWith("\"") && p.EndsWith("\"")) ? p.Substring(1, p.Length - 2).Trim() : p; }

        private static uint CompleteVersion(System.Version v) { return (uint)((v.Major << 24) | (v.Minor << 16) | (v.Revision << 8) | (v.Build)); }
        public static void AddUninstallInformation(string skinName, string skinAuthor, string skinURL, string uninstaller, string[] modifiedFiles)
        {
            RegistryKey k = UninstallKey.CreateSubKey(ProductCode);
            k.SetValue("DisplayName", UI.GetMessage(Msg.Windows7BootSkin) + " - " + skinName);
            k.SetValue("Publisher", "Coder for Life - " + UI.GetMessage(Msg.SkinAuthor) + " " + skinAuthor);
            k.SetValue(SkinName, skinName);
            k.SetValue(SkinAuthor, skinAuthor);
            //k.SetValue("Comments", ...);
            k.SetValue("Readme", Win7BootUpdaterLink);
            k.SetValue("URLInfoAbout", skinURL);
            //k.SetValue("HelpLink", ...);
            //k.SetValue("URLUpdateInfo", ...);

            k.SetValue("UninstallPath", QuotePath(uninstaller) + " /u");
            k.SetValue("UninstallString", QuotePath(uninstaller) + " /u");
            k.SetValue("DisplayIcon", uninstaller); // QuotePath?
            k.SetValue("InstallLocation", QuotePath(Path.GetDirectoryName(uninstaller)));

            k.SetValue("DisplayVersion", Version.Full);
            k.SetValue("InstallDate", DateTime.Today.ToString("yyyyMMdd"));
            k.SetValue("InstallSource", QuotePath(Path.GetDirectoryName(Program.GetExePath())));

            k.SetValue("VersionMajor", Version.Ver.Major, RegistryValueKind.DWord);
            k.SetValue("VersionMinor", Version.Ver.Minor, RegistryValueKind.DWord);
            k.SetValue("NoModify", 1, RegistryValueKind.DWord);
            k.SetValue("NoRepair", 1, RegistryValueKind.DWord);
            //k.SetValue("SystemComponent", 1, RegistryValueKind.DWord); // This makes it not show up in 'Programs and Features'
            k.SetValue("Version", CompleteVersion(Version.Ver), RegistryValueKind.DWord);

            RegistryKey mod = k.CreateSubKey(ModifiedFiles);
            for (int i = 0; i < modifiedFiles.Length; ++i)
                mod.SetValue(i.ToString(), modifiedFiles[i]);
        }
        public static void RemoveUninstallInformation() { try { UninstallKey.DeleteSubKeyTree(ProductCode); } catch (ArgumentException) { } }
        public static bool AlreadyInstalled() { return UninstallKey.OpenSubKey(ProductCode) != null; }
        public static string GetInstalledSkinName() { RegistryKey k = UninstallKey.OpenSubKey(ProductCode); return (k == null) ? null : (string)k.GetValue(SkinName); }
        public static string GetInstalledSkinAuthor() { RegistryKey k = UninstallKey.OpenSubKey(ProductCode); return (k == null) ? null : (string)k.GetValue(SkinAuthor); }
        public static string GetUninstallerPath()
        {
            RegistryKey k = UninstallKey.OpenSubKey(ProductCode);
            if (k == null) return null;
            string uninstaller = (string)k.GetValue("UninstallPath");
            if (uninstaller != null && uninstaller.EndsWith(" /u"))
                uninstaller = UnquotePath(uninstaller.Remove(uninstaller.Length - 3));
            return uninstaller;
        }
        public static string[] GetInstalledModifiedFiles()
        {
            RegistryKey k = UninstallKey.OpenSubKey(ProductCode);
            if (k == null) return null;
            RegistryKey mod = k.OpenSubKey(ModifiedFiles);
            if (mod == null) return null;
            string[] names = mod.GetValueNames(), files = new string[names.Length];
            for (int i = 0; i < names.Length; ++i)
                files[i] = (string)mod.GetValue(names[i]);
            return files;
        }
        public static void RemoveModifiedFile(string path)
        {
            RegistryKey k = UninstallKey.OpenSubKey(ProductCode);
            if (k == null) return;
            RegistryKey mod = k.OpenSubKey(ModifiedFiles);
            if (mod == null) return;

            string[] names = mod.GetValueNames();
            for (int i = 0; i < names.Length; ++i)
                if ((string)mod.GetValue(names[i]) == path)
                {
                    mod.DeleteValue(names[i]);
                    //break;
                }
        }
    }
}
