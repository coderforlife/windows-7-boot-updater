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
    class License : CommonInstallerPage
    {
        public License(Main m) : base(Msg.LicenseAgreement, Msg.PleaseReviewTheLicenseTermsBeforeInstallingTheBootSkin, m)
        {
            Builder.createLabel(Msg.TheWindows7BootUpdaterTerms, new Point(12, 9), this.content);
            string skinLicense = Program.SkinLicense;
            bool hasSkinLicense = !string.IsNullOrEmpty(skinLicense);
            Builder.createStaticTextBox(Res.GetCompressedString("eula"), new Point(12, 25), new Size(473, hasSkinLicense ? 86 : 192), 1, this.content);
            if (hasSkinLicense)
            {
                Builder.createLabel(Msg.TheTermsForTheBootSkinBy, new string[] { Program.SkinName, Program.SkinAuthor }, new Point(12, 115), this.content);
                Builder.createStaticTextBox(skinLicense, new Point(12, 131), new Size(473, 86), 2, this.content);
            }
            Builder.createLabel(Msg.IfYouAcceptTheTermsOfTheAgreement, new Point(12, 220), this.content);

            this.content.ResumeLayout();
            this.ResumeLayout();
        }

        public override Msg NextText { get { return Msg.IAgree; } }
        public override InstallerPageID ThisPage { get { return InstallerPageID.License; } }
        public override InstallerPageID BackPage { get { return InstallerPageID.Welcome; } }
        public override InstallerPageID NextPage { get { return Registry.AlreadyInstalled() ? InstallerPageID.UninstallCurrent : InstallerPageID.ReadyToInstall; } }
    }
}
