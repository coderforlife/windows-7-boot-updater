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

using Win7BootUpdater;

namespace Win7BootUpdater.Installer
{
    internal class Main : Form, TranslatableItem
    {
        private InstallerPageID current_page;
        public InstallerPage[] pages;

        private Panel bottomPanel;
        public Button butBack, butNext, butClose;

        private static Main main;
        public static Main GetInstance() { return main; }

        private bool uninstalling;
        public bool Uninstalling { get { return uninstalling; } }

        public int ReturnValue = 0;

        public Main(bool uninstall)
        {
            this.uninstalling = uninstall;

            // Register the UI messengers
            UI.ErrorMessenger = new UI.Message(this.ShowError);
            //UI.Messenger = new UI.Message(this.ShowMessage);
            //UI.YesNoMessenger = new UI.YesNoMsg(this.ShowYesNo);

            // Loads required libraries
            Updater.Init();

            //Preload Patches
            Updater.PreloadPatch("winload");
            Updater.PreloadPatch("bootmgr");
            Updater.PreloadPatch("winresume");

            // Enable privileges
            if (!Updater.EnablePrivileges())
            {
                MessageBox.Show(UI.GetMessage(Msg.FailedToEnableTheTakeOwnershipPrivilege), UI.GetMessage(Msg.CriticalError), MessageBoxButtons.OK, MessageBoxIcon.Error);
                //throw new Exception(UI.GetMessage(Msg.FailedToEnableTheTakeOwnershipPrivilege));
                ReturnValue = -2;
                return;
            }

            // Singleton setup
            main = this;

            pages = new InstallerPage[]{
                new Pages.Welcome(this),
                new Pages.License(this),
                new Pages.UninstallCurrent(this),
                new Pages.ReadyToInstall(this),
                new Pages.Install(this),
                new Pages.Successful(this),
                new Pages.Error(this),
                new Pages.UninstallQuestion(this),
                new Pages.Uninstall(this),
                new Pages.UninstallSuccessful(this),
                new Pages.UninstallError(this),
            };

            this.SuspendLayout();
            this.ClientSize = new Size(497, 361);
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.FormClosing += new FormClosingEventHandler(this.Main_FormClosing);
            this.Icon = Res.GetAppIcon();

            this.bottomPanel = Builder.createPanel(new Point(0, 314), new Size(497, 47), 100, Builder.bottomLeftRightAnchor, this);
            this.bottomPanel.Paint += new PaintEventHandler(this.bottomPanel_Paint);

            this.butBack = Builder.createButton(new Point(248, 13), 75, 1, Builder.bottomRightAnchor, new EventHandler(this.butBack_Click), this.bottomPanel);
            this.butNext = Builder.createButton(new Point(323, 13), 75, 2, Builder.bottomRightAnchor, new EventHandler(this.butNext_Click), this.bottomPanel);
            this.butClose = Builder.createButton(new Point(409, 13), 75, 3, Builder.bottomRightAnchor, new EventHandler(this.butClose_Click), this.bottomPanel);
            //this.butClose.DialogResult = DialogResult.Cancel;

            this.bottomPanel.ResumeLayout();
            
            this.Controls.AddRange(pages);
            if (uninstall)
            {
                SetPage(InstallerPageID.UninstallQuestion);
            }
            else
            {
                SetPage(InstallerPageID.Welcome);
            }

            Builder.AddTranslatableItem(this);
            this.ResumeLayout();
        }
        public void Translate()
        {
            this.Text = UI.GetMessage(Msg.Windows7BootSkinInstaller) + " - " + UI.GetMessage(Msg.by, Program.SkinName, Program.SkinAuthor);
            this.butBack.Text = UI.GetMessage(this.CurrentPage.BackText);
            this.butNext.Text = UI.GetMessage(this.CurrentPage.NextText);
            this.butClose.Text = UI.GetMessage(this.CurrentPage.CloseText);
        }
        private delegate void dShow(string text, string caption, MessageBoxIcon icon);
        public void ShowMessage(string text, string caption, MessageBoxIcon icon)
        {
            if (this.InvokeRequired)
                this.Invoke(new dShow(this.ShowMessage), text, caption, icon);
            else
                MessageBox.Show(this, text, caption, MessageBoxButtons.OK, icon);
        }
        public void ShowError(string text, string caption) { ShowMessage(text, caption, MessageBoxIcon.Error); }
        private void bottomPanel_Paint(object sender, PaintEventArgs e) {
            e.Graphics.DrawLine(SystemPens.ActiveBorder,   0, 0, bottomPanel.Width, 0);
            e.Graphics.DrawLine(SystemPens.InactiveBorder, 0, 1, bottomPanel.Width, 1);
        }
        public InstallerPage GetPage(InstallerPageID i) { return (i != InstallerPageID.None) ? this.pages[(int)i] : null; }
        private delegate void _SetPage(InstallerPageID i);
        public void SetPage(InstallerPageID i)
        {
            if (this.InvokeRequired) { this.Invoke(new _SetPage(this.SetPage), i); return; }

            this.SuspendLayout();
            this.pages[(int)this.current_page].Visible = false;
            this.current_page = i;
            InstallerPage p = this.pages[(int)i];

            this.butBack.Visible = p.BackOn;
            this.butBack.Text = UI.GetMessage(p.BackText);
            this.butNext.Visible = p.NextOn;
            this.butNext.Text = UI.GetMessage(p.NextText);
            this.butClose.Visible = p.CloseOn;
            this.butClose.Text = UI.GetMessage(p.CloseText);

            if (p.NextOn)       this.AcceptButton = this.butNext;
            else if (p.CloseOn) this.AcceptButton = this.butClose;
            else if (p.BackOn)  this.AcceptButton = this.butBack;
            else                this.AcceptButton = null;

            if (p.CloseOn)      this.CancelButton = this.butClose;
            else if (p.BackOn)  this.CancelButton = this.butBack;
            else if (p.NextOn)  this.CancelButton = this.butNext;
            else                this.CancelButton = null;

            p.Visible = true;
            this.ResumeLayout();
        }
        public InstallerPage CurrentPage { get { return (this.current_page != InstallerPageID.None) ? this.pages[(int)this.current_page] : null; } }
        private void butBack_Click(object sender, EventArgs e) { if (CurrentPage.BackPage != InstallerPageID.None) { SetPage(CurrentPage.BackPage); } }
        private void butNext_Click(object sender, EventArgs e) { if (CurrentPage.NextPage != InstallerPageID.None) { SetPage(CurrentPage.NextPage); } }
        private void butClose_Click(object sender, EventArgs e) { this.Close(); }
        private void Main_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (!CurrentPage.CloseOn)
                e.Cancel = true;
            else if (CurrentPage.AskToClose &&
                MessageBox.Show(this, "Are you sure you want to close the installer before it is complete?", "Exit Setup", MessageBoxButtons.YesNo, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2) != DialogResult.Yes)
            {
                ReturnValue = -20;
                e.Cancel = true;
            }
        }
    }
}
