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
    internal class UninstallQuestion : CommonInstallerPage
    {
        public UninstallQuestion(Main m) : base(Msg.ReadyToUninstall, Msg.NO_MESSAGE, m)
        {
            Builder.createFullPageLabel(Msg.ReadyToUninstall_Full, new string[] { Registry.GetInstalledSkinName(), Registry.GetInstalledSkinAuthor() }, this.content);
        }
        public override InstallerPageID ThisPage { get { return InstallerPageID.UninstallQuestion; } }
        public override InstallerPageID NextPage { get { return InstallerPageID.Uninstall; } }
        public override bool BackOn { get { return false; } }
    }
}
