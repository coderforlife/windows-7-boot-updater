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

namespace Win7BootUpdater.Patches
{
    class Program
    {
        static int Main(string[] args)
        {
            if (args.Length != 1 && args.Length != 2)
            {
                Console.WriteLine("Usage: patch-compiler input.xml [directory | output.patch]");
                return 1;
            }

            string IN = args[0], _out = Path.GetFileNameWithoutExtension(IN) + ".patch";
            string OUT = args.Length == 2 ? (Directory.Exists(args[1]) ? Path.Combine(args[1], _out) : args[1]) : _out;

            PatchFile f = new PatchFile(IN);
            f.Write(new FileStream(OUT, FileMode.Create, FileAccess.Write));
            return 0;
        }
    }
}
