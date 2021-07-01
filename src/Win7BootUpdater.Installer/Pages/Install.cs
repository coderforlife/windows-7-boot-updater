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
    internal class Install : CommonInstallerPage
    {
        Label text;
        ProgressBar progress;

        public Install(Main m) : base(Msg.Installing, Msg.NO_MESSAGE, m)
        {
            this.text = Builder.createLabel(new Point(9, 100), this.content);
            this.progress = (ProgressBar)Builder.setBasic(new ProgressBar(), new Point(12, 116), new Size(473, 23), -1, Builder.topLeftRightAnchor, this.content);
            Builder.createFullPageLabel(Msg.Installing_Full, new string[] { Program.SkinName, Program.SkinAuthor }, this.content);
            UI.ProgressChanged += new UI.Progress(this.ProgessChanged);
        }
        public void ProgessChanged(string text, int cur, int max)
        {
            if (this.InvokeRequired) { this.Invoke(new UI.Progress(this.ProgessChanged), text, cur, max); }
            else
            {
                this.text.Text = text;
                this.progress.Value = cur;
                this.progress.Maximum = max;
            }
        }
        protected override void Execute()
        {
            this.text.Text = "";
            string error = "";
            int err = Program.Install(ref error);
            if (err == 0)
                main.SetPage(InstallerPageID.Successful);
            else
            {
                main.ReturnValue = err;
                main.GetPage(InstallerPageID.Error).SetInformation(Error.MESSAGE, error);
                main.SetPage(InstallerPageID.Error);
            }
        }
        public override bool BackOn { get { return false; } }
        public override bool NextOn { get { return false; } }
        public override bool CloseOn { get { return false; } }
        public override InstallerPageID ThisPage { get { return InstallerPageID.Install; } }
    }
}
