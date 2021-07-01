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
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows.Forms;
using System.Xml;

/*
 * Command line options:
 * /x /u /uninstall         uninstall
 * /s /silent /q /quiet     silent
 */

namespace Win7BootUpdater.Installer
{
    internal static class Program
    {
        [STAThread]
        public static int Main(string[] args)
        {
            Init();

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            if (!Updater.IsWindows7OrNewer())
            {
                MessageBox.Show(UI.GetMessage(Msg.ThisCanOnlyBeUsedInWindows7), UI.GetMessage(Msg.ERROR), MessageBoxButtons.OK, MessageBoxIcon.Error);
                return -1;
            }

            bool silent = false, uninstall = false;
            foreach (string _arg in args) {
                if (_arg[0] != '/' && _arg[0] != '-') continue;
                string arg = _arg.Substring(1);
                if (arg == "s" || arg == "q" || arg == "silent" || arg == "quiet")
                    silent = true;
                else if (arg == "x" || arg == "u" || arg == "uninstall")
                    uninstall = true;
            }

            if (silent)
            {
                if (uninstall) {
                    string restored = "", not_restored = "";
                    return Uninstall(ref restored, ref not_restored);
                }
                else
                {
                    string error = "";
                    return Install(ref error);
                }
            }
            else
            {
                Main m = new Main(uninstall);
                Application.Run(m);
                return m.ReturnValue;
            }
        }

        public static string GetExePath()
        {
            return System.Reflection.Assembly.GetExecutingAssembly().Location;
        }

        #region Constants and Such

        public const string HomepageLink = "http://www.coderforlife.com/";
        public const string W7BULink = "http://www.coderforlife.com/projects/win7boot/";
        //public const string TranslateLink = "http://www.coderforlife.com/projects/win7boot/translate/";
        //public const string PublicBootSkinLink = "http://www.coderforlife.com/projects/win7boot/bootskins/";
        //public const string DonationLink = "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=T8JXGRGG9JDGQ&lc=US&item_name=Coder%20For%20Life&item_number=bootupdater&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donate_LG%2egif%3aNonHosted";

        public const int BOOTSKIN = 1;
        public const int BOOTSKIN_DESC = 2;
        public const int BOOTSKIN_IMAGE = 3;

        public static readonly BootSkin BS = new BootSkin();
        public static readonly XmlDocument BSdesc = new XmlDocument();
        public static readonly Image BSImage = Res.GetImage(Res.Type.RCDATA, BOOTSKIN_IMAGE);
        
        private static void Init()
        {
            Stream s = Res.GetStream(Res.Type.RCDATA, BOOTSKIN);
            string err = BS.Load(s);
            s.Close();
            // TODO: handle error

            XmlReader xml = null;
            try { BSdesc.Load(xml = XmlReader.Create(Res.GetStream(Res.Type.RCDATA, BOOTSKIN_DESC))); }
            catch (Exception)
            {
                // TODO: handle error
            }
            finally { if (xml != null) xml.Close(); }
        }

        private static string GetBSDescValue(string name)
        {
            if (BSdesc == null || BSdesc.DocumentElement == null) return null;
            XmlElement e = BSdesc.DocumentElement[name];
            return (e == null) ? null : e.InnerText;
        }

        public static string SkinName           { get { return GetBSDescValue("Name"); } }
        public static string SkinAuthor         { get { return GetBSDescValue("Author"); } }
        public static string SkinDescription    { get { return GetBSDescValue("Description"); } }
        public static string SkinURL            { get { return GetBSDescValue("URL"); } }
        public static string SkinLicense        { get { return GetBSDescValue("License"); } }

        private delegate uint Check(string file);
        public static readonly string[] names = { "bootres.dll", "winload.exe", "winload.exe.mui", "winresume.exe", "winresume.exe.mui", "bootmgr" };
        public static readonly string[] defaults = { Bootres.def, Winload.def, Winload.defMui, Winresume.def, Winresume.defMui, Bootmgr.def };
        private static readonly Check[] checks = { Bootres.Check, Winload.Check, Winload.CheckMui, Winresume.Check, Winresume.CheckMui, Bootmgr.Check };

        #endregion

        #region Install
        private static int CheckAll(string[] files, ref string error)
        {
            StringBuilder msg = new StringBuilder();
            for (int i = 0; i < names.Length; ++i)
            {
                uint err = checks[i](files[i]);
                if (err != 0)
                    msg.AppendLine(UI.GetMessage(Msg.ThereWasAProblemVerifying, names[i]) + ": " + UI.GetErrorMessage(err, UI.GetMessage(Msg.NoError)));
            }
            if (msg.Length == 0)
                return 0;
            error = msg.ToString().Trim();
            return -4;
        }

        private static int UpdateFiles(string[] files, ref string error)
        {
            UI.InitProgress(Updater.TotalProgress);
            uint err = 0;
            Exception ex = null;
            try
            {
                err = Updater.Update(Program.BS, files[0], files[1], files[2], files[3], files[4], files[5], true);
            }
            catch (Exception _ex) { ex = _ex; }
            if (ex != null && err != 0)
            {
                if (ex != null)
                {
                    error = UI.GetMessage(Msg.ThereWasAnUncaughtExcpetionWhileUpdatingTheFiles) + '\n' + ex;
                    return -5;
                }
                else //if (error != 0)
                {
                    error = UI.GetMessage(Msg.ThereWasAProblemUpdatingTheFiles) + '\n' + UI.GetErrorMessage(err, UI.GetMessage(Msg.NoError));
                    return (int)err;
                }
            }
            return 0;
        }

        private static string GetUninstallerFolder()
        {
#if _WIN64
            return Environment.GetFolderPath(Environment.SpecialFolder.System);
#else
            return Path.Combine(Path.GetDirectoryName(Environment.GetFolderPath(Environment.SpecialFolder.System)), "sysnative");
#endif
        }

        private static int CreateUninstaller(string[] modifiedFiles, ref string error)
        {
            string init = Program.GetExePath();
            string dest = Path.Combine(GetUninstallerFolder(), Path.GetFileName(init));
            File.Copy(init, dest, true);
            Registry.AddUninstallInformation(Program.SkinName, Program.SkinAuthor, Program.SkinURL, dest, modifiedFiles);
            return 0;
        }

        public static int Install(ref string error)
        {
            int err;
            if ((err = Program.CheckAll(Program.defaults, ref error)) != 0) { return err; }
            if ((err = Program.UpdateFiles(Program.defaults, ref error)) != 0) { return err; }
            if ((err = Program.CreateUninstaller(Program.defaults, ref error)) != 0) { return err; }
            return 0;
        }
        #endregion

        public static int Uninstall(ref string restored, ref string not_restored)
        {
            string[] files = Registry.GetInstalledModifiedFiles();
            string[] sources = Updater.Restore(files);

            bool successful = true;
            restored = "";
            not_restored = "";
            for (int i = 0; i < files.Length; ++i)
            {
                if (sources[i] != null)
                {
                    int sp = sources[i].LastIndexOf('\\'), fp = files[i].LastIndexOf('\\');
                    if (sp == fp && sources[i].StartsWith(files[i].Remove(fp)))
                        files[i] = files[i].Substring(fp + 1);
                    if (sources[i].StartsWith(@"\\?\Volume{")) sources[i] = "*" + sources[i].Substring(sources[i].IndexOf('}') + 1);
                    if (files[i].StartsWith(@"\\?\Volume{")) files[i] = "*" + files[i].Substring(files[i].IndexOf('}') + 1);
                    restored += UI.GetMessage(Msg.RestoredTo, sources[i], files[i]) + '\n';
                    Registry.RemoveModifiedFile(files[i]);
                }
                else
                {
                    not_restored += UI.GetMessage(Msg.FailedToRestore, files[i]) + '\n';
                    successful = false;
                }
            }
            restored = restored.Trim();
            not_restored = not_restored.Trim();

            int retval = 0;
            if (successful)
            {
                string uninstaller = Registry.GetUninstallerPath();
                Registry.RemoveUninstallInformation();
                if (uninstaller != null)
                {
                    try
                    {
                        File.Delete(uninstaller);
                    }
                    catch (Exception)
                    {
                        Updater.DeleteAfterExit(uninstaller);
                    }
                }
            }
            else
            {
                retval = -3;
            }
            return retval;
        }
    }
}
