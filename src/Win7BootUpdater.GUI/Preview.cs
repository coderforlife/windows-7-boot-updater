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

//#define _SHOW_1024_1024_

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

using Win7BootUpdater;

namespace Win7BootUpdater.GUI
{
    internal class Preview : UserControl
    {
        private class FullscreenControl : Form {
            private UserControl c;
            private Control origParent;
            private AnchorStyles origAnchor;
            private Point origLocation;
            private Size origSize;
            private BorderStyle origBorder;
            public FullscreenControl(UserControl c) { this.c = c; }
            protected override void OnLoad(EventArgs e)
            {
                base.OnLoad(e);
                this.Icon = Res.GetAppIcon();
                this.FormBorderStyle = FormBorderStyle.None;
#if _SHOW_1024_1024_
                this.Bounds = new Rectangle(200, 200, 1024, 768);
#else
                this.WindowState = FormWindowState.Maximized;
                this.Bounds = Screen.PrimaryScreen.Bounds;
#endif
                this.KeyPreview = true;
                this.TopMost = true;
            }
            // NOTE: For some reason this doesn't work the second time
            // Workaround: make a new FullscreenControl each time
            protected override void OnVisibleChanged(EventArgs e)
            {
                if (this.Visible)
                {
                    // Get the current setup
                    origParent = c.Parent;
                    origAnchor = c.Anchor;
                    origLocation = c.Location;
                    origSize = c.Size;
                    origBorder = c.BorderStyle;

                    // Make it fullscreen
                    this.Controls.Add(c); // NOTE: This for some reason causes Click events on the radio buttons in the main form (the old parent)
                    c.Anchor = AnchorStyles.Left | AnchorStyles.Top | AnchorStyles.Right | AnchorStyles.Bottom;
                    c.Location = new Point(0, 0);
                    c.Size = this.Size;
                    c.BorderStyle = BorderStyle.None;
                    c.Focus();
                }
                else
                {
                    // Restore the original setup
                    origParent.Controls.Add(c);
                    c.Anchor = origAnchor;
                    c.Location = origLocation;
                    c.Size = origSize;
                    c.BorderStyle = origBorder;
                    this.DialogResult = DialogResult.OK;
                }
                base.OnVisibleChanged(e);
            }
            protected override void OnPreviewKeyDown(PreviewKeyDownEventArgs e)
            {
                base.OnPreviewKeyDown(e);
                if (e.KeyCode == Keys.Escape) { e.IsInputKey = true; }
            }
            protected override void OnKeyUp(KeyEventArgs e)
            {
                if (e.KeyCode == Keys.Escape) {
                    Close();
                    e.Handled = true;
                }
                base.OnKeyUp(e);
            }
        }

        protected static Image defaultActivity = null;
        protected static Font instructionFont = null;

        protected static Msg[] instructionKeyMsgs  = new Msg[] { Msg.ESC, Msg.Space, Msg.LeftRight, Msg.HomeEnd, Msg.Frame };
        protected static Msg[] instructionDescMsgs = new Msg[] { Msg.ExitFullscreen, Msg.PlayPause, Msg.BackForward1Frame, Msg.FirstLastFrame, Msg.NO_MESSAGE };
        protected static string[] instructionKeys = new string[5], instructionDesc = new string[5];

        public static Image Invalid = null;

        public static void UpdateInstructionLanguages()
        {
            for (int i = 0; i < instructionKeyMsgs.Length; ++i)
            {
                instructionKeys[i] = UI.GetMessage(instructionKeyMsgs[i]);
                instructionDesc[i] = UI.GetMessage(instructionDescMsgs[i]);
            }
        }

        public BootSkin bs;

        public Preview()
        {
            if (Invalid == null || defaultActivity == null)
            {
                Invalid = Res.GetImage("invalid");
                // If embedded:
                //defaultActivity = Res.GetImage("activity");
                defaultActivity = Bootres.GetAnimation(Bootres.def);
                if (defaultActivity == null)
                    defaultActivity = Animation.CreateFromSingle((Bitmap)Invalid);
            }
            if (instructionFont == null)
            {
                instructionFont = new Font(FontFamily.GenericSansSerif, 6, FontStyle.Regular, GraphicsUnit.Point);
            }

            this.bs = new BootSkin();

            this.animRect = new Rectangle(Animation.X, Animation.Y, Animation.Width, Animation.Height);

            this.isFullscreen = false;
            this.instructionBrush = new SolidBrush(Color.White);

            this.DoubleBuffered = true;
            this.SuspendLayout();
            this.BorderStyle = BorderStyle.Fixed3D;
            this.Size = new Size(Animation.ScreenWidth / 2, Animation.ScreenHeight / 2);
            this.player = new Timer();
            this.player.Interval = 65;
            this.player.Tick += new EventHandler(this.tick);
            this.ResumeLayout();
            this.Reset();
        }
        public void Reset()
        {
            this.SuspendLayout();
            this.active = 0u;
            this.bs.Reset();
            this.animInvalidationRect = scaleRect(animRect);
            this.frameInvalidationRect = scaleRect(frameRect);
            Frame = 1;
            this.ResumeLayout();
        }

        public static void SetDefaultAnimSource(string path)
        {
            Image i = Bootres.GetAnimation(path);
            if (i != null)
            {
                defaultActivity.Dispose();
                defaultActivity = i;
            }
        }

        //public void UseDefaultAnim();
        //public bool IsDefaultAnim();

        #region Active WinXXX Control
        private uint active = 0;
        public bool WinResumeActive
        {
            get { return active == 1u; }
            set { active = value ? 1u : 0u; }
        }
        public uint WinXXXActive
        {
            get { return active; }
            set { active = Clamp(value, 0, 1); }
        }
        #endregion

        #region Playback Control

        private uint frame;
        private bool playForward;
        private int firstFrameDelay;
        private Timer player;
        private void tick(object sender, EventArgs e)
        {
            if (firstFrameDelay > 0)
                firstFrameDelay -= 1;
            else
                Frame = playForward ? (frame >= Animation.Frames ? ((uint)Animation.LoopFrame) : (frame + 1)) : (frame - 1);
        }

        public delegate void FrameChangedHandler(uint frame);
        public delegate void PlayPauseHandler(bool playing);

        public event FrameChangedHandler FrameChanged;
        public event PlayPauseHandler PlayPauseChanged;

        private static uint Clamp(uint x, uint min, uint max) { return (x > max) ? max : ((x < min) ? min : x); }
        public uint Frame
        {
            get { return frame;  }
            set
            {
                uint orig_frame = frame;
                frame = Clamp(value, 1, (uint)Animation.Frames);
                if (frame != orig_frame)
                {
                    this.InvalidateAnimation();
                    if (FrameChanged != null)
                        FrameChanged(frame);
                }
            }
        }
        public void RestartAnimation()
        {
            if (this.Frame != 1)
                this.Frame = 1;
            else
                this.InvalidateAnimation();
        }
        public bool Playing { get { return player.Enabled; } }
        public void Play() { if (!player.Enabled) PlayPause(); }
        public void Pause() { if (player.Enabled) PlayPause(); }
        public void PlayPause()
        {
	        playForward = true;
	        firstFrameDelay = 0;
	        player.Enabled = !player.Enabled;
            if (PlayPauseChanged != null)
                PlayPauseChanged(player.Enabled);
        }
        #endregion

        #region Fullscreen Control
        private bool isFullscreen;
        public bool IsFullscreen() { return isFullscreen; }
        public void Fullscreen()
        {
            if (isFullscreen) { return; }
            isFullscreen = true;
            this.Invalidate();
            new FullscreenControl(this).ShowDialog();
            isFullscreen = false;
            this.Invalidate();
        }
        #endregion

        #region Rectangles and Invalidation
        private Rectangle animInvalidationRect, frameInvalidationRect;
        private RectangleF animRect, frameRect;
        private void InvalidateAnimation()
        {
            this.Invalidate(animInvalidationRect);
            if (isFullscreen)
                this.Invalidate(frameInvalidationRect);
        }
        private Rectangle scaleRect(RectangleF r)
        {
            return new Rectangle(
                (int)(r.X * this.Width / Animation.ScreenWidth), (int)(r.Y * this.Height / Animation.ScreenHeight),
                (int)(r.Width * this.Width / Animation.ScreenWidth), (int)(r.Height * this.Height / Animation.ScreenHeight));
        }
        #endregion

        #region Painting
        //private static float fmod(float a, float b) { return a - (float)((int)(a/b)) * b; }
        //private static float fabs(float x) { return x < 0 ? -x : x; }
        //private static int round(float x) { return (int)(x+0.5); }

        private static int sq(int x) { return x*x; }
        private static double Distance(Color c1, Color c2) {
	        double r = (c1.R+c2.R) / 2.0;
	        return Math.Sqrt((2+r/256)*sq((int)c1.R-c2.R) + 4*sq((int)c1.G-c2.G) + (2+(255-r)/256)*sq((int)c1.B-c2.B));
        }
        private static Color GetHighContrastColor(Color c) {
	        return Distance(c, Color.White) > Distance(c, Color.Black) ? Color.White : Color.Black;
        }

        private SolidBrush instructionBrush;
        private static Image GetAnim(BootSkinFile bsf)
        {
            return bsf.IsDefaultAnim() ? defaultActivity : bsf.Anim;
        }
        private void DoPaint(Graphics g, BootSkinFile bsf, uint frame, bool animOnly, bool frameOnly, bool fullscreen)
        {
            // Fill the entire region with the background color
            g.Clear(bsf.BackColor);

            // Update the area outside the animation (if not just updating the animation)
            if (!animOnly)
            {
                if (bsf.UsesBackgroundImage())
                {
                    // Draw the background image
                    try
                    {
                        g.DrawImage(bsf.Background, new Point(0, 0));
                    }
                    catch (Exception) { g.DrawImage(Invalid, new Point(0, 0)); }
                }
                else
                {
                    // Draw the messages
                    for (uint i = 0; i < bsf.MessageCount; ++i)
                    {
                        string msg = bsf.get_Message(i); // silly that indexers with managed classes don't work
                        Font f = bsf.get_Font(i);
                        Brush b = bsf.get_TextBrush(i);

                        SizeF size = g.MeasureString(msg, f);
                        float calc_height = size.Height;
                        size.Height = f.GetHeight(g);
                        float y_shift = calc_height - size.Height;
                        if (f.Size > 0 && size.Width <= Animation.ScreenWidth && size.Height + bsf.get_Position(i) - y_shift <= Animation.ScreenHeight)
                        {
                            g.FillRectangle(bsf.MessageBackBrush, 0.0f, (float)bsf.get_Position(i), (float)Animation.ScreenWidth, size.Height);
                            g.DrawString(msg, f, b, (Animation.ScreenWidth - size.Width) / 2, bsf.get_Position(i) - y_shift);
                        }
                        else { break; }
                    }
                }
            }

            // Draw the fullscreen information
            if (fullscreen)
            {
                instructionBrush.Color = GetHighContrastColor(bsf.BackColor);
                SizeF size = g.MeasureString(instructionKeys[0], instructionFont);
                float height = size.Height, width = size.Width;
                for (int i = 1; i < instructionKeys.Length; ++i)
                {
                    size = g.MeasureString(instructionKeys[i], instructionFont);
                    if (size.Width > width)
                        width = size.Width;
                }
                width += 4;
                for (int i = 0; i < instructionKeys.Length; ++i)
                {
                    g.DrawString(instructionKeys[i], instructionFont, instructionBrush, 0, height * i);
                    if (i == instructionKeys.Length - 1)
                    {
                        g.DrawString(frame.ToString(), instructionFont, instructionBrush, width, height * i);
                        size = g.MeasureString(Animation.Frames.ToString(), instructionFont);
                        frameRect = new RectangleF(width, height * i, size.Width, size.Height);
                        frameInvalidationRect = scaleRect(frameRect);
                    }
                    else
                    {
                        g.DrawString(instructionDesc[i], instructionFont, instructionBrush, width, height * i);
                    }
                }
            }

            // Draw the animation
            if (!frameOnly)
            {
                // Make sure the background of the animation is complete (has no text) to support transparency in the animation
                g.FillRectangle(bsf.BackBrush, animRect);
                if (bsf.UsesBackgroundImage())
                    try
                    {
                        g.DrawImage(bsf.Background, animRect, animRect, GraphicsUnit.Pixel);
                    }
                    catch (Exception) { }

                Image anim = bsf.IsWinloadAnim() ? GetAnim(bs.Winload) : GetAnim(bsf);
                if (anim == null)
                {
                    g.DrawImage(Invalid, animRect);
                }
                else
                {
                    try
                    {
                        g.DrawImage(anim, animRect, new Rectangle(0, (int)(frame - 1) * Animation.Height, Animation.Width, Animation.Height), GraphicsUnit.Pixel);
                    }
                    catch (Exception) { g.DrawImage(Invalid, animRect); }
                }
            }
        }
        public Image GenerateInstallerImage()
        {
            Bitmap b = new Bitmap(420, 315);
            Graphics g = Graphics.FromImage(b);

            // Scales it so we can pretend we are drawing on a full 1024x768 screen
            g.ScaleTransform(420 / (float)Animation.ScreenWidth, 315 / (float)Animation.ScreenHeight);

            this.DoPaint(g, bs.Winload, Animation.LoopFrame, false, false, false);

            return b;
        }
        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            Graphics g = e.Graphics;

            // Scales it so we can pretend we are drawing on a full 1024x768 screen
            g.ScaleTransform(this.Width / (float)Animation.ScreenWidth, this.Height / (float)Animation.ScreenHeight);

            this.DoPaint(g, bs.get_WinXXX(active), this.frame, g.ClipBounds == animRect, g.ClipBounds == frameRect, this.isFullscreen);
        }

        #endregion

        #region Key Events
        protected override void OnPreviewKeyDown(PreviewKeyDownEventArgs e)
        {
            base.OnPreviewKeyDown(e);
            if (e.KeyCode == Keys.Left || e.KeyCode == Keys.Down || e.KeyCode == Keys.Right || e.KeyCode == Keys.Up)
            {
                e.IsInputKey = true;
            }
        }
        protected override void OnKeyDown(KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.Left:
                case Keys.Down:
                    if (!player.Enabled)
                    {
                        playForward = false;
                        firstFrameDelay = 4;
                        player.Enabled = true;
                    }
                    break;
                case Keys.Right:
                case Keys.Up:
                    if (!player.Enabled)
                    {
                        playForward = true;
                        firstFrameDelay = 4;
                        player.Enabled = true;
                    }
                    break;
            }
            base.OnKeyDown(e);
        }
        protected override void OnKeyUp(KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.Left:
                case Keys.Down:
                    if (firstFrameDelay > 0) Frame = frame - 1;
                    player.Enabled = false;
                    e.Handled = true;
                    break;
                case Keys.Right:
                case Keys.Up:
                    if (firstFrameDelay > 0) Frame = frame + 1;
                    player.Enabled = false;
                    e.Handled = true;
                    break;
                case Keys.Home:
                case Keys.PageUp:
                    Frame = 0;
                    e.Handled = true;
                    break;
                case Keys.End:
                case Keys.PageDown:
                    Frame = (uint)Animation.Frames;
                    e.Handled = true;
                    break;
            }
            base.OnKeyUp(e);
        }
        protected override void OnKeyPress(KeyPressEventArgs e)
        {
            if (e.KeyChar == ' ')
            {
                PlayPause();
                e.Handled = true;
            }
            base.OnKeyPress(e);
        }
        protected override void OnMouseWheel(MouseEventArgs e)
        {
            base.OnMouseWheel(e);
            int count = e.Delta / 120;
            if (count != 0) Frame = (uint)(Frame + count);
        }
        #endregion

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);
            this.animInvalidationRect = scaleRect(animRect);
            this.frameInvalidationRect = scaleRect(frameRect);
        }
    }
}
