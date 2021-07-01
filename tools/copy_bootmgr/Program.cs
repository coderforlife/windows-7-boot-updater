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

using Microsoft.Win32.SafeHandles;
using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Text;

namespace copy_bootmgr
{
    class Program
    {
        // Enumerating volume names
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern FindVolumeHandle FindFirstVolume(StringBuilder lpszVolumeName, uint cchBufferLength);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern bool FindNextVolume(FindVolumeHandle hFindVolume, StringBuilder lpszVolumeName, uint cchBufferLength);
        class FindVolumeHandle : SafeHandleZeroOrMinusOneIsInvalid
        {
            [DllImport("kernel32.dll", SetLastError = true)]
            private static extern bool FindVolumeClose(IntPtr hFindVolume);
            private FindVolumeHandle() : base(true) { }
            public FindVolumeHandle(IntPtr preexistingHandle, bool ownsHandle) : base(ownsHandle) { SetHandle(preexistingHandle); }
            protected override bool ReleaseHandle() { return FindVolumeClose(handle); }
        }

        // Retrieving volume information
        enum DriveType : uint { Unknown=0, Error=1, Removable=2, Fixed=3, Remote=4, CDROM=5, RAMDisk=6 }
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern DriveType GetDriveType(string lpRootPathName);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        extern static bool GetVolumeInformation(string lpRootPathName, StringBuilder lpVolumeNameBuffer,
            uint nVolumeNameSize, UIntPtr lpVolumeSerialNumber, UIntPtr lpMaximumComponentLength,
            UIntPtr lpFileSystemFlags, StringBuilder lpFileSystemNameBuffer, uint nFileSystemNameSize);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern bool GetDiskFreeSpaceEx(string lpDirectoryName, out ulong lpFreeBytesAvailable,
            out ulong lpTotalNumberOfBytes, out ulong lpTotalNumberOfFreeBytes);


        // File Utility Functions that support \\?\-style paths
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern uint GetFileAttributes(string lpFileName);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern bool SetFileAttributes(string lpFileName, uint dwFileAttributes);
        const uint INVALID_FILE_ATTRIBUTES = unchecked((uint)-1);
        const uint FILE_ATTRIBUTE_READONLY = 0x01;
        const uint FILE_ATTRIBUTE_HIDDEN = 0x02;
        const uint FILE_ATTRIBUTE_SYSTEM = 0x04;
        const uint FILE_ATTRIBUTE_DIRECTORY = 0x10;
        const uint FILE_ATTRIBUTE_NORMAL = 0x80;
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern bool CopyFile(string lpExistingFileName, string lpNewFileName, bool bFailIfExists);

        static int Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.Error.WriteLine("Need to include the destination file to copy to");
                return 0;
            }
            string output = args[0];
            string path = findBootmgr();
            if (path == null)
            {
                Console.Error.WriteLine("Unable to find bootmgr");
                return 1;
            }
            Console.Out.WriteLine(path + " => " + output);
            if (!CopyFile(path, output, false))
            {
                Console.Error.WriteLine("Unable to copy bootmgr to output: " + Marshal.GetLastWin32Error().ToString("X"));
                return 2;
            }
            SetFileAttributes(output, FILE_ATTRIBUTE_SYSTEM);
            return 0;
        }

        /* Finds a path to the bootmgr file. */
        static string findBootmgr()
        {
            const uint buf_len = 1024;
            StringBuilder buf = new StringBuilder((int)buf_len, (int)buf_len);
            using (FindVolumeHandle h = FindFirstVolume(buf, buf_len)) {
                if (h.IsInvalid) { throw new Win32Exception(Marshal.GetLastWin32Error()); }
                do
                {
                    string volume = buf.ToString(), path = volume + "bootmgr";
                    uint attribs = GetFileAttributes(path);
                    if (GetDriveType(volume) == DriveType.Fixed && attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY) == 0)
                    {
                        return path;
                        /*if (!GetVolumeInformation(volume, buf, buf_len, UIntPtr.Zero, UIntPtr.Zero, UIntPtr.Zero, null, 0))
                        {
                            continue;
                        }
                        string label = buf.ToString();
                        if (label == "System Reserved")
                        {
                            ulong avail, total, free;
                            if (!GetDiskFreeSpaceEx(volume, out avail, out total, out free)) { continue; }
                            if (total <= 512*1024*1024) { return path; }
                        }*/
                    }
                } while (FindNextVolume(h, buf, buf_len));
            }
            return null;
        }
    }
}
