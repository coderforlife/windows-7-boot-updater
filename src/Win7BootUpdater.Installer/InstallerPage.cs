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
using System.Threading;
using System.Windows.Forms;

namespace Win7BootUpdater.Installer
{
    internal enum InstallerPageID : int
    {
        None = -1,
        Welcome = 0,
        License = 1,
        UninstallCurrent = 2,
        ReadyToInstall = 3,
        Install = 4,
        Successful = 5,
        Error = 6,
        UninstallQuestion = 7,
        Uninstall = 8,
        UninstallSuccessful = 9,
        UninstallError = 10,
    }

    internal abstract class InstallerPage : UserControl
    {
        protected Main main;
        public InstallerPage(Main m)
        {
            this.main = m;
            this.BackColor = SystemColors.Window;
            this.Location = new Point(0, 0);
            this.Size = new Size(497, 314);
            this.Visible = false;
        }
        protected override void OnVisibleChanged(EventArgs e)
        {
            base.OnVisibleChanged(e);
            if (this.Visible)
            {
                Thread t = new Thread(this.Execute);
                t.Name = "Executing Page";
                t.Start();
            }
        }
        protected virtual void Execute() { }
        public delegate void SetInformationHandler(int key, object value);
        public virtual void SetInformation(int key, object value) { }
        public virtual bool BackOn { get { return true; } }
        public virtual bool NextOn { get { return true; } }
        public virtual bool CloseOn { get { return true; } }
        public virtual Msg BackText { get { return Msg.Back; } }
        public virtual Msg NextText { get { return Msg.Next; } }
        public virtual Msg CloseText { get { return Msg.Cancel; } }
        public abstract InstallerPageID ThisPage { get; }
        public virtual InstallerPageID BackPage { get { return InstallerPageID.None; } }
        public virtual InstallerPageID NextPage { get { return InstallerPageID.None; } }
        public virtual bool AskToClose { get { return true; } }
    }
}
