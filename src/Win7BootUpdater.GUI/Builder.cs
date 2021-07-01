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

namespace Win7BootUpdater.GUI
{
    #region Translatables
    internal interface TranslatableItem { void Translate(); }
    internal class TranslatableMenuItem : TranslatableItem
    {
        public Msg msg;
        public ToolStripMenuItem item;
        public TranslatableMenuItem(Msg msg, ToolStripMenuItem item) { this.msg = msg; this.item = item; }
        public void Translate() { this.item.Text = UI.GetMessage(this.msg); }
    }
    internal class TranslatableControl : TranslatableItem
    {
        public Msg msg;
        public string[] opt;
        public Control item;
        public TranslatableControl(Msg msg, Control item, params string[] opt) { this.msg = msg; this.item = item; this.opt = opt; }
        public void Translate() { this.item.Text = UI.GetMessage(this.msg, this.opt); }
    }
    internal class TranslatableLinkLabel : TranslatableItem
    {
        public Msg msg;
        public LinkLabel item;
        public string desc;
        public TranslatableLinkLabel(Msg msg, string desc, string link, LinkLabel item)
        {
            this.msg = msg;
            this.desc = desc;
            this.item = item;
            this.item.Links.Add(String.IsNullOrEmpty(desc) ? new LinkLabel.Link(0, 1, link) : new LinkLabel.Link(0, desc.Length, link));
        }
        public void Translate()
        {
            string text = UI.GetMessage(this.msg);
            this.item.Text = text;
            if (String.IsNullOrEmpty(this.desc))
                this.item.Links[0].Length = text.Length;
            else
                this.item.Links[0].Start = text.IndexOf(this.desc);
        }
    }
    internal class TranslatableDropDown : TranslatableItem
    {
        public Msg[] msgs;
        public ComboBox item;
        public TranslatableDropDown(Msg[] msgs, ComboBox item)
        {
            this.msgs = msgs;
            this.item = item;
            string[] strs = new string[msgs.Length];
            for (int i = 0; i < this.msgs.Length; ++i)
                strs[i] = string.Empty;
            this.item.Items.AddRange(strs);
        }
        public void Translate()
        {
            for (int i = 0; i < this.msgs.Length; ++i)
                this.item.Items[i] = UI.GetMessage(this.msgs[i]);
        }
    }
    #endregion

    internal static class Builder
    {
        private static List<TranslatableItem> translatableItems = new List<TranslatableItem>();
        public static void TranslateAllItems() {
            Main.GetInstance().SuspendLayout();
            foreach (TranslatableItem i in translatableItems) i.Translate();
            Main.GetInstance().ResumeLayout();
        }
        public static void AddTranslatableItem(TranslatableItem i) { translatableItems.Add(i); i.Translate(); }

        public static Size useAutoSize = new Size(0, 0);
        public static void runFile(string path) {
            try
            {
                System.Diagnostics.Process.Start(path);
            }
            catch (Exception e)
            {
                Main.GetInstance().ShowError(UI.GetMessage(Msg.FailedToOpenLink) + ": " + e.Message, UI.GetMessage(Msg.FailedToOpenLink));
            }
        }
        public static void runFile(object sender, EventArgs e) { runFile((string)((ToolStripMenuItem)sender).Tag); }
        public static void linkClicked(object sender, LinkLabelLinkClickedEventArgs e) { runFile((string)e.Link.LinkData); }
        public static readonly AnchorStyles topLeftAnchor = AnchorStyles.Top | AnchorStyles.Left;
        public static readonly AnchorStyles topRightAnchor = AnchorStyles.Top | AnchorStyles.Right;
        public static readonly AnchorStyles bottomLeftAnchor = AnchorStyles.Bottom | AnchorStyles.Left;
        public static readonly AnchorStyles bottomRightAnchor = AnchorStyles.Bottom | AnchorStyles.Right;
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

        public static Label createLabel(Point loc, Control parent) { return (Label)setBasic(new Label(), loc, useAutoSize, -1, topLeftAnchor, parent); }
        public static Label createLabel(Msg text, Point loc, Control parent) { return (Label)setBasic(new Label(), text, loc, useAutoSize, -1, topLeftAnchor, parent); }
        public static Label createLabel(Msg text, string opt, Point loc, Control parent) { return (Label)setBasic(new Label(), text, opt, loc, useAutoSize, -1, topLeftAnchor, parent); }
        public static Label createBoldLabel(Msg text, Point loc, Control parent)
        {
            Label l = createLabel(text, loc, parent);
            l.Font = new Font(l.Font, FontStyle.Bold);
            return l;
        }
        public static Label createRightLabel(Msg text, Point trLoc, Size sz, Control parent)
        {
            Label l = (Label)setBasic(new Label(), text, new Point(trLoc.X-sz.Width, trLoc.Y), sz, -1, topRightAnchor, parent);
            l.TextAlign = ContentAlignment.TopRight;
            return l;
        }
        public static Label createRightLabel(Msg text, string opt, Point trLoc, Size sz, Control parent)
        {
            Label l = (Label)setBasic(new Label(), text, opt, new Point(trLoc.X - sz.Width, trLoc.Y), sz, -1, topRightAnchor, parent);
            l.TextAlign = ContentAlignment.TopRight;
            return l;
        }
        public static LinkLabel createLinkLabel(Msg text, Point loc, int tab, string targetText, string targetLink, Control parent)
        {
            TranslatableLinkLabel t = new TranslatableLinkLabel(text, targetText, targetLink, new LinkLabel());
            LinkLabel l = (LinkLabel)setBasic(t.item, t, loc, useAutoSize, tab, topLeftAnchor, parent);
            l.UseCompatibleTextRendering = true;
            l.LinkBehavior = LinkBehavior.HoverUnderline;
            l.LinkClicked += new LinkLabelLinkClickedEventHandler(Builder.linkClicked);
            return l;
        }
        public static PictureBox createPicture(Point loc, Control parent) { return createPicture(loc, new Size(15, 15), parent); }
        public static PictureBox createPicture(Point loc, Size sz, Control parent) { return (PictureBox)setBasic(new PictureBox(), loc, sz, -1, topLeftAnchor, parent); }
        public static GroupBox createGroupBox(Msg text, Point loc, Size sz, int tab, AnchorStyles a, Control parent)
        {
            GroupBox g = (GroupBox)setBasic(new GroupBox(), text, loc, sz, tab, a, parent);
            g.SuspendLayout();
            //g.TabStop = false;
            return g;
        }
        public class TextPanel : Panel, TranslatableItem
        {
            private Msg txt;
            private SolidBrush b;
            private PointF p = new PointF();
            private bool needToRecalc = true;
            public TextPanel(Msg txt) : base() { this.txt = txt; b = new SolidBrush(this.ForeColor); this.DoubleBuffered = true; }
            public void Translate() { this.Text = UI.GetMessage(this.txt); }
            protected override void OnTextChanged(EventArgs e) { base.OnTextChanged(e); needToRecalc = true; base.Refresh(); }
            protected override void OnFontChanged(EventArgs e) { base.OnFontChanged(e); needToRecalc = true; base.Refresh(); }
            protected override void OnForeColorChanged(EventArgs e) { base.OnForeColorChanged(e); b.Color = this.ForeColor; base.Refresh(); }
            protected override void OnPaint(PaintEventArgs e)
            {
                base.OnPaint(e);
                if (!string.IsNullOrEmpty(this.Text))
                {
                    Graphics g = e.Graphics;
                    if (needToRecalc)
                    {
                        SizeF c = g.VisibleClipBounds.Size;
                        SizeF s = g.MeasureString(this.Text, this.Font, c);
                        p.X = (c.Width - s.Width) / 2;
                        p.Y = (c.Height - s.Height) / 2;
                        needToRecalc = false;
                    }
                    g.DrawString(this.Text, this.Font, b, p);
                }
            }
        }
        public static TextPanel createTextPanel(Msg m, Point loc, Size sz, Color c, Control parent)
        {
            TextPanel p = (TextPanel)setBasic(new TextPanel(m), loc, sz, -1, Builder.topRightAnchor, parent);
            Builder.AddTranslatableItem(p);
            p.BackColor = c;
            p.BringToFront();
            return p;
        }
        public static TextBox createTextBox(Point loc, int width, int tab, AnchorStyles a, Control parent)
        {
            return (TextBox)setBasic(new TextBox(), loc, new Size(width, 20), tab, a, parent);
        }
        public static TextBox createMultilineTextBox(Point loc, Size sz, int tab, AnchorStyles a, Control parent)
        {
            TextBox t = (TextBox)setBasic(new TextBox(), loc, sz, tab, a, parent);
            t.Multiline = true;
            t.ScrollBars = ScrollBars.Vertical;
            return t;
        }
        public static TabControl createTabControl(Point loc, Size sz, int tab, AnchorStyles a, Control parent)
        {
            TabControl t = new TabControl();
            t.SuspendLayout();
            t.SelectedIndex = 0;
            setBasic(t, loc, sz, tab, a, parent);
            return t;
        }
        public static TabPage createTab(Msg text, TabControl parent)
        {
            TabPage t = new TabPage();
            t.SuspendLayout();
            setBasic(t, text, new Point(4, 22), useAutoSize, parent.Controls.Count, t.Anchor, parent);
            t.Padding = new Padding(3);
            t.UseVisualStyleBackColor = true;
            return t;
        }
        public static TabPage createTab(Msg text, string opt, TabControl parent)
        {
            TabPage t = new TabPage();
            t.SuspendLayout();
            setBasic(t, text, opt, new Point(4, 22), useAutoSize, parent.Controls.Count, t.Anchor, parent);
            t.Padding = new Padding(3);
            t.UseVisualStyleBackColor = true;
            return t;
        }
        public static TextBoxBase createFullTextTab(Msg text, bool formatted, TabControl parent)
        {
            TabPage t = createTab(text, parent);
            t.Padding = new Padding(0);
            t.Margin = t.Padding;
            t.Location = new Point(1, 19);
            TextBoxBase c;
            if (formatted)
                ((FormattedTextBox)(c = new FormattedTextBox())).ScrollBars = RichTextBoxScrollBars.Vertical;
            else
                ((TextBox)(c = new TextBox())).ScrollBars = ScrollBars.Vertical;
            Builder.setBasic(c, new Point(-3, -3), new Size(parent.Width - 2, t.Height + 6), 0, Builder.topBottomLeftRightAnchor, t);
            c.BackColor = SystemColors.Window;
            c.Multiline = true;
            c.ReadOnly = true;
            t.ResumeLayout();
            //return t;
            return c;
        }
        public static RadioButton createRadio(Msg text, Point loc, int tab, EventHandler click, Control parent)
        {
            RadioButton r = (RadioButton)setBasic(new RadioButton(), text, loc, useAutoSize, tab, topLeftAnchor, parent);
            //r.TabStop = true;
            r.UseVisualStyleBackColor = true;
            r.Click += click;
            return r;
        }
        public static ComboBox createDropDown(Msg[] list, Point loc, int width, int tab, AnchorStyles a, EventHandler change, Control parent)
        {
            TranslatableDropDown t = new TranslatableDropDown(list, new ComboBox());
            ComboBox c = (ComboBox)setBasic(t.item, t, loc, new Size(width, 21), tab, a, parent);
            c.DropDownStyle = ComboBoxStyle.DropDownList;
            //c.FormattingEnabled = true;
            c.SelectedIndexChanged += change;
            return c;
        }
        public static Button createButton(Point loc, int width, int tab, AnchorStyles a, EventHandler click, Control parent)
        {
            Button b = (Button)setBasic(new Button(), loc, new Size(width, 23), tab, a, parent);
            b.UseVisualStyleBackColor = true;
            b.Click += click;
            return b;
        }
        public static Button createButton(Msg text, Point loc, int width, int tab, AnchorStyles a, EventHandler click, Control parent)
        {
            Button b = (Button)setBasic(new Button(), text, loc, new Size(width, 23), tab, a, parent);
            b.UseVisualStyleBackColor = true;
            b.Click += click;
            return b;
        }
        public static NumericUpDown createNumField(int x, int min, int max, Point loc, int width, int tab, AnchorStyles a, EventHandler change, Control parent)
        {
            NumericUpDown n = (NumericUpDown)setBasic(new NumericUpDown(), loc, new Size(width, 20), tab, a, parent);
            n.Value = x;
            n.Minimum = min;
            n.Maximum = max;
            n.ValueChanged += change;
            return n;
        }

        public static ToolStripMenuItem createMenuItem(params ToolStripItem[] items)
        {
            ToolStripMenuItem m = new ToolStripMenuItem();
            m.DropDownItems.AddRange(items);
            return m;
        }
        public static ToolStripMenuItem createMenuItem(Msg text, params ToolStripItem[] items)
        {
            ToolStripMenuItem m = createMenuItem(items);
            AddTranslatableItem(new TranslatableMenuItem(text, m));
            return m;
        }
        public static ToolStripMenuItem createMenuItem(EventHandler click)
        {
            ToolStripMenuItem m = new ToolStripMenuItem();
            m.Click += click;
            return m;
        }
        public static ToolStripMenuItem createMenuItem(string text, EventHandler click)
        {
            ToolStripMenuItem m = createMenuItem(click);
            m.Text = text;
            return m;
        }
        public static ToolStripMenuItem createMenuItem(Msg text, EventHandler click)
        {
            ToolStripMenuItem m = createMenuItem(click);
            AddTranslatableItem(new TranslatableMenuItem(text, m));
            return m;
        }
        public static ToolStripMenuItem createMenuItem(Msg text, string path)
        {
            ToolStripMenuItem m = createMenuItem(text, new System.EventHandler(Builder.runFile));
            m.Tag = path;
            return m;
        }
    }
}