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
using System.Collections.Generic;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;

namespace Win7BootUpdater.GUI {
    internal class TranslatablePathLabel : TranslatableItem
    {
        Label lbl;
        public TranslatablePathLabel(Label lbl) { this.lbl = lbl; }
        public void Translate()
        {
            string text = this.lbl.Text;
            if (text.Length > 2 && text[0] == '[' && text[text.Length - 1] == ']')
                this.lbl.Text = ((bool)this.lbl.Tag) ? UI.GetMessage(Msg.Embedded) : UI.GetMessage(Msg.NoneSelected);
        }
    }
    internal class TranslatableOnHiddenSystemDrive : TranslatableItem
    {
        ToolStripMenuItem item;
        public TranslatableOnHiddenSystemDrive(ToolStripMenuItem item) { this.item = item; this.item.Tag = true; }
        public void Translate() { if ((bool)this.item.Tag) this.item.Text = "bootmgr: " + UI.GetMessage(Msg.OnHiddenSystemPartition); }
    }

    internal class Main : Form
    {
        private delegate void EmptyFunc();

        private class ProgressBox : Form, TranslatableItem
        {
            private bool done;
            private Label text;
            private ProgressBar progress;
            public ProgressBox()
            {
                done = false;
                this.SuspendLayout();
                this.ClientSize = new Size(284, 60);
                this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
                Builder.AddTranslatableItem(this);
                this.MaximizeBox = false;
                this.MinimizeBox = false;
                this.ControlBox = false;
                this.ShowInTaskbar = false;
                this.StartPosition = FormStartPosition.CenterParent;
                this.text = Builder.createLabel(new Point(12, 9), this);
                this.progress = (ProgressBar)Builder.setBasic(new ProgressBar(), new Point(12, 25), new Size(260, 23), -1, Builder.topLeftRightAnchor, this);
                this.ResumeLayout();
                UI.ProgressChanged += new UI.Progress(this.ProgessChanged);
            }
            public void Translate() { this.Text = UI.GetMessage(Msg.Windows7BootUpdater); }
            public void ProgessChanged(string text, int cur, int max)
            {
                if (this.InvokeRequired) {
                    this.Invoke(new UI.Progress(this.ProgessChanged), text, cur, max);
                } else {
                    this.text.Text = text;
                    this.progress.Value = cur;
                    this.progress.Maximum = max;
                }
            }
            public void DoneAndClose()
            {
                if (this.InvokeRequired) {
                    this.Invoke(new EmptyFunc(this.DoneAndClose));
                } else {
                    this.done = true;
                    this.Hide();
                }
            }
            protected override void OnVisibleChanged(EventArgs e) { base.OnVisibleChanged(e); if (this.Visible) this.done = false; }
            protected override void OnFormClosing(FormClosingEventArgs e) { base.OnFormClosing(e); if (!this.done) e.Cancel = true; }
        }

        private class SysFileMenuItem : ToolStripMenuItem, TranslatableItem {
            public delegate uint Check(string path);

            private static OpenFileDialog openFile;

            private Check check;
            private string name, file, filter;
            private bool active;

            public SysFileMenuItem(string file, Check check, string filter, bool active)
            {
                if (openFile == null)
                {
                    openFile = new OpenFileDialog();
                    openFile.RestoreDirectory = true;
                }

                this.name = filter.Substring(0, filter.IndexOf('|'));

                this.check = check;
                this.filter = filter;
                this.active = active;

                this.File = file;

                Builder.AddTranslatableItem((TranslatableItem)this);
            }
            public void Translate() { this.Text = UI.GetMessage(Msg.Colon, name, this.file); }
            public bool IsValid() { return this.Image == Main.yes; }
            public void ProcessError()
            {
                if (!this.active) { return; }
                uint err = this.check(this.File);
                if (err == 0)
                {
                    this.Image = Main.yes;
                }
                else
                {
                    UI.ShowError(UI.GetMessage(Msg.ThereWasAProblemVerifying, name),
                        Main.GetInstance().Visible ? UI.GetMessage(Msg.SelectADifferentFile) : UI.GetMessage(Msg.GoToOptionsAndSelectAnAppropiate, name), err,
                        UI.GetMessage(Msg.ErrorWith, name));
                    this.Image = Main.no;
                }
            }
            protected override void OnClick(EventArgs e)
            {
                openFile.FileName = File;
                openFile.Filter = this.filter + "|" + UI.GetMessage(Msg.AllFiles) + "|*.*";
                if (openFile.ShowDialog(Main.GetInstance()) == DialogResult.OK)
                {
                    this.SetFileFull(openFile.FileName);
                }
                base.OnClick(e);
            }
            public bool Active
            {
                get { return this.active; }
                set
                {
                    if (value != this.active)
                    {
                        this.active = value;
                        if (this.active)
                            this.ProcessError();
                        else
                            this.Image = null;
                    }
                }
            }
            public string File
            {
                get { return this.file; }
                /*private*/ set
                {
                    this.file = value;
                    this.Text = UI.GetMessage(Msg.Colon, name, value);
                    this.ProcessError();
                }
            }
            public void SetFileFull(string file)
            {
                this.active = true;
                this.File = file;
                if (this.Tag != null)
                {
                    ((ToolStripMenuItem)this.Tag).Image = null;
                    this.OwnerItem.Image = this.Image;
                    this.OwnerItem.Text = this.Text.Trim('.');
                    this.OwnerItem.Tag = false;
                }
            }
        }

        public const string HomepageLink = "http://www.coderforlife.com/";
        public const string W7BULink = "http://www.coderforlife.com/projects/win7boot/";
        public const string TranslateLink = "http://www.coderforlife.com/projects/win7boot/translate/";
        public const string DonationLink = "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=T8JXGRGG9JDGQ&lc=US&item_name=Coder%20For%20Life&item_number=bootupdater&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donate_LG%2egif%3aNonHosted";

        public static void Threaded(ParameterizedThreadStart s, string name, params object[] o) { Thread t = new Thread(s); t.Name = name; t.Start(o.Length == 1 ? o[0] : o); }
        public static void Threaded(ThreadStart s, string name) { Thread t = new Thread(s); t.Name = name; t.Start(); }

        #region Menus
        private SysFileMenuItem bootres, winload, winloadMui, winresume, winresumeMui, bootmgr;
        private void createMenu()
        {
            MenuStrip menu = new MenuStrip();
            menu.SuspendLayout();
            menu.Items.Add(Builder.createMenuItem(Msg.File,
                Builder.createMenuItem(Msg.LoadBootSkin, new EventHandler(this.load)),
                Builder.createMenuItem(Msg.SaveBootSkinAs, new EventHandler(this.save)),
                Builder.createMenuItem(Msg.CreateBootSkinInstaller, new EventHandler(this.showCreateInstaller)),
                new ToolStripSeparator(),
                Builder.createMenuItem(Msg.Exit, new EventHandler(this.exit))
            ));
            this.bootres = new SysFileMenuItem(Bootres.def, new SysFileMenuItem.Check(Bootres.Check), "bootres.dll|*.dll", true);
            this.bootres.Click += new EventHandler(update_bootres);
            this.winload = new SysFileMenuItem(Winload.def, new SysFileMenuItem.Check(Winload.Check), "winload.exe|*.exe", true);
            this.winloadMui = new SysFileMenuItem(Winload.defMui, new SysFileMenuItem.Check(Winload.CheckMui), "winload.exe.mui|*.mui", true);
            this.winresume = new SysFileMenuItem(Winresume.def, new SysFileMenuItem.Check(Winresume.Check), "winresume.exe|*.exe", true);
            this.winresumeMui = new SysFileMenuItem(Winresume.defMui, new SysFileMenuItem.Check(Winresume.CheckMui), "winresume.exe.mui|*.mui", true);

            ToolStripMenuItem bm;
            bool hid = Bootmgr.DefaultIsOnHiddenSystemPartition();
            this.bootmgr = new SysFileMenuItem(hid ? Bootmgr.defFallBack : Bootmgr.def, new SysFileMenuItem.Check(Bootmgr.Check), "bootmgr|bootmgr", !hid);
            if (hid)
            {
                this.bootmgr.Tag = bm = Builder.createMenuItem(new EventHandler(this.setBootmgrToHiddenDrive));
                Builder.AddTranslatableItem(new TranslatableOnHiddenSystemDrive(bm));
                bm.Image = Main.yes;
                bm = Builder.createMenuItem(bm, this.bootmgr);
                Builder.AddTranslatableItem(new TranslatableOnHiddenSystemDrive(bm));
                bm.Image = Main.yes;
            }
            else
            {
                bm = this.bootmgr;
            }

            menu.Items.Add(Builder.createMenuItem(Msg.Options_Menu,
                Builder.createMenuItem(Msg.SelectWindowsFolder_Menu, new EventHandler(this.selectWindowsFolder)),
                new ToolStripSeparator(),
                this.bootres, this.winload, this.winloadMui, this.winresume, this.winresumeMui, bm,
                new ToolStripSeparator(),
                Builder.createMenuItem(Msg.RestoreBackups, new EventHandler(this.restore))
            ));

            ToolStripMenuItem lang = Builder.createMenuItem(Msg.Language);
            CultureInfo locale = UI.Locale;
            foreach (CultureInfo c in UI.AvailableLocales())
            {
                ToolStripMenuItem l = Builder.createMenuItem(UI.LocaleDisplayName(c), new EventHandler(this.setLanguage));
                l.Checked = c.Equals(locale);
                l.Tag = c;
                lang.DropDownItems.Add(l);
            }
            lang.DropDownItems.Add(new ToolStripSeparator());
            lang.DropDownItems.Add(Builder.createMenuItem(Msg.ContributeATranslation, TranslateLink));
            menu.Items.Add(lang);

            menu.Items.Add(Builder.createMenuItem(Msg.Help,
                Builder.createMenuItem(Msg.About_Menu, new EventHandler(this.showAbout)),
                //Builder.createMenuItem(Msg.CheckForUpdates, new EventHandler(this.checkForUpdates)),
                Builder.createMenuItem(Msg.Windows7BootUpdaterWebpage, W7BULink),
                Builder.createMenuItem(Msg.Donate_Menu, DonationLink)
            ));

            this.Controls.Add(menu);
            menu.ResumeLayout();
        }
        private static string IfPathExists(string path) { return File.Exists(path) ? path : null; }
        private void selectWindowsFolder(object sender, EventArgs e)
        {
            SelectFolderDialog d = new SelectFolderDialog();
            //d.Description = UI.GetMessage(Msg.SelectTheFolderThatContainsTheAnimation) + " " + Animation.DirDesc;
            if (d.ShowDialog() == DialogResult.OK)
            {
                string bootmgr = null, bootres = null, winload = null, winloadMui = null, winresume = null, winresumeMui = null;

                Updater.DisableFSRedirection();

                string win = d.SelectedPath;
                string sys32 = Path.Combine(win, "System32");
                string parent = Path.GetDirectoryName(win);

                bootmgr = IfPathExists(Path.Combine(parent, "bootmgr"));

                if (Directory.Exists(sys32))
                {
                    bootres = IfPathExists(Path.Combine(sys32, "bootres.dll"));
                    winload = IfPathExists(Path.Combine(sys32, "winload.exe"));
                    winresume = IfPathExists(Path.Combine(sys32, "winresume.exe"));

                    string sys32l = Path.Combine(sys32, Updater.GetPreferredLocale());
                    if (!Directory.Exists(sys32l)) sys32l = Path.Combine(sys32, "en-US");
                    if (Directory.Exists(sys32l))
                    {
                        winloadMui   = IfPathExists(Path.Combine(sys32l, "winload.exe.mui"));
                        winresumeMui = IfPathExists(Path.Combine(sys32l, "winresume.exe.mui"));
                    }

                    if (winloadMui == null)   winloadMui   = IfPathExists(Path.Combine(sys32, "winload.exe.mui"));
                    if (winresumeMui == null) winresumeMui = IfPathExists(Path.Combine(sys32, "winresume.exe.mui"));
                }

                if (bootmgr == null)
                {
                    string boot = Path.Combine(win, "Boot"), pcat = Path.Combine(boot, "PCAT");
                    if (Directory.Exists(pcat))
                        bootmgr = IfPathExists(Path.Combine(pcat, "bootmgr"));
                }

                if (bootmgr == null)      bootmgr      = IfPathExists(Path.Combine(win, "bootmgr"));
                if (bootres == null)      bootres      = IfPathExists(Path.Combine(win, "bootres.dll"));
                if (winload == null)      winload      = IfPathExists(Path.Combine(win, "winload.exe"));
                if (winloadMui == null)   winloadMui   = IfPathExists(Path.Combine(win, "winload.exe.mui"));
                if (winresume == null)    winresume    = IfPathExists(Path.Combine(win, "winresume.exe"));
                if (winresumeMui == null) winresumeMui = IfPathExists(Path.Combine(win, "winresume.exe.mui"));

                Updater.RevertFSRedirection();

                List<string> updated = new List<string>(5);
                if (bootmgr != null)      { this.bootmgr.SetFileFull(bootmgr);           updated.Add("bootmgr"); }
                if (bootres != null)      { this.bootres.SetFileFull(bootres);           updated.Add("bootres.dll"); }
                if (winload != null)      { this.winload.SetFileFull(winload);           updated.Add("winload.exe");}
                if (winloadMui != null)   { this.winloadMui.SetFileFull(winloadMui);     updated.Add("winload.exe.mui");}
                if (winresume != null)    { this.winresume.SetFileFull(winresume);       updated.Add("winresume.exe");}
                if (winresumeMui != null) { this.winresumeMui.SetFileFull(winresumeMui); updated.Add("winresume.exe.mui"); }

                if (updated.Count == 0)
                    ShowMessage(UI.GetMessage(Msg.NoAcceptableFilesWereFound), UI.GetMessage(Msg.SelectWindowsFolder), MessageBoxIcon.None);
                else
                    ShowMessage(UI.GetMessage(Msg.TheFollowingFilesWereFound, string.Join(", ", updated.ToArray())), UI.GetMessage(Msg.SelectWindowsFolder), MessageBoxIcon.None);
            }
        }
        private void setLanguage(object sender, EventArgs e)
        {
            ToolStripMenuItem m = (ToolStripMenuItem)sender;
            ToolStripMenuItem lang = (ToolStripMenuItem)m.OwnerItem;
            foreach (ToolStripItem i in lang.DropDownItems)
            {
                if (typeof(ToolStripMenuItem).IsAssignableFrom(i.GetType()))
                    ((ToolStripMenuItem)i).Checked = i.Equals(m);
            }
            UI.Locale = (CultureInfo)m.Tag;

            // Update the GUI for the new locale
            Builder.TranslateAllItems();
            Preview.UpdateInstructionLanguages();
            this.openFile.Filter = UI.GetMessage(Msg.ImageFiles) + "|*.png;*.bmp;*.gif;*.jpg;*.jpeg;*.tif;*.tiff";
            this.updateFrameTootip((uint)frames.Value);
            this.playPauseChanged(preview.Playing);
        }
        private void setBootmgrToHiddenDrive(object sender, EventArgs e)
        {
            this.bootmgr.Active = false;
            ToolStripMenuItem t = (ToolStripMenuItem)sender;
            t.Image = Main.yes;
            t.OwnerItem.Text = t.Text;
            t.OwnerItem.Tag = true;
            t.OwnerItem.Image = Main.yes;
        }
        private void update_bootres(object sender, EventArgs e) { Preview.SetDefaultAnimSource(bootres.File); }
        private void load(object sender, EventArgs e)
        {
            if (prepareForLoad(sender, e))
            {
                OpenFileDialog d = new OpenFileDialog();
                d.Filter = UI.GetMessage(Msg.BootSkinForWindows7) + "|*.bs7";
                d.RestoreDirectory = true;
                if (d.ShowDialog(this) == DialogResult.OK)
                    this.load(d.FileName);
            }
        }
        private bool prepareForLoad(object sender, EventArgs e)
        {
            preview.Pause();
            if (this.edited)
            {
                DialogResult r = MessageBox.Show(this.TopForm(), UI.GetMessage(Msg.DoYouWishToSaveTheCurrentSettingsBeforeLoadingADifferentBootSkin),
                    UI.GetMessage(Msg.SaveModifications), MessageBoxButtons.YesNoCancel, MessageBoxIcon.Warning);
                if (r == DialogResult.Cancel)
                    return false;
                else if (r == DialogResult.Yes)
                    this.save(sender, e);
            }
            return true;
        }
        private void save(object sender, EventArgs e)
        {
            preview.Pause();
            SaveFileDialog d = new SaveFileDialog();
            d.Filter = UI.GetMessage(Msg.BootSkinForWindows7) + "|*.bs7";
            d.RestoreDirectory = true;
            if (d.ShowDialog(this) == DialogResult.OK)
            {
                this.SetWaiting(true);
                Threaded(this.saveOnThread, "Saving Boot Skin", d.FileName);
            }
        }
        private void saveOnThread(object file)  {
            string err;
            lock (Main.loading_saving_lock)
            {
                err = preview.bs.Save((string)file);
            }
            this.SetWaiting(false);
            this.edited = false;
            if (err != null)
                ShowError(err, UI.GetMessage(Msg.SaveError));
        }
        private void exit(object sender, EventArgs e) { this.Close(); }
        //private void checkForUpdates(object sender, EventArgs e) { checkForUpdates(true); }
        private void showAbout(object sender, EventArgs e) { this.preview.Pause(); if (this.about == null) this.about = new About(); about.ShowDialog(this); }
        private void showCreateInstaller(object sender, EventArgs e) { this.preview.Pause(); if (this.createInstaller == null) this.createInstaller = new CreateInstaller(); this.createInstaller.ShowDialog(this); }
        #endregion

        private static uint GetWinIndex(object sender, Control[] list) { return (list[0] == sender) ? 0u : ((list[1] == sender) ? 1u : 2u); }
        private static uint GetWinIndex(object sender, Control[,] list, uint id) { return (list[0, id] == sender) ? 0u : ((list[1, id] == sender) ? 1u : 2u); }
        private static uint GetId(object c) { return (uint)((Control)c).Tag; }

        private bool edited;
        private ToolTip toolTip;
        private ColorDialog colorPicker;
        private OpenFileDialog openFile;
        private About about;
        private CreateInstaller createInstaller;
        private ProgressBox progress;

        #region Animation Box
        private int[] last_selected = new int[2];
        private GroupBox[] grpAnim = new GroupBox[2];
        private ComboBox[] comboAnim = new ComboBox[2];
        private Label[] lblAnim = new Label[2];
        private PictureBox[] animStatus = new PictureBox[2];
        private void createAnimationBox(TabPage t, uint win_index)
        {
            this.grpAnim[win_index] = Builder.createGroupBox(Msg.Animation, new Point(6, 6), new Size(254, 72), 0, Builder.topLeftRightAnchor, t);
            this.comboAnim[win_index] = Builder.createDropDown(new Msg[]{ (win_index == 1 ? Msg.Same : Msg.Default), Msg.StaticImage, Msg.Animation_Key },
                new Point(6, 19), 242, 0, Builder.bottomLeftRightAnchor, new EventHandler(this.selectAnimation), this.grpAnim[win_index]);
            makeReselectable(this.animStatus[win_index] = Builder.createPicture(new Point(6, 50), this.grpAnim[win_index]));
            makeReselectable(this.lblAnim[win_index] = Builder.createLabel(new Point(27, 51), this.grpAnim[win_index]));
            Builder.AddTranslatableItem(new TranslatablePathLabel(this.lblAnim[win_index]));
            this.grpAnim[win_index].ResumeLayout();
        }
        private void makeReselectable(Control c)
        {
            c.Click += new EventHandler(this.reselectAnimation);
            c.Cursor = Cursors.Hand;
        }
        private void setAnim(uint index)
        {
            if (this.syncing || this.preview.IsFullscreen()) { return; }

            this.preview.Pause();
            this.SetWaiting(true);

            bool revert = false;

            switch (this.comboAnim[index].SelectedIndex)
            {
                case 0: // Default / Same
                    animStatus[index].Visible = false;
                    lblAnim[index].Visible = false;
                    if (index == 1) this.preview.bs.get_WinXXX(index).UseWinloadAnim();
                    else this.preview.bs.get_WinXXX(index).UseDefaultAnim();
                    finishedSettingAnim(false, index);
                    break;
                case 1: // Static
                    openFile.FileName = null;
                    if (!(revert = openFile.ShowDialog(this) != DialogResult.OK))
                        Threaded(createAnimFromSingleOnThread, "Create Anim From Single", openFile.FileName, index);
                    break;
                case 2: // Animation
                    SelectFolderDialog d = new SelectFolderDialog();
                    d.Description = UI.GetMessage(Msg.SelectTheFolderThatContainsTheAnimation) + " " + Animation.DirDesc;
                    if (!(revert = d.ShowDialog(this) != DialogResult.OK))
                        Threaded(createAnimFromFolderOnThread, "Create Anim From Folder", d.SelectedPath, index);
                    break;
            }

            if (revert)
                finishedSettingAnim(true, index);
        }
        private void createAnimFromSingleOnThread(object _list)
        {
            object[] list = (object[])_list;
            string file = (string)list[0];
            Image img = Animation.CreateFromSingle(file);
            this.Invoke(new dFinishedLoadingAnim(this.finishedLoadingAnim), (uint)list[1], img, file);
        }
        private void createAnimFromFolderOnThread(object _list)
        {
            object[] list = (object[])_list;
            string path = (string)list[0];
            Image img = Animation.CreateFromFolder(path);
            this.Invoke(new dFinishedLoadingAnim(this.finishedLoadingAnim), (uint)list[1], img, path);
        }
        private delegate void dFinishedLoadingAnim(uint index, Image img, string path);
        private void finishedLoadingAnim(uint index, Image img, string path)
        {
            animStatus[index].Image = (img != null) ? yes : no;
            animStatus[index].Visible = true;
            lblAnim[index].Text = path;
            lblAnim[index].Visible = true;
            this.preview.bs.get_WinXXX(index).Anim = img;
            finishedSettingAnim(false, index);
        }
        private void finishedSettingAnim(bool revert, uint index)
        {
            this.SetWaiting(false);
            if (revert)
            {
                syncing = true;
                this.comboAnim[index].SelectedIndex = last_selected[index];
                syncing = false;
            }
            else
            {
                last_selected[index] = this.comboAnim[index].SelectedIndex;
                this.preview.RestartAnimation();
                this.edited = true;
            }
        }
        private void reselectAnimation(object sender, EventArgs e)
        {
            uint index = GetWinIndex(sender, this.lblAnim);
            if (index >= 2u) index = GetWinIndex(sender, this.animStatus);
            setAnim(index);
        }
        private void selectAnimation(object sender, EventArgs e)
        {
            setAnim(GetWinIndex(sender, this.comboAnim));
        }
        #endregion

        #region Background Color Area
        private Button[] butBg = new Button[2];
        private void createBackgroundArea(TabPage t, uint win_index)
        {
            Builder.createLabel(Msg.Background_Key, new Point(6, 89), t);
            this.butBg[win_index] = Builder.createButton(new Point(116, 84), 144, 1, Builder.topLeftRightAnchor, new System.EventHandler(this.changeBackground), t);
        }
        private void changeBackground(object sender, EventArgs e)
        {
            uint index = GetWinIndex(sender, butBg);
            this.preview.Pause();
            colorPicker.AllowFullOpen = false;
            colorPicker.CustomColors = WinXXX.BackgroundColors; // make sure these are the custom colors
            colorPicker.Color = this.preview.bs.get_WinXXX(index).BackColor;
            if (colorPicker.ShowDialog(this) == DialogResult.OK)
            {
                this.preview.bs.get_WinXXX(index).BackColor = colorPicker.Color;
                butBg[index].BackColor = this.preview.bs.get_WinXXX(index).BackColor; // BootSkin.BackColor forces to a possible color
                this.preview.Invalidate();
                this.edited = true;
            }
            colorPicker.AllowFullOpen = true;
        }
        #endregion

        #region Method Area
        //private RadioButton[] radioMethodSimple = new RadioButton[2], radioMethodPro = new RadioButton[2];
        private ComboBox[] comboMethod = new ComboBox[2];
        private Image[] backupImage = new Image[2];
        private void createMethodArea(TabPage t, uint win_index)
        {
            Builder.createLabel(Msg.Method, new Point(6, 117), t);
            this.comboMethod[win_index] = Builder.createDropDown(new Msg[] { Msg.Simple, Msg.Complete }, new Point(116, 113), 144, 2, Builder.bottomLeftRightAnchor, new EventHandler(this.selectMethod), t);
        }
        private void selectMethod(object sender, EventArgs e)
        {
            if (this.syncing || this.preview.IsFullscreen()) { return; }

            uint index = GetWinIndex(sender, this.comboMethod);
            bool simple = this.comboMethod[index].SelectedIndex == 0;
            this.grpMsgs[index].Visible = simple;
            this.grpBg[index].Visible = !simple;
            this.preview.bs.get_WinXXX(index).Background = simple ? null : this.backupImage[index];
            this.preview.Invalidate();
            this.edited = true;

        }
        #endregion

        #region Messages Box
        private GroupBox[] grpMsgs = new GroupBox[2];
        private ComboBox[] comboCount = new ComboBox[2];
        private Button[] butBgColor = new Button[2];
        private TabControl[] tabMessages = new TabControl[2];

        private TabPage[,] tabPage = new TabPage[2,2];
        private TextBox[,] txtMsg = new TextBox[2,2];
        private Button[,] butColor = new Button[2,2];
        private NumericUpDown[,] numVert = new NumericUpDown[2,2], numSize = new NumericUpDown[2,2];

        private void createMessagesBox(TabPage t, uint win_index)
        {
            this.grpMsgs[win_index] = Builder.createGroupBox(Msg.Messages, new Point(6, 140), new Size(254, 220), 4, Builder.topLeftRightAnchor, t);

            Builder.createLabel(Msg.Count, new Point(6, 22), this.grpMsgs[win_index]);
            this.comboCount[win_index] = Builder.createDropDown(new Msg[] { Msg.None, Msg.Message_1, Msg.Messages_2 }, new Point(110, 19), 138, 0, Builder.topLeftRightAnchor, new EventHandler(this.changeMsgCount), this.grpMsgs[win_index]);
            Builder.createLabel(Msg.Background_Key, new Point(6, 51), this.grpMsgs[win_index]);
            this.butBgColor[win_index] = Builder.createButton(new Point(110, 46), 138, 2, Builder.topLeftRightAnchor, new EventHandler(this.changeMsgBgColor), this.grpMsgs[win_index]);

            this.tabMessages[win_index] = Builder.createTabControl(new Point(6, 75), new Size(242, 137), 3, Builder.topLeftRightAnchor, this.grpMsgs[win_index]);
            for (uint i = 0u; i < 2u; ++i)
            {
                int tabStart = (i==0u) ? 0 : 4;
                this.tabPage[win_index,i] = Builder.createTab(Msg.Message, (i+1).ToString(), this.tabMessages[win_index]);

                Builder.createLabel(Msg.Text, new Point(6, 9), this.tabPage[win_index, i]);
                this.txtMsg[win_index, i] = Builder.createTextBox(new Point(100, 6), 128, tabStart, Builder.topLeftRightAnchor, this.tabPage[win_index, i]);
                this.txtMsg[win_index, i].TextChanged += new EventHandler(this.changeMsgText);
                this.txtMsg[win_index, i].Tag = i;

                Builder.createLabel(Msg.FontColor, new Point(6, 37), this.tabPage[win_index, i]);
                this.butColor[win_index, i] = Builder.createButton(new Point(100, 32), 128, tabStart + 1, Builder.topLeftRightAnchor, new EventHandler(this.changeMsgColor), this.tabPage[win_index, i]);
                this.butColor[win_index, i].Tag = i;

                Builder.createLabel(Msg.Position, new Point(6, 63), this.tabPage[win_index, i]);
                this.numVert[win_index, i] = Builder.createNumField(0, 0, 768, new Point(100, 61), 128, tabStart + 2, Builder.topLeftRightAnchor, new EventHandler(this.changeMsgPosition), this.tabPage[win_index, i]);
                this.numVert[win_index, i].Tag = i;

                Builder.createLabel(Msg.FontSize, new Point(6, 89), this.tabPage[win_index, i]);
                this.numSize[win_index, i] = Builder.createNumField(1, 1, 120, new Point(100, 87), 128, tabStart + 3, Builder.topLeftRightAnchor, new EventHandler(this.changeMsgSize), this.tabPage[win_index, i]);
                this.numSize[win_index, i].Tag = i;

                this.tabPage[win_index, i].ResumeLayout();
            }
            this.tabMessages[win_index].ResumeLayout();
            this.grpMsgs[win_index].ResumeLayout();
        }
        private void changeMsgCount(object sender, EventArgs e)
        {
            uint index = GetWinIndex(sender, comboCount);
            int count = comboCount[index].SelectedIndex;
            for (int i = 0; i < tabMessages[index].TabCount; ++i)
                tabMessages[index].Controls[i].Enabled = i < count;
            this.preview.bs.get_WinXXX(index).MessageCount = count;
            this.preview.Invalidate();
            this.edited = true;
        }
        private void changeMsgBgColor(object sender, EventArgs e)
        {
            uint index = GetWinIndex(sender, butBgColor);
            this.preview.Pause();
            colorPicker.Color = this.preview.bs.get_WinXXX(index).MessageBackColor;
            if (colorPicker.ShowDialog(this) == DialogResult.OK) {
                butBgColor[index].BackColor = colorPicker.Color;
                this.preview.bs.get_WinXXX(index).MessageBackColor = colorPicker.Color;
                this.preview.Invalidate();
                this.edited = true;
            }
        }
        private void changeMsgText(object sender, EventArgs e) {
            uint i = GetId(sender);
            uint index = GetWinIndex(sender, txtMsg, i);
            this.preview.bs.get_WinXXX(index).set_Message(i, txtMsg[index,i].Text); // silly that indexers with managed classes don't work
            this.preview.Invalidate();
            this.edited = true;
        }
        private void changeMsgColor(object sender, EventArgs e)
        {
            this.preview.Pause();
            uint i = GetId(sender);
            uint index = GetWinIndex(sender, butColor, i);
            colorPicker.Color = this.preview.bs.get_WinXXX(index).get_TextColor(i);
            if (colorPicker.ShowDialog(this) == DialogResult.OK) {
                butColor[index,i].BackColor = colorPicker.Color;
                this.preview.bs.get_WinXXX(index).set_TextColor(i, colorPicker.Color);
                this.preview.Invalidate();
                this.edited = true;
            }
        }
        private void changeMsgPosition(object sender, EventArgs e) {
            uint i = GetId(sender);
            uint index = GetWinIndex(sender, numVert, i);
            this.preview.bs.get_WinXXX(index).set_Position(i, (int)numVert[index,i].Value);
            this.preview.Invalidate();
            this.edited = true;
        }
        private void changeMsgSize(object sender, EventArgs e) {
            uint i = GetId(sender);
            uint index = GetWinIndex(sender, numSize, i);
            this.preview.bs.get_WinXXX(index).set_TextSize(i, (int)numSize[index, i].Value);
            this.preview.Invalidate();
            this.edited = true;
        }
        #endregion

        #region Background Image Box
        private GroupBox[] grpBg = new GroupBox[2];
        private Button[] butBgImg = new Button[2];
        private Label[] lblBg =  new Label[2];
        private PictureBox[] bgStatus = new PictureBox[2];
        private void createBackgroundBox(TabPage t, uint win_index)
        {
            this.grpBg[win_index] = Builder.createGroupBox(Msg.Background, new Point(6, 140), new Size(254, 70), 5, Builder.topLeftRightAnchor, t);
            this.butBgImg[win_index] = Builder.createButton(Msg.SelectBackgroundImage, new Point(6, 19), 242, 0, Builder.topLeftRightAnchor, new EventHandler(this.changeBackgroundImage), grpBg[win_index]);
            this.bgStatus[win_index] = Builder.createPicture(new Point(6, 48), grpBg[win_index]);
            this.lblBg[win_index] = Builder.createLabel(new Point(27, 49), grpBg[win_index]);
            Builder.AddTranslatableItem(new TranslatablePathLabel(this.lblBg[win_index]));
            this.grpBg[win_index].ResumeLayout();
        }
        private void changeBackgroundImage(object sender, EventArgs e)
        {
            uint index = GetWinIndex(sender, butBgImg);
            this.preview.Pause();
            openFile.FileName = null;
            if (openFile.ShowDialog(this) == DialogResult.OK) {
                this.SetWaiting(true);
                Threaded(loadBackgroundImageOnThread, "Load Background Image", openFile.FileName, index);
            }
        }
        private void loadBackgroundImageOnThread(object _list)
        {
            object[] list = (object[])_list;
            string file = (string)list[0];

            Stream s = null;
            Bitmap img = null;
            try
            {
                // we don't want to lock the file
                img = new Bitmap(s = File.OpenRead(file));
            }
            catch (Exception) { }
            finally { if (s != null) s.Close(); }

            this.Invoke(new dFinishedLoadingBackgroundImage(this.finishedLoadingBackgroundImage), (uint)list[1], img, file);
        }
        private delegate void dFinishedLoadingBackgroundImage(uint index, Bitmap img, string file);
        private void finishedLoadingBackgroundImage(uint index, Bitmap img, string file)
        {
            try
            {
                if ((img.Flags & 0x1000) != 0) // ImageFlagsHasRealDPI
                {
                    Graphics g = this.CreateGraphics();
                    img.SetResolution(g.DpiX, g.DpiY);
                }
            }
            catch (Exception) { }
            bgStatus[index].Image = (img != null) ? yes : no;
            lblBg[index].Text = file;
            this.preview.bs.get_WinXXX(index).Background = (img != null) ? img : Preview.Invalid;
            this.backupImage[index] = this.preview.bs.get_WinXXX(index).Background;
            this.preview.Invalidate();
            this.SetWaiting(false);
            this.edited = true;
        }
        #endregion

        #region Preview Area
        private Preview preview;
        private TrackBar frames;
        private Button butPlay, butFullscreen;
        private void createPreviewArea()
        {
            this.preview = (Preview)Builder.setBasic(new Preview(), new Point(290, 12+24), new Size(512, 384), -1, Builder.topRightAnchor, this);
            this.preview.PlayPauseChanged += new Preview.PlayPauseHandler(this.playPauseChanged);
            this.preview.FrameChanged += new Preview.FrameChangedHandler(this.frameChanged);
            this.preview.AllowDrop = true;
            this.preview.DragEnter += new DragEventHandler(this.dragEnter);
            this.preview.DragDrop += new DragEventHandler(this.dragDrop);
            this.preview.DoubleClick += new EventHandler(this.fullscreen);

            this.frames = (TrackBar)Builder.setBasic(new TrackBar(), new Point(290, 423+24), new Size(431, 45), 17, Builder.topRightAnchor, this);
            this.frames.Minimum = 1;
            this.frames.Maximum = Animation.Frames;
            this.frames.Value = 1;
            this.frames.ValueChanged += new EventHandler(this.changeFrame);
            this.frames.TickStyle = TickStyle.TopLeft;
            //this.frames.TickFrequency = 2;

            Point l = this.frames.Location;
            l.Offset(13, -20);
            int w = this.frames.ClientSize.Width - 26;
            int x = (int)((Animation.LoopFrame - 0.5) * w / Animation.Frames);
            Builder.createTextPanel(Msg.Straight, new Point(l.X, l.Y), new Size(x, 20), Color.FromArgb(128, 255, 128), this);
            Builder.createTextPanel(Msg.Loop, new Point(l.X + x, l.Y), new Size(w - x, 20), Color.FromArgb(128, 128, 255), this);
            this.butPlay = Builder.createButton(Msg.Play, new Point(727, 402 + 24), 75, 15, Builder.topRightAnchor, new EventHandler(this.play), this);
            this.butFullscreen = Builder.createButton(Msg.Fullscreen, new Point(727, 431 + 24), 75, 16, Builder.topRightAnchor, new EventHandler(this.fullscreen), this);
        }
        private void playPauseChanged(bool playing) { butPlay.Text = UI.GetMessage(playing ? Msg.Pause : Msg.Play); }
        private void updateFrameTootip(uint frame)
        {
            if (!this.preview.IsFullscreen())
            {
                this.toolTip.SetToolTip(this.frames, UI.GetMessage(Msg.Frame) + ": " + frame);
                this.toolTip.SetToolTip(this.preview, UI.GetMessage(Msg.Frame) + ": " + frame);
            }
        }
        private void frameChanged(uint frame)
        {
            frames.Value = (int)frame;
            updateFrameTootip(frame);
        }
        private void changeFrame(object sender, EventArgs e) { this.preview.Frame = (uint)frames.Value; }
        private void play(object sender, EventArgs e) { this.preview.PlayPause(); }
        private void fullscreen(object sender, EventArgs e)
        {
            this.toolTip.SetToolTip(this.preview, null);
            preview.Fullscreen();
        }
        private void switchDisplay(object sender, TabControlEventArgs e)
        {
            preview.WinXXXActive = (uint)e.TabPageIndex;
            preview.Invalidate();
        }
        #endregion

        [DllImport("user32.dll", SetLastError = true)]
        private static extern IntPtr ChangeWindowMessageFilter(uint message, uint dwFlag);
        private const uint MSGFLT_ADD = 1;
        private const uint WM_COPYGLOBALDATA = 0x0049, WM_COPYDATA = 0x004A, WM_DROPFILES = 0x0233;

        /*private void checkForUpdates(bool showIfOkay)
        {
            this.SetWaiting(true);
            Threaded(checkForUpdatesOnThread, "Check for Updates", showIfOkay);
        }
        private void checkForUpdatesOnThread(object showIfOkay)
        {
            System.Version v = Updater.GetLatestAvailableVersion();
            if (v != null) ShowMessage(UI.GetMessage(Msg.YouAreNotRunningTheLatestVersion, v.ToString()), UI.GetMessage(Msg.VersionCheck), MessageBoxIcon.Warning);
            else if ((bool)showIfOkay) ShowMessage(UI.GetMessage(Msg.YouAreRunningTheLatestVersion), UI.GetMessage(Msg.VersionCheck), MessageBoxIcon.None);
            this.SetWaiting(false);
        }*/

        private static object loading_saving_lock = new object();

        public static Image yes, no;

        private static Main main;
        public static Main GetInstance() { return main; }
        public static Preview Preview { get { return GetInstance().preview; } }

        private Button butApply;
        public Main(string[] args)
        {
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
                throw new Exception(UI.GetMessage(Msg.FailedToEnableTheTakeOwnershipPrivilege));
            }

            // Singleton setup
            main = this;

            // Load basic graphics
            yes = Res.GetImage("yes");
            no = Res.GetImage("no");

            // To enable drag-n-drop on Vista / 7 with UAC enabled (this may not work)
            ChangeWindowMessageFilter(WM_COPYGLOBALDATA, MSGFLT_ADD);
            ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
            ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);

            // Setup locale for the preview
            Preview.UpdateInstructionLanguages();

            this.SuspendLayout();
            this.ClientSize = new Size(814, 464);
            this.MaximizeBox = false;
            this.MaximumSize = new Size(10000, 525);
            this.MinimumSize = new Size(830, 525);
            this.Icon = Res.GetAppIcon();
            Builder.AddTranslatableItem(new TranslatableControl(Msg.Windows7BootUpdater, this));

            createMenu();
            TabControl winxxx_tabs = Builder.createTabControl(new Point(12, 36), new Size(272, 388), 2, Builder.topLeftRightAnchor, this);
            winxxx_tabs.Selected += new TabControlEventHandler(this.switchDisplay);
            TabPage[] winxxx_pages = new TabPage[2];
            for (uint i = 0; i < 2; ++i)
            {
                winxxx_pages[i] = Builder.createTab(i == 0 ? Msg.Booting : Msg.Resuming, winxxx_tabs);
                createAnimationBox(winxxx_pages[i], i);
                createBackgroundArea(winxxx_pages[i], i);
                createMethodArea(winxxx_pages[i], i);
                createMessagesBox(winxxx_pages[i], i);
                createBackgroundBox(winxxx_pages[i], i);
                winxxx_pages[i].ResumeLayout();
            }
            winxxx_tabs.ResumeLayout();

            this.butApply = Builder.createButton(Msg.Apply, new Point(12, 448), 272, 10, Builder.bottomLeftRightAnchor, new EventHandler(this.apply), this);
            this.butApply.Font = new Font(this.butApply.Font.FontFamily, 12.0f, FontStyle.Bold);
            this.butApply.Height = 30;
            createPreviewArea();
            this.AcceptButton = this.butApply;

            this.toolTip = new ToolTip();
            this.colorPicker = new ColorDialog();
            this.colorPicker.FullOpen = true;
            this.colorPicker.CustomColors = WinXXX.BackgroundColors;
            this.openFile = new OpenFileDialog();
            this.openFile.Filter = UI.GetMessage(Msg.ImageFiles)+"|*.png;*.bmp;*.gif;*.jpg;*.jpeg;*.tif;*.tiff";
            this.openFile.RestoreDirectory = true;
            this.backupImage[0] = Preview.Invalid;
            this.backupImage[1] = Preview.Invalid;

            this.ResumeLayout();
            this.CreateHandle();

            this.load(args.Length > 0 ? args[0] : null);
        }

        #region Messaging Functions
        private delegate void dWaiting(bool yes);
        private void SetWaiting(bool on)
        {
            if (this.InvokeRequired)
                this.Invoke(new dWaiting(this.SetWaiting), on);
            else
            {
                //this.Cursor = on ? Cursors.WaitCursor : this.DefaultCursor;
                this.UseWaitCursor = on;
                this.butApply.Enabled = !on;
            }
        }

        private delegate void dShow(string text, string caption, MessageBoxIcon icon);
        private Form TopForm()
        {
            if (progress != null && progress.Visible)                       return progress;
            else if (createInstaller != null && createInstaller.Visible)    return createInstaller;
            else if (about != null && about.Visible)                        return about;
            return this;
        }
        public void ShowMessage(string text, string caption, MessageBoxIcon icon)
        {
            if (this.InvokeRequired)
            {
                this.Invoke(new dShow(this.ShowMessage), text, caption, icon);
            }
            else
            {
                if (this.preview != null) preview.Pause();
                MessageBox.Show(TopForm(), text, caption, MessageBoxButtons.OK, icon);
            }
        }
        public void ShowError(string text, string caption) { ShowMessage(text, caption, MessageBoxIcon.Error); }
        /*
        public void ShowMessage(string text, string caption) { ShowMessage(text, caption, MessageBoxIcon.None); }
        public bool ShowYesNo(string text, string caption)
        {
            if (this.InvokeRequired)
            {
                return (bool)this.Invoke(new UI.YesNoMsg(this.ShowYesNo), text, caption);
            }
            else
            {
                if (this.preview != null) preview.Pause();
                return MessageBox.Show(TopForm(), text, caption, MessageBoxButtons.YesNo, MessageBoxIcon.None) == DialogResult.Yes;
            }
        }
        */
        #endregion

        #region Loading and Syncing
        private void load(string file) // if file is null, loads from winload/winresume
        {
            this.preview.Pause();
            this.SetWaiting(true);
            Threaded(loadOnThread, "Loading Boot Skin", file);
        }
        private void loadOnThread(object file)
        {
            string err;
            lock (Main.loading_saving_lock)
            {
                if (file == null)
                {
                    //preview.bs.Load(this.winload.File, this.winresume.File);
                    err = null;
                }
                else
                {
                    err = preview.bs.Load((string)file);
                }
            }
            if (this.InvokeRequired) // not required (throws an error) if the window is not yet visible
                this.Invoke(new dFinishedLoading(this.finishedloading), err);
        }
        private delegate void dFinishedLoading(string err);
        private void finishedloading(string err)
        {
            SyncWithPreview();
            this.SetWaiting(false);
            this.edited = false;
            if (err != null)
                ShowError(err, UI.GetMessage(Msg.LoadError));
        }
        private bool syncing = false;
        private void SyncWithPreview()
        {
            for (uint win_index = 0u; win_index < 2u; ++win_index)
            {
                BootSkinFile bsf = preview.bs.get_WinXXX(win_index);
                this.syncing = true;
                bool def = bsf.AnimIsNotSet();
                if (def)
                {
                    this.comboAnim[win_index].SelectedIndex = 0;
                    this.last_selected[win_index] = 0;
                }
                else
                {
                    this.comboAnim[win_index].SelectedIndex = 2;
                    this.last_selected[win_index] = 2;
                    this.animStatus[win_index].Image = yes;
                    this.lblAnim[win_index].Text = UI.GetMessage(Msg.Embedded);
                    this.lblAnim[win_index].Tag = true;
                }
                this.animStatus[win_index].Visible = !def;
                this.lblAnim[win_index].Visible = !def;

                this.butBg[win_index].BackColor = bsf.BackColor;

                this.comboCount[win_index].SelectedIndex = bsf.MessageCount;
                this.butBgColor[win_index].BackColor = bsf.MessageBackColor;
                for (uint i = 0; i < 2u; ++i)
                {
                    this.txtMsg[win_index, i].Text = bsf.get_Message(i); // silly that indexers with managed classes don't work
                    this.butColor[win_index, i].BackColor = bsf.get_TextColor(i);
                    this.numVert[win_index, i].Value = bsf.get_Position(i);
                    this.numSize[win_index, i].Value = bsf.get_TextSize(i);
                }

                bool simple = !bsf.UsesBackgroundImage();
                this.grpMsgs[win_index].Visible = simple;
                this.grpBg[win_index].Visible = !simple;
                if (simple)
                {
                    this.bgStatus[win_index].Image = no;
                    this.lblBg[win_index].Text = UI.GetMessage(Msg.NoneSelected);
                    this.lblBg[win_index].Tag = false;
                    this.comboMethod[win_index].SelectedIndex = 0;
                    this.backupImage[win_index] = Preview.Invalid;
                }
                else
                {
                    this.bgStatus[win_index].Image = yes;
                    this.lblBg[win_index].Text = UI.GetMessage(Msg.Embedded);
                    this.lblBg[win_index].Tag = true;
                    this.comboMethod[win_index].SelectedIndex = 1;
                    this.backupImage[win_index] = bsf.Background;
                }
            }

            updateFrameTootip(this.preview.Frame);
            //if (this.Visible || this.radioDefaultAnim[1].Checked)
                this.syncing = false;
        }
        #endregion

        #region Drag 'n Drop
        private void dragEnter(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop) == true)
                e.Effect = DragDropEffects.All;
        }
        private void dragDrop(object sender, DragEventArgs e)
        {
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
            load(files[0]);
        }
        #endregion

        #region Check and Restore
        private string GetSelectedBootmgrPath()
        {
            bool hid = Bootmgr.DefaultIsOnHiddenSystemPartition() && !bootmgr.Active;
            return hid ? Bootmgr.def : bootmgr.File;
        }

        private bool checkAll()
        {
            bool hid = Bootmgr.DefaultIsOnHiddenSystemPartition() && !bootmgr.Active;
            bootres.ProcessError();
            winload.ProcessError();
            winloadMui.ProcessError();
            winresume.ProcessError();
            winresumeMui.ProcessError();
            if (!hid) bootmgr.ProcessError();
            return bootres.IsValid() && winload.IsValid() && winloadMui.IsValid() && winresume.IsValid() && winresumeMui.IsValid() && (hid || bootmgr.IsValid());
        }
        private void restore(object sender, EventArgs e)
        {
            this.preview.Pause();
            string[] files = new string[] { bootres.File, winload.File, winloadMui.File, winresume.File, winresumeMui.File, GetSelectedBootmgrPath() };
            string[] sources = Updater.Restore(files);
            string s = "";
            for (int i = 0; i < files.Length; ++i) {
                if (sources[i] != null) {
                    int sp = sources[i].LastIndexOf('\\'), fp = files[i].LastIndexOf('\\');
                    if (sp == fp && sources[i].StartsWith(files[i].Remove(fp)))
                        files[i] = files[i].Substring(fp + 1);
                    if (sources[i].StartsWith("\\\\?\\Volume{"))    sources[i] = "*"+sources[i].Substring(sources[i].IndexOf('}')+1);
                    if (files  [i].StartsWith("\\\\?\\Volume{"))    files[i]   = "*"+files  [i].Substring(files  [i].IndexOf('}')+1);
                    s += UI.GetMessage(Msg.RestoredTo, sources[i], files[i]) + '\n';
                }
            }
            s = s.Trim();
            MessageBox.Show(this, s.Equals("") ? UI.GetMessage(Msg.NoFilesWereRestored) : s, UI.GetMessage(Msg.FileRestoration));
            Threaded(this.afterRestoreThread, "After Restore");
        }
        private void afterRestoreThread()
        {
            Thread.Sleep(100);
            afterRestore();
        }
        private void afterRestore()
        {
            if (this.InvokeRequired)
            {
                this.Invoke(new EmptyFunc(this.afterRestore));
            }
            else
            {
                checkAll();
                Preview.SetDefaultAnimSource(bootres.File);
            }
        }
        #endregion

        #region Updating
        private delegate void dFinishUpdate(uint error, Exception ex);
        private void apply(object sender, EventArgs e)
        {
            this.preview.Pause();
            if (!checkAll()) { return; }
            if (this.progress == null)
                this.progress = new ProgressBox();
            UI.InitProgress(Updater.TotalProgress);
            Threaded(this.update, "Updater Thread");
            this.progress.ShowDialog(this);
        }
        private void update()
        {
            uint error = 0;
            Exception ex = null;
            while (!this.progress.Visible) { Thread.Sleep(0); }
            try
            {
                error = Updater.Update(preview.bs, bootres.File, winload.File, winloadMui.File, winresume.File, winresumeMui.File, GetSelectedBootmgrPath(), true);
            }
            catch (Exception _ex) { ex = _ex; }
            this.Invoke(new dFinishUpdate(this.finishUpdate), error, ex);
        }
        private void finishUpdate(uint error, Exception ex)
        {
            if (ex != null)
                UI.ShowError(UI.GetMessage(Msg.ThereWasAnUncaughtExcpetionWhileUpdatingTheFiles) + '\n' + ex, UI.GetMessage(Msg.BootUpdateException));
            else if (error != 0)
                UI.ShowError(UI.GetMessage(Msg.ThereWasAProblemUpdatingTheFiles), null, error, UI.GetMessage(Msg.BootUpdateError));
            else
                ShowMessage(UI.GetMessage(Msg.SuccessfullyUpdatedTheBootAnimationAndText), UI.GetMessage(Msg.BootUpdateSuccess), MessageBoxIcon.None);
            this.progress.DoneAndClose();
        }
        #endregion
    }
}
