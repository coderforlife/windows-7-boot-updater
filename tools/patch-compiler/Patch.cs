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

using Ionic.Zlib;
using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Text;
using System.Xml;

namespace Win7BootUpdater.Patches
{
    public enum Platforms : ushort { I386 = 0x014C, AMD64 = 0x8664 };
    public enum Compressions : ushort { None = 0, GZip = 1, Deflate = 2 };
    static class Sections
    {
        public static readonly sbyte[] Blank = { 0, 0, 0, 0, 0, 0, 0, 0 };
        public static readonly sbyte[] text = { (sbyte)'.', (sbyte)'t', (sbyte)'e', (sbyte)'x', (sbyte)'t', 0, 0, 0 };
        public static readonly sbyte[] rdata = { (sbyte)'.', (sbyte)'r', (sbyte)'d', (sbyte)'a', (sbyte)'t', (sbyte)'a', 0, 0 };
        public static readonly sbyte[][] All = { Blank, text, rdata };
    }

    public abstract class Patch
    {
        private static bool Equal<T>(T[] a, T[] b)
        {
            if (a.Length != b.Length) return false;
            EqualityComparer<T> q = EqualityComparer<T>.Default;
            for (int i = 0; i < a.Length; i++)
                if (!q.Equals(a[i], b[i]))
                    return false;
            return true;
        }

        ///////////////////////////////////////////////////////////////////////////////
        ///// Reading functions (XML)
        ///////////////////////////////////////////////////////////////////////////////
        private static byte HexNibble(char c) { return (byte)(char.IsDigit(c) ? (c - '0') : (c - 'A' + 10)); }
        protected static sbyte[] ReadSection(XmlNode n)
        {
            string sect = n.InnerText;
            sbyte[] s = new sbyte[Sections.Blank.Length];
            int i = 0;
            for (; i < sect.Length; ++i) s[i] = (sbyte)sect[i];
            for (; i < s.Length; ++i) s[i] = 0;
            return s;
        }
        protected static byte[] ReadBytesBasic(string s)
        {
            s = s.ToUpper();
            ushort l = (ushort)(s.Length / 2);
            byte[] x = new byte[l];
            for (int i = 0; i < l; ++i)
                x[i] = (byte)((HexNibble(s[2 * i]) << 4) | HexNibble(s[2 * i + 1]));
            return x;
        }
        protected static byte[] ReadBytesBasic(string s, ref byte wildcard)
        {
            s = s.ToUpper();
            ushort l = (ushort)(s.Length / 2);
            byte[] x = new byte[l];
            List<int> wildcardPos = new List<int>();
            bool[] n = new bool[256];
            for (int i = 0; i < l; ++i)
            {
                if (s[2 * i] == '?')
                { // && s[2*i+1] == '?'
                    wildcardPos.Add(i);
                }
                else {
                    x[i] = (Byte)((HexNibble(s[2 * i]) << 4) | HexNibble(s[2 * i + 1]));
                    n[x[i]] = true;
                }
            }
            if (wildcardPos.Count == 0)
            {
                wildcard = x[0]; // no wildcard if the wildcard equals the first byte
            }
            else
            {
                int i;
                for (i = 0; i < 256; ++i)
                {
                    if (!n[i])
                    {
                        wildcard = (Byte)i;
                        break;
                    }
                }
                if (i == 256) { throw new Exception("No possible wildcard value"); }
                for (i = 0; i < wildcardPos.Count; ++i)
                {
                    x[wildcardPos[i]] = wildcard;
                }
            }
            return x;
        }
        protected static string ReadBytesPositions(XmlNode n, ref ushort[] pos)
        {
            string s = n.InnerText.Replace(" ", "").Replace("]", ""); // we don't care about spaces and the ending bracket
            List<ushort> positions = new List<ushort>();
            int bracket = 0;
            while ((bracket = s.IndexOf('[', bracket)) >= 0)
            {
                positions.Add((ushort)(bracket / 2));
                s = s.Remove(bracket, 1);
            }
            pos = positions.ToArray();
            return s.Replace("[", "");
        }
        protected static string ReadBytesPosition(XmlNode n, ref ushort pos)
        {
            string s = n.InnerText.Replace(" ", "").Replace("]", ""); // we don't care about spaces and the ending bracket
            int bracket = s.IndexOf('[');
            if (bracket < 0) throw new Exception("Need a position");
            pos = (ushort)(bracket / 2);
            return s.Replace("[", "");
        }

        protected static string ReadBytesNoPositions(XmlNode n)
        {
            // we don't care about positions in this version
            return n.InnerText.Replace(" ", "").Replace("[", "").Replace("]", "");
        }

        protected static string ReadBytesPositions(XmlNode n, ref ushort[] pos, ref ushort[] va_pos)
        {
            string s = n.InnerText.Replace(" ", "").Replace("]", "").Replace("}", ""); // we don't care about spaces and the ending bracket
            List<ushort> positions = new List<ushort>();
            List<ushort> vaPositions = new List<ushort>();
            int bracket = 0;
            for (;;)
            {
                int bracket1 = s.IndexOf('[', bracket), bracket2 = s.IndexOf('{', bracket);
                bracket = bracket1 < 0 ? bracket2 : (bracket2 < 0 ? bracket1 : Math.Min(bracket1, bracket2));
                if (bracket < 0)
                    break;
                else if (bracket1 == bracket)
                    positions.Add((ushort)(bracket / 2));
                else
                    vaPositions.Add((ushort)(bracket / 2));
                s = s.Remove(bracket, 1);
            }
            pos = positions.ToArray();
            va_pos = vaPositions.ToArray();
            return s.Replace("[", "").Replace("{", "");
        }

        protected static byte[] ReadBytes(XmlNode n, ref ushort[] pos, ref ushort[] va_pos) { return ReadBytesBasic(ReadBytesPositions(n, ref pos, ref va_pos)); }

        protected static byte[] ReadBytes(XmlNode n, ref ushort[] pos, ref byte wildcard) { return ReadBytesBasic(ReadBytesPositions(n, ref pos), ref wildcard); }
        protected static byte[] ReadBytes(XmlNode n, ref ushort pos, ref byte wildcard) { return ReadBytesBasic(ReadBytesPosition(n, ref pos), ref wildcard); }
        protected static byte[] ReadBytes(XmlNode n, ref byte wildcard) { return ReadBytesBasic(ReadBytesNoPositions(n), ref wildcard); }
        //protected static byte[] ReadBytes(XmlNode n, ref ushort[] pos) { return ReadBytesBasic(ReadBytesPositions(n, ref pos)); }
        protected static byte[] ReadBytes(XmlNode n, ref ushort pos) { return ReadBytesBasic(ReadBytesPosition(n, ref pos)); }
        protected static byte[] ReadBytes(XmlNode n) { return ReadBytesBasic(ReadBytesNoPositions(n)); }

        protected static T[] ReadArray<T>(XmlNodeList n)
        {
            ushort l = (ushort)n.Count;
            T[] a = new T[l];
            for (int i = 0; i < l; ++i)
                a[i] = (T)Convert.ChangeType(n[i].InnerText, typeof(T));
            return a;
        }


        ///////////////////////////////////////////////////////////////////////////////
        ///// Writing functions and macros
        ///////////////////////////////////////////////////////////////////////////////
        protected static void WriteSection(BinaryWriter b, sbyte[] section)
        {
            for (byte i = 0; i < Sections.All.Length; ++i)
                if (Equal(Sections.All[i], section))
                {
                    b.Write((byte)0); b.Write(i); return;
                }
            b.Write((byte[])(Array)section);
        }
        protected static void WriteBytes(BinaryWriter b, byte[] x)
        {
            b.Write((ushort)x.Length);
            b.Write(x);
        }
        protected static void WriteArray(BinaryWriter b, ushort[] a)
        {
            b.Write((ushort)a.Length);
            for (int i = 0; i < a.Length; ++i)
                b.Write(a[i]);
        }
        protected static void WriteString(BinaryWriter b, string s)
        {
            for (int i = 0; i < s.Length; ++i)
                b.Write(s[i]);
            b.Write('\0');
        }



        public ushort Type { get { return (ushort)this.GetType().GetField("Type", BindingFlags.Static | BindingFlags.Public | BindingFlags.DeclaredOnly).GetValue(this); } }
        public abstract void Write(BinaryWriter b);
    }

    namespace Types
    {
        class Direct : Win7BootUpdater.Patches.Patch
        {
            private sbyte[] section;
            private byte wildcard;
            private byte[] target, value;
            public new const ushort Type = 0x0001;
            public Direct(XmlNode n)
            {
                section = ReadSection(n.SelectSingleNode("Section"));
                target = ReadBytes(n.SelectSingleNode("Target"), ref wildcard);
                value = ReadBytes(n.SelectSingleNode("Value"));
                if (target.Length != value.Length) { throw new Exception("Target and Value in PatchDirect must be the same length"); }
            }
            public override void Write(BinaryWriter b)
            {
                WriteSection(b, section);
                b.Write(wildcard);
                WriteBytes(b, target);
                WriteBytes(b, value);
            }
        }

        class Dwords : Win7BootUpdater.Patches.Patch
        {
            private sbyte[] section;
            private ushort[] pos;
            private byte wildcard;
            private byte[] target;
            public new const ushort Type = 0x0002;
            public Dwords(XmlNode n)
            {
                section = ReadSection(n.SelectSingleNode("Section"));
                target = ReadBytes(n.SelectSingleNode("Target"), ref pos, ref wildcard);

                for (int i = 0; i < pos.Length; ++i)
                    if (pos[i] + sizeof(uint) > (uint)target.Length)
                    {
                        throw new Exception("Illegal position in patch description");
                    }
            }
            public ushort Count { get { return (ushort)pos.Length; } }
            public override void Write(BinaryWriter b)
            {
                WriteSection(b, section);
                WriteArray(b, pos);
                b.Write(wildcard);
                WriteBytes(b, target);
            }
        }

        class String : Win7BootUpdater.Patches.Patch
        {
            private sbyte[] section;
            private ushort pos;
            private byte wildcard;
            private byte[] target;
            public new const ushort Type = 0x0003;
            public String(XmlNode n)
            {
                section = ReadSection(n.SelectSingleNode("Section"));
                target = ReadBytes(n.SelectSingleNode("Target"), ref pos, ref wildcard);
            }
            public override void Write(BinaryWriter b)
            {
                WriteSection(b, section);
                b.Write(pos);
                b.Write(wildcard);
                WriteBytes(b, target);
            }
        }

        class AddFunction : Win7BootUpdater.Patches.Patch
        {
            private sbyte[] section;
            private ushort callPos;
            private byte wildcard;
            private byte[] target, call, func;
            private ushort[] patchPos, funcPos;
            private string[] funcNames;
            public new const ushort Type = 0x0004;
            public AddFunction(XmlNode n)
            {
                section = ReadSection(n.SelectSingleNode("Section"));
                target = ReadBytes(n.SelectSingleNode("Target"), ref wildcard);
                call = ReadBytes(n.SelectSingleNode("Call"), ref callPos);
                func = ReadBytes(n.SelectSingleNode("Function"), ref patchPos, ref funcPos);
                XmlNode FuncNames = n.SelectSingleNode("FuncNames");
                if (FuncNames == null) { funcNames = new string[0]; }
                else
                {
                    int len = FuncNames.ChildNodes.Count;
                    funcNames = new string[len];
                    for (int i = 0; i < len; i++) { funcNames[i] = FuncNames.ChildNodes[i].InnerText; }
                }

                for (int i = 0; i < patchPos.Length; ++i)
                    if (patchPos[i] + sizeof(uint) > (uint)func.Length) { throw new Exception("Illegal position in patch description"); }
                int total_with_name = 0;
                for (int i = 0; i < funcPos.Length; ++i) // may also need a 64-bit version
                {
                    if (funcPos[i] + sizeof(uint) > (uint)func.Length) { throw new Exception("Illegal position in patch description"); }
                    else if (func[funcPos[i]] == 0 && func[funcPos[i]+1] == 0 && func[funcPos[i]+2] == 0 && func[funcPos[i]+3] == 0) { total_with_name++; }
                }
                if (funcNames.Length != total_with_name) { throw new Exception("Invalid number of function names"); }
            }
            public override void Write(BinaryWriter b)
            {
                WriteSection(b, section);
                b.Write(wildcard);
                WriteBytes(b, target);
                WriteBytes(b, call);
                b.Write(callPos);
                WriteBytes(b, func);
                WriteArray(b, patchPos);
                WriteArray(b, funcPos);
                for (int i = 0; i < funcNames.Length; i++)
                {
                    WriteBytes(b, ASCIIEncoding.ASCII.GetBytes(funcNames[i]+'\0'));
                }
            }
        }
    }

    class PatchVersion
    {
        public static ulong ReadVersion(string s, Dictionary<string, ulong> names)
        {
            if (s == null || s.Length == 0) return 0;
            if (names != null && names.ContainsKey(s))
                return names[s];
            string[] x = s.Split('.');
            if (x.Length != 4)
                throw new ArgumentException();
            ulong v = 0;
            for (int i = 0; i < 4; ++i)
                v |= ((ulong)ushort.Parse(x[i])) << ((3 - i) * 16);
            return v;
        }

        private ulong min, max; // the format from VS_FIXEDFILEINFO: 64-bits, 8 bits for each of: major, minor, build, revision
                                // max can be all 0s which means no max
        private Patch patch;
        public PatchVersion(XmlElement n, Dictionary<string, ulong> versionNames)
        {
            min = ReadVersion(n.GetAttribute("min"), versionNames);
            max = ReadVersion(n.GetAttribute("max"), versionNames);
            XmlElement p = (XmlElement)n.FirstChild;
            if      (p.Name == "PatchDirect")      patch = new Types.Direct(p);
            else if (p.Name == "PatchDwords")      patch = new Types.Dwords(p);
            else if (p.Name == "PatchString")      patch = new Types.String(p);
            else if (p.Name == "PatchAddFunction") patch = new Types.AddFunction(p);
            else throw new Exception("Loading Patch Failed");
        }
        public ulong Min { get { return min; } }
        public ulong Max { get { return max; } }
        public bool NoMax { get { return max == 0; } }
        public Patch Get() { return patch; }
        public void Write(BinaryWriter b)
        {
            b.Write(min);
            b.Write(max);
            b.Write(patch.Type);
            patch.Write(b);
        }
    }

    class PatchPlatform
    {
        private ushort platform; // IMAGE_FILE_MACHINE_I386 or IMAGE_FILE_MACHINE_AMD64
        private PatchVersion[] versions;
        public PatchPlatform(XmlElement n, Dictionary<string, ulong> versionNames)
        {
            platform = (ushort)Enum.Parse(typeof(Platforms), n.GetAttribute("type"));
            ushort l = (ushort)n.ChildNodes.Count;
            versions = new PatchVersion[l];
            for (int i = 0; i < l; ++i)
                versions[i] = new PatchVersion((XmlElement)n.ChildNodes[i], versionNames);
        }
        public ushort Type { get { return platform; } }
        public PatchVersion[] Get(ulong version)
        {
            List<PatchVersion> results = new List<PatchVersion>();
            foreach (PatchVersion v in versions)
                if (v.Min <= version && (v.NoMax || v.Max >= version))
                    results.Add(v);
            return results.ToArray();
        }
        public Patch[] GetPatches(ulong version)
        {
            List<Patch> results = new List<Patch>();
            foreach (PatchVersion v in versions)
                if (v.Min <= version && (v.NoMax || v.Max >= version))
                    results.Add(v.Get());
            return results.ToArray();
        }
        public void Write(BinaryWriter b)
        {
            b.Write(platform);
            b.Write((ushort)versions.Length);
            foreach (PatchVersion v in versions)
                v.Write(b);
        }
    }

    class PatchEntry
    {
        private ushort id;
        private PatchPlatform[] platforms;
        public PatchEntry(XmlElement n, Dictionary<string, ulong> versionNames)
        {
            id = ushort.Parse(n.GetAttribute("id"));
            ushort l = (ushort)n.ChildNodes.Count;
            platforms = new PatchPlatform[l];
            for (int i = 0; i < l; ++i)
                platforms[i] = new PatchPlatform((XmlElement)n.ChildNodes[i], versionNames);
        }
        public ushort Id { get { return id; } }
        public PatchPlatform Get(ushort platform)
        {
            foreach (PatchPlatform p in platforms)
                if (p.Type == platform)
                    return p;
            return null;
        }
        public void Write(BinaryWriter b)
        {
            b.Write(id);
            b.Write((ushort)platforms.Length);
            foreach (PatchPlatform p in platforms)
                p.Write(b);
        }
    }

    class PatchFile
    {
        public const ushort Magic = 0x7C9A;

        private ushort format_major, format_minor;
        private ushort file_major, file_minor;
        private PatchEntry[] entries;
        public PatchFile(string xmlFile)
        {

            XmlReader reader = null;
            try
            {
                // Load the XML Schema

                string schema = Resources.patch;
                StringReader sr = new StringReader(schema);

                // Prepare the XML Reader
                XmlReaderSettings settings = new XmlReaderSettings();
                settings.IgnoreWhitespace = true;
                settings.IgnoreComments = true;
                settings.ValidationType = ValidationType.Schema;

                // Parse the Schema
                settings.Schemas.Add(null, new XmlTextReader(sr));
                sr.Dispose();
                sr = null;

                // Load the XML file and validate it
                reader = XmlReader.Create(xmlFile, settings);

                // Create an XmlDocument (with DOM interface)
                XmlDocument xml = new XmlDocument();
                xml.PreserveWhitespace = false;
                xml.Load(reader);
                XmlElement root = xml.DocumentElement;

                // Read the document
                string vers = root.GetAttribute("version");
                int dot = vers.IndexOf('.');
                format_major = 0;
                format_minor = 3;
                file_major = ushort.Parse(vers.Substring(0, dot));
                file_minor = ushort.Parse(vers.Substring(dot + 1));

                Dictionary<string, ulong> vns = new Dictionary<string, ulong>();
                foreach (XmlElement e in root.SelectNodes("Version"))
                    vns.Add(e.GetAttribute("name"), PatchVersion.ReadVersion(e.GetAttribute("value"), null));

                XmlNodeList entryNodes = root.SelectNodes("Entry");
                ushort l = (ushort)entryNodes.Count;
                entries = new PatchEntry[l];
                for (int i = 0; i < l; ++i)
                    entries[i] = new PatchEntry((XmlElement)entryNodes[i], vns);
            }
            finally
            {
                if (reader != null) reader.Close();
            }
        }
        public PatchEntry[] Get(ushort id)
        {
            List<PatchEntry> results = new List<PatchEntry>();
            foreach (PatchEntry e in entries)
                if (e.Id == id)
                    results.Add(e);
            return results.ToArray();
        }
        public Patch[] Get(ushort id, ushort platform, ulong version)
        {
            List<Patch> results = new List<Patch>();
            PatchEntry[] es = Get(id);
            foreach (PatchEntry e in es)
            {
                PatchPlatform p = e.Get(platform);
                if (p != null)
                    results.AddRange(p.GetPatches(version));
            }
            return results.ToArray();
        }
        public void Write(Stream s)
        {
            BinaryWriter b = new BinaryWriter(s);
            b.Write((ushort)Magic);
            b.Write((ushort)format_major);
            b.Write((ushort)format_minor);
            b.Write((ushort)file_major);
            b.Write((ushort)file_minor);

            //b.Write((ushort)Compressions.None);

            //b.Write((ushort)Compressions.GZip);
            //b = new BinaryWriter(new GZipStream(s, CompressionMode.Compress));

            b.Write((ushort)Compressions.Deflate);
            b = new BinaryWriter(new DeflateStream(s, CompressionMode.Compress));

            b.Write((ushort)entries.Length);
            foreach (PatchEntry e in entries)
                e.Write(b);
            b.Close();
        }
    }
}
