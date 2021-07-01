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
using System.Drawing;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Resources;

namespace Win7BootUpdater.GUI
{
    internal static class Res
    {
        public static ResourceManager resources = new ResourceManager("Win7BootUpdater.GUI", Assembly.GetExecutingAssembly());
        //public static object Get(string name) { return resources.GetObject(name); }
        public static byte[] GetData(string name) { return (byte[])resources.GetObject(name); }
        public static string GetString(string name) { return (string)resources.GetObject(name); }
        public static string GetCompressedString(string name) {
            StreamReader s = new StreamReader(new GZipStream(resources.GetStream(name), CompressionMode.Decompress));
            string str = s.ReadToEnd();
            s.Close();
            return str;
        }
        public static byte[] GetCompressedBytes(string name)
        {
            Stream input = new GZipStream(resources.GetStream(name), CompressionMode.Decompress);
            MemoryStream output = new MemoryStream();
            byte[] buf = new byte[102400]; // 100kb at a time
            int read;
            while ((read = input.Read(buf, 0, 102400)) > 0) output.Write(buf, 0, read);
            return output.ToArray();
        }
        public static Icon GetAppIcon() { return (Icon)resources.GetObject("app"); }
        public static Image GetImage(string name)
        {
            // Images are stored as streams since storing the activity.bmp as a
            // Bitmap doubles the size of the program...
            Stream data = resources.GetStream(name);
            Bitmap b = new Bitmap(data);
            data.Close();
            return b;
        }
    }
}
