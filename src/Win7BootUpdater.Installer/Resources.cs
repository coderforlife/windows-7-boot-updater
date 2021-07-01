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
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Resources;
using System.Runtime.InteropServices;

namespace Win7BootUpdater.Installer
{
    internal static class Res
    {
        internal struct Type
        {
            //public static readonly Type CURSOR = 1, BITMAP = 2, ICON = 3;
            //public static readonly Type MENU = 4, DIALOG = 5, STRING = 6;
            //public static readonly Type FONTDIR = 7, FONT = 8, ACCELERATOR = 9;
            public static readonly Type RCDATA = 10;
            //public static readonly Type MESSAGETABLE = 11, GROUP_CURSOR = 12, GROUP_ICON = 14;
            //public static readonly Type VERSION = 16, DLGINCLUDE = 17, PLUGPLAY = 19;
            //public static readonly Type VXD = 20, ANICURSOR = 21, ANIICON = 22;
            //public static readonly Type HTML = 23, MANIFEST = 24;

            private IntPtr p;
            public Type(int i) { p = new IntPtr(i); }
            public Type(string s) { p = Marshal.StringToHGlobalUni(s); }
            public static implicit operator Type(int i) { return new Type(i); }
            public static implicit operator Type(string s) { return new Type(s); }
            public static implicit operator IntPtr(Type t) { return t.p; }
        }


        [DllImport("kernel32", SetLastError = true)] private static extern IntPtr GetModuleHandle(string lpModuleName);
        [DllImport("kernel32", SetLastError = true)] private static extern IntPtr LoadLibrary(string lpFileName);
        [DllImport("kernel32", SetLastError = true)] private static extern IntPtr FindResource(IntPtr hModule, IntPtr lpID, IntPtr lpType);
        [DllImport("kernel32", SetLastError = true)] private static extern IntPtr LoadResource(IntPtr hModule, IntPtr hResInfo);
        [DllImport("kernel32", SetLastError = true)] private static extern uint SizeofResource(IntPtr hModule, IntPtr hResInfo);

        private static IntPtr hMod = IntPtr.Zero;

        private static IntPtr GetModule()
        {
            if (hMod == IntPtr.Zero)
            {
                string file = Path.GetFileName(Program.GetExePath());
                hMod = GetModuleHandle(file);
                if (hMod == IntPtr.Zero) hMod = LoadLibrary(file);
                if (hMod == IntPtr.Zero) throw new Win32Exception();
            }
            return hMod;
        }

        public static Stream GetStream(Type type, Type id)
        {
            IntPtr hMod = GetModule();
            IntPtr hRes = FindResource(hMod, id, type);
            if (hRes == IntPtr.Zero) throw new Win32Exception();
            uint size = SizeofResource(hMod, hRes);
            IntPtr res = LoadResource(hMod, hRes);
            if (size == 0 || res == IntPtr.Zero) throw new Win32Exception();
            byte[] data = new byte[size];
            Marshal.Copy(res, data, 0, (int)size);
            return new MemoryStream(data);
        }

        public static Image GetImage(Type type, Type id)
        {
            Stream s = null;
            Image i = null;
            try { i = Bitmap.FromStream(s = GetStream(type, id)); }
            finally { if (s != null) s.Close(); }
            return i;
        }

        private static ResourceManager resources = new ResourceManager("Win7BootUpdater.Installer", Assembly.GetExecutingAssembly());

        public static Icon GetAppIcon() { return (Icon)resources.GetObject("app"); }
        public static string GetCompressedString(string name) {
            StreamReader s = new StreamReader(new GZipStream(resources.GetStream(name), CompressionMode.Decompress));
            string str = s.ReadToEnd();
            s.Close();
            return str;
        }
    }
}
