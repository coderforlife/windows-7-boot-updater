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
    internal class UninstallSuccessful : CommonInstallerPage, TranslatableItem
    {
        public const int RESTORED = 1;
        private string restored = "";

        private FormattedTextBox text;

        private static readonly string name = Registry.GetInstalledSkinName();
        private static readonly string author = Registry.GetInstalledSkinAuthor();

        public UninstallSuccessful(Main m) : base(Msg.SuccessfullyUninstalled, Msg.NO_MESSAGE, m) {
            this.text = Builder.createFullPageLabel(this.content);
            Builder.AddTranslatableItem(this);
        }
        public override void SetInformation(int key, object value)
        {
            if (this.InvokeRequired)
            {
                this.Invoke(new SetInformationHandler(this.SetInformation), key, value);
            }
            else
            {
                if (key == RESTORED)
                {
                    this.restored = (string)value;
                    Translate();
                }
            }
        }
        public void Translate() { this.text.Text = UI.GetMessage(Msg.SuccessfullyUninstalled_Full, name, author, this.restored); }
        public override InstallerPageID ThisPage { get { return InstallerPageID.UninstallSuccessful; } }
        public override InstallerPageID BackPage { get { return InstallerPageID.License; } }
        public override InstallerPageID NextPage { get { return InstallerPageID.ReadyToInstall; } }
        public override bool NextOn { get { return !main.Uninstalling; } }
        public override bool BackOn { get { return !main.Uninstalling; } }
        public override Msg CloseText { get { return main.Uninstalling ? Msg.Close : Msg.Cancel; } }
        public override bool AskToClose { get { return !main.Uninstalling; } }
    }
}
