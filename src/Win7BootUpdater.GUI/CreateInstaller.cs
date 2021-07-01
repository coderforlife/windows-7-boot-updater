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
using System.Xml;

namespace Win7BootUpdater.GUI
{
    internal class CreateInstaller : Form, TranslatableItem
    {
        private TextBox name, author, url, desc, license;
        private SaveFileDialog save;

        public CreateInstaller()
        {
            this.save = new SaveFileDialog();
            this.save.RestoreDirectory = true;

            this.SuspendLayout();
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.StartPosition = FormStartPosition.CenterParent;
            this.ClientSize = new Size(584, 292);
            this.ShowInTaskbar = false;
            this.MaximizeBox = false;
            this.Icon = Res.GetAppIcon();
            Builder.AddTranslatableItem(this);

            Builder.createLabel(Msg.CreateInstallerDesc, new Point(12, 9), this);

            Builder.createBoldLabel(Msg.SkinName, new Point(12, 61), this);
            name = Builder.createTextBox(new Point(116, 58), 456, 1, Builder.topLeftRightAnchor, this);

            Builder.createBoldLabel(Msg.SkinAuthor, new Point(12, 87), this);
            author = Builder.createTextBox(new Point(116, 84), 456, 2, Builder.topLeftRightAnchor, this);

            Builder.createLabel(Msg.URL, new Point(12, 113), this);
            url = Builder.createTextBox(new Point(116, 110), 456, 3, Builder.topLeftRightAnchor, this);

            Builder.createLabel(Msg.Description, new Point(12, 139), this);
            desc = Builder.createMultilineTextBox(new Point(116, 136), new Size(456, 52), 4, Builder.topLeftRightAnchor, this);

            Builder.createLabel(Msg.License_, new Point(12, 197), this);
            license = Builder.createMultilineTextBox(new Point(116, 194), new Size(456, 52), 5, Builder.topLeftRightAnchor, this);

            this.CancelButton = Builder.createButton(Msg.Close, new Point(416, 257), 75, 10, Builder.bottomRightAnchor, new EventHandler(this.Cancel), this);
            this.AcceptButton = Builder.createButton(Msg.Create, new Point(497, 257), 75, 11, Builder.bottomRightAnchor, new EventHandler(this.Create), this);

            this.ResumeLayout();
        }

        public void Translate()
        {
            this.Text = UI.GetMessage(Msg.CreateInstaller);
            this.save.Filter = UI.GetMessage(Msg.Installer) + "|*.exe";
        }

        private void Cancel(object sender, EventArgs e) {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        private void Create(object sender, EventArgs e)
        {
            string name = this.name.Text.Trim(), author = this.author.Text.Trim();
            if (name.Length == 0 || author.Length == 0)
            {
                Main.GetInstance().ShowError(UI.GetMessage(Msg.YouMustProvideANameAndAuthorForTheSkin), UI.GetMessage(Msg.CreateInstallerError));
            }
            else if (this.save.ShowDialog(this) == DialogResult.OK)
            {
                this.Cursor = Cursors.WaitCursor;
                Main.Threaded(this.CreateOnThread, "Creating Installer", name, author, this.url.Text.Trim(), this.desc.Text.Trim(), this.license.Text.Trim());
            }
        }

        private void CreateOnThread(object _list)
        {
            object[] list = (object[])_list;
            string name = (string)list[0], author = (string)list[1], url = (string)list[2], desc = (string)list[3], license = (string)list[4];
            string err = null;
            try
            {
                XmlDocument xml = new XmlDocument();
                XmlNode n = xml.AppendChild(xml.CreateElement("BootSkinDescription"));
                n.AppendChild(xml.CreateElement("Name")).InnerText = name;
                n.AppendChild(xml.CreateElement("Author")).InnerText = author;
                if (desc.Length > 0) n.AppendChild(xml.CreateElement("Description")).InnerText = desc;
                if (url.Length > 0) n.AppendChild(xml.CreateElement("URL")).InnerText = url;
                if (license.Length > 0) n.AppendChild(xml.CreateElement("License")).InnerText = license;
                err = Updater.CreateInstaller(Main.Preview.bs, xml, Main.Preview.GenerateInstallerImage(), Res.GetCompressedBytes("installer"), save.FileName);
            }
            catch (Exception ex) { err = ex.ToString(); }
            this.Invoke(new dFinishUpCreate(this.FinishUpCreate), err);
        }

        private delegate void dFinishUpCreate(string err);
        private void FinishUpCreate(string err)
        {
            this.Cursor = this.DefaultCursor;
            if (err != null)
                Main.GetInstance().ShowError(UI.GetMessage(Msg.ThereWasAProblemCreatingTheInstaller) + '\n' + err, UI.GetMessage(Msg.CreateInstallerError));
            else
            {
                this.Close();
                this.AcceptButton.DialogResult = DialogResult.OK;
            }
        }
    }
}
