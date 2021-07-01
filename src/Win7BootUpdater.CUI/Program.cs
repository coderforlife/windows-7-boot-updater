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
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

using Win7BootUpdater;

namespace Win7BootUpdater.CUI
{
    internal static class ConsoleEx
    {
        public static bool IsOutputRedirected
        {
            get { return FileType.Char != GetFileType(GetStdHandle(StdHandle.Stdout)); }
        }
        public static bool IsInputRedirected
        {
            get { return FileType.Char != GetFileType(GetStdHandle(StdHandle.Stdin)); }
        }
        public static bool IsErrorRedirected
        {
            get { return FileType.Char != GetFileType(GetStdHandle(StdHandle.Stderr)); }
        }

        // P/Invoke:
        private enum FileType { Unknown, Disk, Char, Pipe };
        private enum StdHandle { Stdin = -10, Stdout = -11, Stderr = -12 };
        [DllImport("kernel32.dll")]
        private static extern FileType GetFileType(IntPtr hdl);
        [DllImport("kernel32.dll")]
        private static extern IntPtr GetStdHandle(StdHandle std);
    }

    internal static class Program
    {
        #region General Output
        static void About() {
            Console.WriteLine();
            Console.WriteLine(UI.GetMessage(Msg.Windows7BootUpdater) + " - " + UI.GetMessage(Msg.CommandLineVersion));
            Console.WriteLine(UI.GetMessage(Msg.Version, Version.Desc));
            Console.WriteLine(UI.GetMessage(Msg.ByJeffBush) + " - jeff@coderforlife.com");
            Console.WriteLine();
            Console.WriteLine(UI.GetMessage(Msg.VisitForMoreInformation));
            Console.WriteLine(UI.GetMessage(Msg.SeeForLicensingOfThisProduct));
            Console.WriteLine();
        }

        static List<string> Wrap(string s, int width)
        {
            s = s.Trim();
            List<string> lines = new List<string>();
            while (s.Length > width)
            {
                string S = s.Substring(0, width);
                int space = S.LastIndexOf(' ');
                if (space == -1)
                {
                    space = s.IndexOf(' ');
                    if (space == -1) space = s.Length;
                }
                lines.Add(s.Remove(space));
                s = s.Substring(space + 1);
            }
            if (s.Length > 0)
                lines.Add(s);
            else if (lines.Count == 0)
                lines.Add("");
            return lines;
        }

        static string Wrap(string s, int indent, int first_line_indent)
        {
            int width = ConsoleEx.IsOutputRedirected ? 80 : Console.BufferWidth;
            List<string> lines = Wrap(s, width - indent);
            StringBuilder sb = new StringBuilder();
            sb.Append(' ', first_line_indent).AppendLine(lines[0]);
            for (int i = 1; i < lines.Count; ++i)
                sb.Append(' ', indent).AppendLine(lines[i]);
            return sb.ToString().TrimEnd();
        }

        static void Usage()
        {
            string program = Path.GetFileNameWithoutExtension(Environment.GetCommandLineArgs()[0]);
            if (program.Contains(" ")) {
                program = '"'+program+'"';
            }
            string usage = "  {0} {1} [{2}...]";
            Console.WriteLine(UI.GetMessage(Msg.Usage));
            Console.WriteLine(String.Format(usage, program, "bootskin.bs7", UI.GetMessage(Msg.Options)));
            Console.WriteLine("    " + UI.GetMessage(Msg.OrToRestoreBackupFiles));
            Console.WriteLine(String.Format(usage, program, "/restore", UI.GetMessage(Msg.Options)));
            Console.WriteLine("    " + "or to pre-download debug-symbols (needed for full-screen image version)");
            Console.WriteLine(String.Format(usage, program, "/download", UI.GetMessage(Msg.Options)));
            Console.WriteLine();
            Console.WriteLine(UI.GetMessage(Msg.WhereTheOptionsAre));
            Console.WriteLine("  " + UI.GetMessage(Msg.FolderOpt, "/Windows", Wrap(UI.GetMessage(Msg.SetsAsManyOfTheOptionsBelowAsPossible), 20, 2)));
            Console.WriteLine("  " + UI.GetMessage(Msg.FileDefault, "/Bootres", defaults["bootres"]));
            Console.WriteLine("  " + UI.GetMessage(Msg.FileDefault, "/Winload", defaults["winload"]));
            Console.WriteLine("  " + UI.GetMessage(Msg.FileDefault, "/WinloadMui", defaults["winloadmui"]));
            Console.WriteLine("  " + UI.GetMessage(Msg.FileDefault, "/Winresume", defaults["winresume"]));
            Console.WriteLine("  " + UI.GetMessage(Msg.FileDefault, "/WinresumeMui", defaults["winresumemui"]));
            Console.WriteLine("  " + UI.GetMessage(Msg.FileDefault, "/Bootmgr", Bootmgr.DefaultIsOnHiddenSystemPartition() ? UI.GetMessage(Msg.OnHiddenSystemPartition) : defaults["bootmgr"]));
            Console.WriteLine();
            Console.WriteLine(UI.GetMessage(Msg.YouCanUseTheGUIProgramToCreateBS7Files));
            Console.WriteLine();
        }
        #endregion

        #region Messaging Functions
        static void ShowError(string text, string caption)
        {
            string msg = "! " + UI.GetMessage(Msg.ERROR) + ": " + (String.IsNullOrEmpty(caption) ? "" : caption + ": ") + text;
            if (ConsoleEx.IsErrorRedirected)
            {
                Console.Error.WriteLine(msg);
                Console.Error.WriteLine();
            }
            else
            {
                ConsoleColor cc = Console.ForegroundColor;
                Console.ForegroundColor = ConsoleColor.Red;
                Console.Error.WriteLine(msg);
                Console.Error.WriteLine();
                Console.ForegroundColor = cc;
            }
        }
        static void ShowMessage(string text, string caption)
        {
            Console.WriteLine((String.IsNullOrEmpty(caption) ? "" : caption + ": ") + text);
            Console.WriteLine();
        }
        static bool ShowYesNo(string text, string caption)
        {
            char Y = UI.GetMessage(Msg.Y)[0], n = UI.GetMessage(Msg.n)[0];
            Console.WriteLine((String.IsNullOrEmpty(caption) ? "" : caption + ": ") + text + " [" + Y + "/" + n + "] ");
            string line;
            while ((line = Console.ReadLine()) != null) {
                line = line.TrimStart();
                if (line.Length == 0 || line[0] == Y || line[0] == UI.GetMessage(Msg.y)[0])
                    return true;
                else if (line[0] == n || line[0] == UI.GetMessage(Msg.N)[0])
                    return false;
            }
            return false;
        }
        static string last_text = null;
        static void ProgessChanged(string text, int cur, int max)
        {
            if (text != last_text)
            {
                //if (ConsoleEx.IsOutputRedirected)
                //{
                    Console.WriteLine(text);
                //}
                //else
                //{
                //    Console.SetCursorPosition(0, Console.CursorTop - 1);
                //    Console.WriteLine(text + new string(' ', Console.BufferWidth - text.Length));
                //}
                last_text = text;
            }

            /*if (!ConsoleEx.IsOutputRedirected)
            {
                // This could probably use some work
                int width = Console.BufferWidth;
                int bars = cur * (width - 2) / max;
                int blanks = width - 2 - bars;
                string s = "[" + new string('=', bars) + new string(' ', blanks) + "]";
                Console.CursorVisible = false;
                Console.SetCursorPosition(0, Console.CursorTop - 1);
                Console.Write(s);
                Console.CursorVisible = true;
            }*/
        }
        #endregion

        #region Argument Parsing / Defaults
        delegate uint Check(string file);
        static Dictionary<string, string> defaults = new Dictionary<string, string>();
        static Dictionary<string, Check> checks = new Dictionary<string, Check>();
        static void SetupDefaults()
        {
            defaults.Add("bootres", Bootres.def);
            defaults.Add("winload", Winload.def);
            defaults.Add("winloadmui", Winload.defMui);
            defaults.Add("winresume", Winresume.def);
            defaults.Add("winresumemui", Winresume.defMui);
            defaults.Add("bootmgr", Bootmgr.def);

            checks.Add("bootres", Bootres.Check);
            checks.Add("winload", Winload.Check);
            checks.Add("winloadmui", Winload.CheckMui);
            checks.Add("winresume", Winresume.Check);
            checks.Add("winresumemui", Winresume.CheckMui);
            checks.Add("bootmgr", Bootmgr.Check);
        }
        private static string IfPathExists(string path) { return File.Exists(path) ? path : null; }
        static void LoadFileFromFolder(string win, Dictionary<string, string> opts)
        {
            string bootmgr = null, bootres = null, winload = null, winloadMui = null, winresume = null, winresumeMui = null;

            Updater.DisableFSRedirection();

            string sys32 = Path.Combine(win, "System32");
            string parent = Path.GetDirectoryName(win);

            bootmgr = IfPathExists(Path.Combine(parent, "bootmgr"));

            if (Directory.Exists(sys32))
            {
                bootres = IfPathExists(Path.Combine(sys32, "bootres.dll"));
                winload = IfPathExists(Path.Combine(sys32, "winload.exe"));
                winresume = IfPathExists(Path.Combine(sys32, "winresume.exe"));

                string sys32l = Path.Combine(sys32, Updater.GetPreferredLocale());
                if (!Directory.Exists(sys32l)) sys32l = Path.Combine(sys32, "en-US");
                if (Directory.Exists(sys32l))
                {
                    winloadMui = IfPathExists(Path.Combine(sys32l, "winload.exe.mui"));
                    winresumeMui = IfPathExists(Path.Combine(sys32l, "winresume.exe.mui"));
                }

                if (winloadMui == null) winloadMui = IfPathExists(Path.Combine(sys32, "winload.exe.mui"));
                if (winresumeMui == null) winresumeMui = IfPathExists(Path.Combine(sys32, "winresume.exe.mui"));
            }

            if (bootmgr == null)
            {
                string boot = Path.Combine(win, "Boot"), pcat = Path.Combine(boot, "PCAT");
                if (Directory.Exists(pcat))
                    bootmgr = IfPathExists(Path.Combine(pcat, "bootmgr"));
            }

            if (bootmgr == null) bootmgr = IfPathExists(Path.Combine(win, "bootmgr"));
            if (bootres == null) bootres = IfPathExists(Path.Combine(win, "bootres.dll"));
            if (winload == null) winload = IfPathExists(Path.Combine(win, "winload.exe"));
            if (winloadMui == null) winloadMui = IfPathExists(Path.Combine(win, "winload.exe.mui"));
            if (winresume == null) winresume = IfPathExists(Path.Combine(win, "winresume.exe"));
            if (winresumeMui == null) winresumeMui = IfPathExists(Path.Combine(win, "winresume.exe.mui"));

            Updater.RevertFSRedirection();

            List<string> updated = new List<string>(5);
            if (bootmgr != null) { opts["bootmgr"] = bootmgr; updated.Add("bootmgr"); }
            if (bootres != null) { opts["bootres"] = bootres; updated.Add("bootres.dll"); }
            if (winload != null) { opts["winload"] = winload; updated.Add("winload.exe"); }
            if (winloadMui != null) { opts["winloadMui"] = winloadMui; updated.Add("winload.exe.mui"); }
            if (winresume != null) { opts["winresume"] = winresume; updated.Add("winresume.exe"); }
            if (winresumeMui != null) { opts["winresumeMui"] = winresumeMui; updated.Add("winresume.exe.mui"); }

            if (updated.Count == 0)
                Console.WriteLine(UI.GetMessage(Msg.SelectWindowsFolder) + ": " + UI.GetMessage(Msg.NoAcceptableFilesWereFound));
            else
                Console.WriteLine(UI.GetMessage(Msg.SelectWindowsFolder) + ":\n" + UI.GetMessage(Msg.TheFollowingFilesWereFound, string.Join("\n", updated.ToArray())), UI.GetMessage(Msg.SelectWindowsFolder));
        }
        static Dictionary<string, string> GetOptions(string[] args)
        {
            Dictionary<string, string> opts = new Dictionary<string, string>(defaults);
            for (int i = 1; i < args.Length; i += 2)
            {
                if (args[i][0] != '/' && args[i][0] != '-')
                {
                    UI.ShowError(UI.GetMessage(Msg.UnrecognizedOption, args[i]), "");
                    return null;
                }
                string name = args[i].Substring(1).ToLower();
                if (name == "windows")
                {
                    LoadFileFromFolder(args[i + 1], opts);
                }
                else if (!opts.ContainsKey(name))
                {
                    UI.ShowError(UI.GetMessage(Msg.UnrecognizedOption, args[i]), "");
                    return null;
                }
                else
                {
                    opts[name] = Path.GetFullPath(args[i + 1]);
                }
            }

            uint err;
            foreach (string key in opts.Keys)
                if ((err = checks[key].Invoke(opts[key])) != 0)
                {
                    UI.ShowError(UI.GetMessage(Msg.ThereWasAProblemVerifying, opts[key]), UI.GetMessage(Msg.SelectADifferentFile), err, "");
                    return null;
                }

            return opts;
        }
        static Dictionary<string, string> ParseArgs(string[] args, out string file, out bool restore, out bool download)
        {
            file = null;
            restore = false;
			download = false;

            if (args.Length % 2 == 0) // inappropriate number of options
            {
                if (args.Length > 1)
                    UI.ShowError(UI.GetMessage(Msg.WrongNumberOfArguments), "");
                return null;
            }

            string zero = args[0].ToLower();
            restore = (zero.Equals("/restore") || zero.Equals("-restore"));
            download = (zero.Equals("/download") || zero.Equals("-download"));
            if (!restore && !download)
            {
                file = args[0];
                if (!File.Exists(file))
                {
                    if (!File.Exists(file + ".bs7"))
                    {
                        UI.ShowError(UI.GetMessage(Msg.CouldNotFindTheGivenBS7File, file), "");
                        return null;
                    }
                    file += ".bs7";
                }
            }

            return GetOptions(args);
        }
        #endregion

        static int Download(Dictionary<string, string> opts)
        {
            string[] files = new string[] { opts["winload"], opts["winresume"] };
            for (int i = 0; i < files.Length; ++i)
            {
				string dest = Updater.DownloadPDB(files[i]);
                if (dest != null) { Console.WriteLine("Saved PDB for {0} to {1}", files[i], dest); }
				else { Console.WriteLine("Failed to download PDB for {0}", files[i]); }
            }
			return 0;
		}
		
        static int Restore(Dictionary<string, string> opts)
        {
            string[] files = new string[] { opts["bootres"], opts["winload"], opts["winloadmui"], opts["winresume"], opts["winresumemui"], opts["bootmgr"] };
            string[] sources = Updater.Restore(files);
            bool any = false;
            for (int i = 0; i < files.Length; ++i)
            {
                if (sources[i] != null)
                {
                    any = true;
                    int s = sources[i].LastIndexOf('\\'), f = files[i].LastIndexOf('\\');
                    if (s == f && sources[i].StartsWith(files[i].Remove(f)))
                        files[i] = files[i].Substring(f+1);
                    if (sources[i].StartsWith("\\\\?\\Volume{")) sources[i] = "*" + sources[i].Substring(sources[i].IndexOf('}') + 1);
                    if (files[i].StartsWith("\\\\?\\Volume{")) files[i] = "*" + files[i].Substring(files[i].IndexOf('}') + 1);
                    Console.WriteLine(UI.GetMessage(Msg.RestoredTo, sources[i], files[i]));
                }
            }
            if (!any)
                Console.WriteLine(UI.GetMessage(Msg.NoFilesWereRestored));
            Console.WriteLine();
            return 0;
        }

        static int Update(string file, Dictionary<string, string> opts)
        {
            // Load the boot skin
            BootSkin bs = new BootSkin();
            string bs_err = bs.Load(file);
            if (bs_err != null)
            {
                UI.ShowError(UI.GetMessage(Msg.TheBS7FileProvidedIsInvalid, file) + ": " + bs_err, "");
                return -3;
            }

            // Initialize the update
            Console.WriteLine();
            UI.ProgressChanged += new UI.Progress(Program.ProgessChanged);
            UI.InitProgress(Updater.TotalProgress);

            // Run the update
            uint error = 0;
            Exception ex = null;
            try
            {
                error = Updater.Update(bs, opts["bootres"], opts["winload"], opts["winloadmui"], opts["winresume"], opts["winresumemui"], opts["bootmgr"], true);
            }
            catch (Exception _ex) { ex = _ex; }

            Console.WriteLine();

            // Report the results of the update
            if (ex != null)
                UI.ShowError(UI.GetMessage(Msg.ThereWasAnUncaughtExcpetionWhileUpdatingTheFiles) + "\n" + ex, "");
            else if (error != 0)
                UI.ShowError(UI.GetMessage(Msg.ThereWasAProblemUpdatingTheFiles), null, error, "");
            else
                Console.WriteLine(UI.GetMessage(Msg.SuccessfullyUpdatedTheBootAnimationAndText));

            Console.WriteLine();

            return (int)error;
        }

        static void Init()
        {
            // Register the UI messengers
            UI.ErrorMessenger = new UI.Message(Program.ShowError);
            //UI.Messenger = new UI.Message(Program.ShowMessage);
            //UI.YesNoMessenger = new UI.YesNoMsg(Program.ShowYesNo);

            // Initialize the library
            Updater.Init();
        }

        internal static int Main(string[] args)
        {
            Init();
            About();

            if (!Updater.EnablePrivileges())
            {
                UI.ShowError(UI.GetMessage(Msg.FailedToEnableTheTakeOwnershipPrivilege), "");
                return -3;
            }

            SetupDefaults(); // Setup defaults for files

            // Parse the command line
            bool restore, download;
            string file;
            Dictionary<string, string> opts = ParseArgs(args, out file, out restore, out download);
            if (opts == null)
            {
                Usage();
                return -2;
            }

            // Run the desired command
            return download ? Download(opts) : (restore ? Restore(opts) : Update(file, opts));
        }
    }
}
