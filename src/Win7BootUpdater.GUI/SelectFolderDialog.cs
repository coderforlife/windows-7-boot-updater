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
using System.IO;
using System.Runtime.InteropServices;
using System.Windows.Forms;

using Win7BootUpdater.GUI.COM;

namespace Win7BootUpdater.GUI
{
    internal class SelectFolderDialog : CommonDialog
    {
        private class WindowHandleWrapper : IWin32Window
        {
            private IntPtr h;
            public WindowHandleWrapper(IntPtr h) { this.h = h; }
            public virtual IntPtr Handle { get { return this.h; }  }
        }
        private class FolderDialogEvents : FileDialogEventsAdapater
        {
            private string text;
            public FolderDialogEvents(string text) { this.text = text; }
            public override void OnButtonClicked([In, MarshalAs(UnmanagedType.Interface)] IFileDialogCustomize pfdc, [In] uint dwIDCtl) { MessageBox.Show(this.text); }
        }
        
		// Shell Parsing Names
        [DllImport("shell32.dll", CharSet = CharSet.Unicode)]
        private static extern int SHCreateItemFromParsingName([MarshalAs(UnmanagedType.LPWStr)] string pszPath, IntPtr pbc, ref Guid riid, [MarshalAs(UnmanagedType.Interface)] out object ppv);
        private static IShellItem CreateItemFromParsingName(string path)
        {
            Guid guid = new Guid(IIDGuid.IShellItem);
            object item;
            int hr = SHCreateItemFromParsingName(path, IntPtr.Zero, ref guid, out item);
            if (hr != 0) { throw new System.ComponentModel.Win32Exception(hr); }
            return (IShellItem)item;
        }

        private FolderBrowserDialog old_dialog = null;
        private string description = String.Empty, selectedPath = String.Empty;

        public SelectFolderDialog()
        {
            if (!Win7BootUpdater.Updater.IsWindowsVistaOrNewer())
            {
                this.old_dialog = new FolderBrowserDialog();
                this.old_dialog.ShowNewFolderButton = false;
            }
        }

        public override void Reset()
        {
            if (this.old_dialog != null) { this.old_dialog.Reset(); }
            else { this.description = String.Empty; this.selectedPath = String.Empty; }
        }

        #region Properties
        public string Description
        {
            get { return (this.old_dialog != null) ? this.old_dialog.Description : this.description; }
            set { if (this.old_dialog != null) { this.old_dialog.Description = value; } else { this.description = value ?? String.Empty; } }
        }
        public string SelectedPath
        {
            get { return (this.old_dialog != null) ? this.old_dialog.SelectedPath : this.selectedPath; }
            set { if (this.old_dialog != null) { this.old_dialog.SelectedPath = value; } else { this.selectedPath = value ?? String.Empty; } }
        }
        #endregion

        protected override bool RunDialog(IntPtr hwndOwner)
        {
            if (this.old_dialog != null) { return this.old_dialog.ShowDialog(hwndOwner == IntPtr.Zero ? null : new WindowHandleWrapper(hwndOwner)) == DialogResult.OK; }

            IFileOpenDialog dialog = null;
            try {
                dialog = (IFileOpenDialog)new NativeFileOpenDialog();

                // Set Properties
                uint cookie = 0;
                if (this.description.Length != 0)
                {
                    IFileDialogCustomize customize = (IFileDialogCustomize)dialog;
                    customize.AddPushButton(0, UI.GetMessage(Msg.AnimationInformation));
                    dialog.Advise(new FolderDialogEvents(this.description), out cookie);
                }
                dialog.SetOptions(FOS.PICKFOLDERS | FOS.FORCEFILESYSTEM | FOS.FILEMUSTEXIST);
                if (this.selectedPath.Length != 0)
                {
                    string parent = Path.GetDirectoryName(this.selectedPath);
                    if (parent == null || !Directory.Exists(parent))
                    {
                        dialog.SetFileName(this.selectedPath);
                    }
                    else
                    {
                        dialog.SetFolder(CreateItemFromParsingName(parent));
                        dialog.SetFileName(Path.GetFileName(this.selectedPath));
                    }
                }

                // Show dialog
                int result = dialog.Show(hwndOwner);

                // Remove events
                if (this.description.Length != 0) { dialog.Unadvise(cookie); }

                // Handle the error
                if ((uint)result == (uint)HRESULT.ERROR_CANCELLED) { return false; }
                else if (result < 0) { throw Marshal.GetExceptionForHR(result); }
    
                // Get Result
                IShellItem item;
                dialog.GetResult(out item);
                item.GetDisplayName(SIGDN.FILESYSPATH, out selectedPath);
                return true;
            }
            finally
            {
                if (dialog != null) { Marshal.FinalReleaseComObject(dialog); }
            }
        }

        protected override void Dispose(bool disposing)
        {
            try
            {
                if (disposing && this.old_dialog != null) { this.old_dialog.Dispose(); }
            }
            finally { base.Dispose(disposing); }
        }
    }
}
