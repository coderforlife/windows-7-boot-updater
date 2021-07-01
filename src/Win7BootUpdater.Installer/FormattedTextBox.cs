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
using System.Text;
using System.Windows.Forms;

namespace Win7BootUpdater.Installer
{
    class FormattedTextBox : RichTextBox
    {
        private class LinkInfo
        {
            public int start, end;
            public string link;
            public string text;
            public bool hover = false;
            public void Go()
            {
                //System.Diagnostics.Process.Start(this.link);
                Builder.runFile(this.link);
            }
        }

        private Dictionary<int, LinkInfo> links = new Dictionary<int, LinkInfo>();

        [System.Runtime.InteropServices.DllImport("user32.dll")]
        private static extern bool HideCaret(IntPtr hWnd);

        private static FontStyle GetFontStyle(char c)
        {
            switch (Char.ToUpper(c))
            {
                case 'B': return FontStyle.Bold;
                case 'U': return FontStyle.Underline;
                case 'I': return FontStyle.Italic;
                case 'S': return FontStyle.Strikeout;
            }
            return FontStyle.Regular;
        }

        public FormattedTextBox() : base()
        {
            this.CreateHandle();
            this.DetectUrls = false;
            this.Cursor = Cursors.Default;
        }

        public override string Text
        {
            set
            {
                this.Select(0, 0); // makes sure selecting things later will work
                // This is required since Select can cause the handle to be (re)created
                // That causes the Text to be set again and an error with concurrent modification/enumeration of links

                string txt = value.Replace("\r\n", "\n").Trim(); // fix line endings
                links.Clear();

                FontStyle current_fs = this.Font.Style;
                List<KeyValuePair<int, FontStyle>> fss = new List<KeyValuePair<int, FontStyle>>();

                int bracket = txt.IndexOf('[');
                while (bracket >= 0)
                {
                    int bracket1 = bracket + 1;
                    if (txt[bracket1] == '[') // text is [ when using [[
                    {
                        txt = txt.Remove(bracket, 1);
                        bracket = txt.IndexOf('[', bracket1);
                        continue;
                    }
                    if (txt[bracket1] == ']') // text is [] when using []
                    {
                        bracket = txt.IndexOf('[', bracket1);
                        continue;
                    }

                    int space = txt.IndexOf(' ', bracket1);
                    int close = txt.IndexOf(']', bracket1);
                    if (close < 0) break;
                    int len = close - bracket1;
                    if (len == 1 || (len == 2 && txt[bracket1] == '/'))
                    {
                        FontStyle fs = GetFontStyle(txt[close - 1]);
                        if (fs != FontStyle.Regular)
                        {
                            bool add = len == 1;
                            if (add) current_fs |= fs;
                            else current_fs &= ~fs;
                            txt = txt.Remove(bracket, add ? 3 : 4);
                            fss.Add(new KeyValuePair<int, FontStyle>(bracket, current_fs));
                            bracket = txt.IndexOf('[', bracket);
                            continue;
                        }
                    }

                    LinkInfo li = new LinkInfo();
                    li.start = bracket;
                    if (space < 0 || space > close)
                    {
                        li.link = li.text = txt.Substring(bracket1, close - bracket1);
                    }
                    else
                    {
                        li.link = txt.Substring(bracket1, space - bracket1);
                        li.text = txt.Substring(space + 1, close - space - 1);
                    }
                    if (li.link.IndexOf('@') > 0)
                        li.link = "mailto:" + li.link;
                    li.end = li.start + li.text.Length;
                    links.Add(li.start, li);
                    txt = txt.Substring(0, bracket) + li.text + txt.Substring(close + 1);
                    bracket = txt.IndexOf('[', li.end);
                }

                base.Text = txt;

                for (int i = 0; i < fss.Count; ++i)
                {
                    this.Select(fss[i].Key, (i == fss.Count - 1) ? txt.Length : fss[i + 1].Key);
                    this.SelectionFont = new Font(this.SelectionFont, fss[i].Value);
                }

                foreach (KeyValuePair<int, LinkInfo> p in links)
                {
                    this.Select(p.Key, p.Value.text.Length);
                    this.SelectionColor = Color.Blue;
                }

                this.DeselectAll();
                HideCaret(this.Handle);
            }
        }

        private void ProcessMouse(MouseEventArgs e)
        {
            this.SuspendLayout();
            bool hover = false;
            int idx = (e == null) ? -1 : this.GetCharIndexFromPosition(e.Location);
            foreach (KeyValuePair<int, LinkInfo> p in links)
            {
                if (idx >= p.Key && idx < p.Value.end)
                {
                    if (!p.Value.hover)
                    {
                        this.Select(p.Key, p.Value.text.Length);
                        this.SelectionFont = new Font(this.SelectionFont, this.SelectionFont.Style | FontStyle.Underline);
                    }
                    hover = p.Value.hover = true;
                }
                else if (p.Value.hover)
                {
                    this.Select(p.Key, p.Value.text.Length);
                    this.SelectionFont = new Font(this.SelectionFont, this.SelectionFont.Style & ~FontStyle.Underline);
                    p.Value.hover = false;
                }
            }
            Cursor c = hover ? Cursors.Hand : Cursors.Default;
            if (c != this.Cursor)
                this.Cursor = c;
            this.DeselectAll();
            this.ResumeLayout();
            HideCaret(this.Handle);
        }

        protected override void OnMouseLeave(EventArgs e) { base.OnMouseLeave(e); ProcessMouse(null); }
        protected override void OnMouseMove(MouseEventArgs e) { base.OnMouseMove(e); ProcessMouse(e); }
        protected override void OnMouseWheel(MouseEventArgs e) { base.OnMouseEnter(e); ProcessMouse(e); }
        protected override void OnMouseClick(MouseEventArgs e)
        {
            base.OnMouseClick(e);
            int idx = this.GetCharIndexFromPosition(e.Location);
            foreach (KeyValuePair<int, LinkInfo> p in links)
                if (idx >= p.Key && idx < p.Value.end) { p.Value.Go(); return; }
            HideCaret(this.Handle);
        }
        /*protected override void OnClick(EventArgs e)
        {
            base.OnClick(e);
            foreach (KeyValuePair<int, LinkInfo> p in links)
                if (p.Value.hover) { p.Value.Go(); return; }
        }*/

        protected override void OnEnter(EventArgs e) { base.OnEnter(e); HideCaret(this.Handle); }
        protected override void OnKeyUp(KeyEventArgs e) { base.OnKeyUp(e); HideCaret(this.Handle); }
        protected override void OnMouseDown(MouseEventArgs e) { base.OnMouseDown(e); HideCaret(this.Handle); }
        protected override void OnSelectionChanged(EventArgs e) { base.OnSelectionChanged(e); HideCaret(this.Handle); }
        protected override void OnParentVisibleChanged(EventArgs e) { base.OnParentVisibleChanged(e); HideCaret(this.Handle); }
    }
}
