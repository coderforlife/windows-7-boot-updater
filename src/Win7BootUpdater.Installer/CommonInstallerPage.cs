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

namespace Win7BootUpdater.Installer
{
    abstract class CommonInstallerPage : InstallerPage
    {
        protected Panel topPanel;
        protected Panel content;

        public CommonInstallerPage(Msg title, Msg subtitle, Main m) : base(m)
        {
            this.SuspendLayout();
            this.BackColor = SystemColors.Control;
            this.topPanel = Builder.createPanel(new Point(0, 0), new Size(497, 59), 0, Builder.topLeftRightAnchor, this);
            this.topPanel.BackColor = SystemColors.Window;
            Builder.createBoldLabel(title, new Point(12, 9), this.topPanel);
            Builder.createLabel(subtitle, new Point(12, 27), this.topPanel);
            Builder.createPicture(new Point(453, 12), new Size(32, 32), this.topPanel).Image = Res.GetAppIcon().ToBitmap();
            this.topPanel.Paint += new System.Windows.Forms.PaintEventHandler(this.topPanel_Paint);
            this.topPanel.ResumeLayout();
            this.content = Builder.createPanel(new Point(0, 59), new Size(497, 255), 0, Builder.topLeftRightAnchor, this);
        }
        private void topPanel_Paint(object sender, PaintEventArgs e)
        {
            e.Graphics.DrawLine(SystemPens.ActiveBorder,   0, topPanel.Height - 2, topPanel.Width, topPanel.Height - 2);
            e.Graphics.DrawLine(SystemPens.InactiveBorder, 0, topPanel.Height - 1, topPanel.Width, topPanel.Height - 1);
        }
    }
}
