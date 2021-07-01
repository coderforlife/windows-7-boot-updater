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
using System.Windows.Forms;

namespace Win7BootUpdater.Installer.Pages
{
    internal class UninstallError : CommonInstallerPage, TranslatableItem
    {
        public const int RESTORED = UninstallSuccessful.RESTORED;
        public const int NOT_RESTORED = 2;
        private string restored = "";
        private string not_restored = "";

        private FormattedTextBox text;

        public UninstallError(Main m) : base(Msg.FailedToUninstall, Msg.NO_MESSAGE, m)
        {
            Builder.createPicture(new Point(453, 12), new Size(32, 32), this.content).Image = SystemIcons.Error.ToBitmap();
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
                else if (key == NOT_RESTORED)
                {
                    this.not_restored = (string)value;
                    Translate();
                }
            }
        }
        public void Translate() { this.text.Text = UI.GetMessage(Msg.FailedToUninstall_Full, Registry.GetInstalledSkinName(), Registry.GetInstalledSkinAuthor(), this.not_restored, this.restored); }
        public override InstallerPageID ThisPage { get { return InstallerPageID.UninstallError; } }
        public override bool BackOn { get { return false; } }
        public override bool NextOn { get { return false; } }
        public override Msg CloseText { get { return Msg.Close; } }
        public override bool AskToClose { get { return false; } }
    }
}
