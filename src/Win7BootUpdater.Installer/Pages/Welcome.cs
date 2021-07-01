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
    internal class Welcome : InstallerPage, TranslatableItem
    {
        private FormattedTextBox text;

        public Welcome(Main m) : base(m)
        {
            this.SuspendLayout();

            PictureBox p = Builder.createPicture(new Point(0, 0), new Size(170, 314), this);
            p.Anchor = Builder.topBottomLeft;
            p.Image = Program.BSImage;
            p.SizeMode = PictureBoxSizeMode.CenterImage;

            Builder.createCenteredMultilineLabel(Msg.BootSkinInstallerForWindows7, new Point(176, 12), new Size(309, 42), this).Font =
                new Font(this.Font.FontFamily, 12F, FontStyle.Bold);
            Builder.createCenteredMultilineLabel(Msg.by, new string[] { Program.SkinName, Program.SkinAuthor }, new Point(176, 60), new Size(309, 30), this).Font =
                new Font(this.Font, FontStyle.Bold);

            this.text = Builder.createMultilineLabel(new Point(176, 96), new Size(309, 206), Builder.topBottomLeftRightAnchor, this);

            Builder.AddTranslatableItem(this);
            this.ResumeLayout();
        }
        public void Translate() {
            string desc = Program.SkinDescription;
            desc = (desc == null) ? desc = "" : desc.Trim();
            if (desc.Length > 0)
                desc += "\n\n";
            this.text.Text = desc + UI.GetMessage(Msg.Welcome_Full, Program.W7BULink, Program.HomepageLink);
        }
        public override bool BackOn { get { return false; } }
        public override InstallerPageID ThisPage { get { return InstallerPageID.Welcome; } }
        public override InstallerPageID NextPage { get { return InstallerPageID.License; } }
    }
}
