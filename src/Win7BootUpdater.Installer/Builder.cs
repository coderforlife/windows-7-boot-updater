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
using System.Windows.Forms;

namespace Win7BootUpdater.Installer
{
    #region Translatables
    internal interface TranslatableItem { void Translate(); }
    internal class TranslatableControl : TranslatableItem
    {
        public Msg msg;
        public string[] opt;
        public Control item;
        public TranslatableControl(Msg msg, Control item, params string[] opt) { this.msg = msg; this.item = item; this.opt = opt; }
        public void Translate() { this.item.Text = UI.GetMessage(this.msg, this.opt); }
    }
    #endregion

    internal static class Builder
    {
        private static List<TranslatableItem> translatableItems = new List<TranslatableItem>();
        public static void TranslateAllItems()
        {
            Main.GetInstance().SuspendLayout();
            foreach (TranslatableItem i in translatableItems) i.Translate();
            Main.GetInstance().ResumeLayout();
        }
        public static void AddTranslatableItem(TranslatableItem i) { translatableItems.Add(i); i.Translate(); }

        public static readonly Size useAutoSize = Size.Empty;
        public static void runFile(string path)
        {
            try
            {
                System.Diagnostics.Process.Start(path);
            }
            catch (Exception e)
            {
                Main.GetInstance().ShowError(UI.GetMessage(Msg.FailedToOpenLink) + ": " + e.Message, UI.GetMessage(Msg.FailedToOpenLink));
            }
        }
        public static readonly AnchorStyles topLeftAnchor = AnchorStyles.Top | AnchorStyles.Left;
        public static readonly AnchorStyles bottomRightAnchor = AnchorStyles.Bottom | AnchorStyles.Right;
        public static readonly AnchorStyles topBottomLeft = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left;
        public static readonly AnchorStyles bottomLeftRightAnchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
        public static readonly AnchorStyles topLeftRightAnchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
        public static readonly AnchorStyles topBottomLeftRightAnchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;

        public static Control setBasic(Control c, Point loc, Size sz, int tab, AnchorStyles a, Control parent)
        {
            c.Location = loc;
            if (sz.Equals(useAutoSize)) { c.AutoSize = true; } else { c.Size = sz; }
            c.TabStop = tab >= 0;
            if (c.TabStop) { c.TabIndex = tab; }
            c.Anchor = a;
            parent.Controls.Add(c);
            return c;
        }
        public static Control setBasic(Control c, TranslatableItem t, Point loc, Size sz, int tab, AnchorStyles a, Control parent) { AddTranslatableItem(t); return setBasic(c, loc, sz, tab, a, parent); }
        public static Control setBasic(Control c, Msg text, Point loc, Size sz, int tab, AnchorStyles a, Control parent) { return setBasic(c, new TranslatableControl(text, c), loc, sz, tab, a, parent); }
        public static Control setBasic(Control c, Msg text, string opt, Point loc, Size sz, int tab, AnchorStyles a, Control parent) { return setBasic(c, new TranslatableControl(text, c, opt), loc, sz, tab, a, parent); }
        public static Control setBasic(Control c, Msg text, string[] opt, Point loc, Size sz, int tab, AnchorStyles a, Control parent) { return setBasic(c, new TranslatableControl(text, c, opt), loc, sz, tab, a, parent); }

        public static Label createLabel(Point loc, Control parent) { return (Label)setBasic(new Label(), loc, useAutoSize, -1, topLeftAnchor, parent); }
        public static Label createLabel(Msg text, Point loc, Control parent) { return (Label)setBasic(new Label(), text, loc, useAutoSize, -1, topLeftAnchor, parent); }
        //public static Label createLabel(Msg text, string opt, Point loc, Control parent) { return (Label)setBasic(new Label(), text, opt, loc, useAutoSize, -1, topLeftAnchor, parent); }
        public static Label createLabel(Msg text, string[] opt, Point loc, Control parent) { return (Label)setBasic(new Label(), text, opt, loc, useAutoSize, -1, topLeftAnchor, parent); }
        public static Label createBoldLabel(Msg text, Point loc, Control parent)
        {
            Label l = createLabel(text, loc, parent);
            l.Font = new Font(l.Font, FontStyle.Bold);
            return l;
        }
        private static FormattedTextBox setMultilineLabel(FormattedTextBox t, Control parent)
        {
            t.BackColor = parent.BackColor;
            t.BorderStyle = BorderStyle.None;
            //t.Enabled = false;
            t.ReadOnly = true;
            t.Multiline = true;
            return t;
        }
        public static FormattedTextBox createMultilineLabel(Point loc, Size sz, AnchorStyles a, Control parent) { return setMultilineLabel((FormattedTextBox)setBasic(new FormattedTextBox(), loc, sz, -1, a, parent), parent); }
        public static FormattedTextBox createMultilineLabel(Msg text, Point loc, Size sz, AnchorStyles a, Control parent) { return setMultilineLabel((FormattedTextBox)setBasic(new FormattedTextBox(), text, loc, sz, -1, a, parent), parent); }
        public static FormattedTextBox createMultilineLabel(Msg text, string[] opt, Point loc, Size sz, AnchorStyles a, Control parent) { return setMultilineLabel((FormattedTextBox)setBasic(new FormattedTextBox(), text, opt, loc, sz, -1, a, parent), parent); }
        public static FormattedTextBox createFullPageLabel(Control parent) { return createMultilineLabel(new Point(12, 12), new Size(473, 231), topBottomLeftRightAnchor, parent); }
        public static FormattedTextBox createFullPageLabel(Msg text, Control parent) { return createMultilineLabel(text, new Point(12, 12), new Size(473, 231), topBottomLeftRightAnchor, parent); }
        public static FormattedTextBox createFullPageLabel(Msg text, string[] opt, Control parent) { return createMultilineLabel(text, opt, new Point(12, 12), new Size(473, 231), topBottomLeftRightAnchor, parent); }
        public static FormattedTextBox createCenteredMultilineLabel(Msg text, Point loc, Size sz, Control parent) { return createCenteredMultilineLabel(text, null, loc, sz, parent); }
        public static FormattedTextBox createCenteredMultilineLabel(Msg text, string[] opt, Point loc, Size sz, Control parent)
        {
            FormattedTextBox t = createMultilineLabel(text, opt, loc, sz, topLeftRightAnchor, parent);
            t.SelectAll();
            t.SelectionAlignment = HorizontalAlignment.Center;
            t.DeselectAll();
            return t;
        }
        public static TextBox createStaticTextBox(string text, Point loc, Size sz, int tab, Control parent)
        {
            TextBox t = (TextBox)setBasic(new TextBox(), loc, sz, tab, topLeftRightAnchor, parent);
            t.Text = text;
            t.BackColor = SystemColors.Window;
            t.Multiline = true;
            t.ReadOnly = true;
            t.ScrollBars = ScrollBars.Vertical;
            return t;
        }
        public static Panel createPanel(Point loc, Size sz, int tab, AnchorStyles a,Control parent)
        {
            Panel p = (Panel)Builder.setBasic(new Panel(), loc, sz, 0, a, parent);
            p.SuspendLayout();
            return p;
        }
        public static PictureBox createPicture(Point loc, Size sz, Control parent) { return (PictureBox)setBasic(new PictureBox(), loc, sz, -1, topLeftAnchor, parent); }
        public static Button createButton(Point loc, int width, int tab, AnchorStyles a, EventHandler click, Control parent)
        {
            Button b = (Button)setBasic(new Button(), loc, new Size(width, 23), tab, a, parent);
            b.UseVisualStyleBackColor = true;
            b.Click += click;
            return b;
        }
    }
}
