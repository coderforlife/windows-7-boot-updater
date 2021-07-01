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
    internal class Error : CommonInstallerPage, TranslatableItem
    {
        public const int MESSAGE = 3;
        private string message = "";

        private FormattedTextBox text;

        public Error(Main m) : base(Msg.FailedToInstall, Msg.NO_MESSAGE, m)
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
                if (key == MESSAGE)
                {
                    this.message = (string)value;
                    Translate();
                }
            }
        }
        public void Translate() { this.text.Text = UI.GetMessage(Msg.FailedToInstall_Full, Program.SkinName, Program.SkinAuthor, this.message); }
        public override InstallerPageID ThisPage { get { return InstallerPageID.Error; } }
        public override bool BackOn { get { return false; } }
        public override bool NextOn { get { return false; } }
        public override Msg CloseText { get { return Msg.Close; } }
        public override bool AskToClose { get { return false; } }
    }
}
