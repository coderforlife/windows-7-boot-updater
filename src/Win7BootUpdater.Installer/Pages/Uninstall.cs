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

namespace Win7BootUpdater.Installer.Pages
{
    internal class Uninstall : CommonInstallerPage
    {
        public Uninstall(Main m) : base(Msg.Uninstalling, Msg.NO_MESSAGE, m)
        {
            Builder.createFullPageLabel(Msg.Uninstalling_Full, new string[] { Registry.GetInstalledSkinName(), Registry.GetInstalledSkinAuthor() }, this.content);
        }
        protected override void Execute()
        {
            string restored = "", not_restored = "";
            int err = Program.Uninstall(ref restored, ref not_restored);

            InstallerPageID pid = (err == 0) ? InstallerPageID.UninstallSuccessful : InstallerPageID.UninstallError;
            InstallerPage p = main.GetPage(pid);
            if (err != 0)
                p.SetInformation(UninstallError.NOT_RESTORED, not_restored);
            p.SetInformation(UninstallSuccessful.RESTORED, restored);
            main.SetPage(pid);
            main.ReturnValue = err;
        }
        public override bool BackOn { get { return false; } }
        public override bool NextOn { get { return false; } }
        public override bool CloseOn { get { return false; } }
        public override InstallerPageID ThisPage { get { return InstallerPageID.Uninstall; } }
    }
}
