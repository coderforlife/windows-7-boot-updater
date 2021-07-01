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
using System.Globalization;
using System.Text;
using System.Windows.Forms;

namespace Win7BootUpdater.GUI
{
    internal abstract class TranslatableTextBox : TranslatableItem
    {
        private TextBoxBase box;
        public TranslatableTextBox(TextBoxBase box) { this.box = box; }
        public void Translate()
        {
            StringBuilder sb = new StringBuilder();
            BuildText(sb);
            box.Text = sb.ToString();
        }
        public abstract void BuildText(StringBuilder sb);
    }
    internal class TranslatableAboutBox : TranslatableTextBox
    {
        public TranslatableAboutBox(TextBoxBase box) : base(box) { }
        public override void BuildText(StringBuilder sb)
        {
            sb.AppendLine(UI.GetMessage(Msg.ProblemsBugsCommentsEmailMeAt));
            sb.AppendLine(UI.GetMessage(Msg.MyHomepage, Main.HomepageLink));
            sb.AppendLine(UI.GetMessage(Msg.Windows7BootPage, Main.W7BULink));
            sb.AppendLine();
            sb.AppendLine(UI.GetMessage(Msg.LikeThisProgramDonate, Main.DonationLink));
            sb.AppendLine();
            sb.AppendLine(UI.GetMessage(Msg.ThanksTo));
            sb.AppendLine(UI.GetMessage(Msg.ForAllOfTheirInput));
            sb.AppendLine(UI.GetMessage(Msg.ForReferencePNGLibrary));
            sb.AppendLine(UI.GetMessage(Msg.ForTheMonitorPaintingLogo));
            sb.AppendLine(UI.GetMessage(Msg.AndAllThePeopleWhoTestedTheProgramAndGaveWonderfulFeedback));
        }
    }
    internal class TranslatableTranslations : TranslatableTextBox
    {
        public TranslatableTranslations(TextBoxBase box) : base(box) { }
        public override void BuildText(StringBuilder sb)
        {
            CultureInfo[] locales = UI.AvailableLocales();
            string[] trans = UI.GetTranslators();
            int len = locales.Length;
            for (int i = 0; i < len; ++i)
            {
                if (!string.IsNullOrEmpty(trans[i]))
                {
                    string msg = UI.GetMessage(Msg.TranslatedBy, UI.LocaleDisplayName(locales[i]), trans[i]);
                    if (UI.Locale == locales[i])
                        msg = "[b]" + msg + "[/b]";
                    sb.AppendLine(msg);
                }
            }
            sb.AppendLine();
            sb.AppendLine(string.Format("[{0} {1}]", Main.TranslateLink, UI.GetMessage(Msg.ContributeATranslation)));
        }
    }

    internal class About : Form, TranslatableItem
    {
        private static string eula = null;
        private TabControl tabs;
        public About()
        {
            this.SuspendLayout();
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.StartPosition = FormStartPosition.CenterParent;
            this.ClientSize = new Size(584, 292);
            this.ShowInTaskbar = false;
            this.MaximizeBox = false;
            this.Icon = Res.GetAppIcon();
            Builder.AddTranslatableItem(this);

            Builder.createPicture(new Point(12, 12), new Size(128, 128), this).Image = Res.GetImage("logo");
            Builder.createPicture(new Point(12, 152), new Size(128, 128), this).Image = Res.GetImage("c4l");
            Builder.createLabel(Msg.Windows7BootUpdater, new Point(146, 9), this).Font = (new Font(Label.DefaultFont.FontFamily, 16, FontStyle.Bold, GraphicsUnit.Point, 0));
            Builder.createRightLabel(Msg.Version, Version.Desc, new Point(572, 9), new Size(145, 15), this).Font = (new Font(Label.DefaultFont.FontFamily, 9, FontStyle.Bold, GraphicsUnit.Point, 0));
            Builder.createRightLabel(Msg.ByJeffBush, new Point(572, 24), new Size(145, 13), this);

            tabs = Builder.createTabControl(new Point(151, 38), new Size(421, 213), 1, Builder.topBottomLeftRightAnchor, this);

            Builder.AddTranslatableItem(new TranslatableAboutBox(Builder.createFullTextTab(Msg.About, true, tabs)));
            Builder.AddTranslatableItem(new TranslatableTranslations(Builder.createFullTextTab(Msg.Translations, true, tabs)));

            TabPage tab = Builder.createTab(Msg.Versions, tabs);
            Builder.createLabel(Msg.Database, "Winload", new Point(6, 6), tab);
            Builder.createLabel(new Point(146, 6), tab).Text = Updater.GetPatchVersion("winload");
            Builder.createLabel(Msg.Database, "Winresume", new Point(6, 21), tab);
            Builder.createLabel(new Point(146, 21), tab).Text = Updater.GetPatchVersion("winresume");
            Builder.createLabel(Msg.Database, "Bootmgr", new Point(6, 36), tab);
            Builder.createLabel(new Point(146, 36), tab).Text = Updater.GetPatchVersion("bootmgr");
            tab.ResumeLayout();

            if (eula == null) eula = Res.GetCompressedString("eula");
            Builder.createFullTextTab(Msg.License, false, tabs).Text = eula;

            tabs.ResumeLayout();

            this.AcceptButton = Builder.createButton(Msg.Close, new Point(494, 257), 78, 2, Builder.bottomRightAnchor, new System.EventHandler(this.close_Click), this);
            this.CancelButton = this.AcceptButton;
            this.AcceptButton.DialogResult = System.Windows.Forms.DialogResult.OK;

            this.ResumeLayout();
        }
        public void Translate() { this.Text = UI.GetMessage(Msg.Windows7BootUpdater) + " - " + UI.GetMessage(Msg.About); }
        protected void close_Click(object sender, EventArgs e) { this.Close(); }
        protected override void OnShown(EventArgs e) { base.OnShown(e); this.tabs.SelectedIndex = 0; }
    }
}
