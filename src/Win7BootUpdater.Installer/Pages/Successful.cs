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
using System.Windows.Forms;

namespace Win7BootUpdater.Installer.Pages
{
    internal class Successful : CommonInstallerPage
    {
        public Successful(Main m) : base(Msg.SuccessfullyInstalled, Msg.NO_MESSAGE, m) {
            Builder.createFullPageLabel(Msg.SuccessfullyInstalled_Full, new string[] { Program.SkinName, Program.SkinAuthor }, this.content);
        }
        public override InstallerPageID ThisPage { get { return InstallerPageID.Successful; } }
        public override bool NextOn { get { return false; } }
        public override bool BackOn { get { return false; } }
        public override Msg CloseText { get { return Msg.Close; } }
        public override bool AskToClose { get { return false; } }
    }
}
