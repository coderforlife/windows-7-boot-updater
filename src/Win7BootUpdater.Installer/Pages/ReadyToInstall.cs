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
using System.Text;

namespace Win7BootUpdater.Installer.Pages
{
    internal class ReadyToInstall : CommonInstallerPage, TranslatableItem
    {
        private FormattedTextBox text;

        public ReadyToInstall(Main m) : base(Msg.ReadyToInstall, Msg.NO_MESSAGE, m)
        {
            this.text = Builder.createFullPageLabel(this.content);
            Builder.AddTranslatableItem(this);
        }
        private string GetFileList()
        {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < Program.defaults.Length; ++i)
            {
                if (i == Program.defaults.Length - 1 && Bootmgr.DefaultIsOnHiddenSystemPartition())
                    sb.AppendLine("bootmgr "+ UI.GetMessage(Msg.OnHiddenSystemPartition));
                else
                    sb.AppendLine(Program.defaults[i]);
            }
            return sb.ToString().Trim();
        }
        public void Translate()
        {
            this.text.Text = UI.GetMessage(Msg.ReadyToInstall_Full, Program.SkinName, Program.SkinAuthor, GetFileList(), Program.W7BULink);
        }
        public override Msg NextText { get { return Msg.Install; } }
        public override InstallerPageID ThisPage { get { return InstallerPageID.ReadyToInstall; } }
        public override InstallerPageID BackPage { get { return InstallerPageID.License; } }
        public override InstallerPageID NextPage { get { return InstallerPageID.Install; } }
    }
}
