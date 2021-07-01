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

#include "Resources.h"

#include "WMI.h"
#include "Zip.h"

using namespace Win7BootUpdater;
using namespace Win7BootUpdater::Compression;
using namespace Win7BootUpdater::Patches;

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Globalization;
using namespace System::IO;
using namespace System::IO::Compression;
//using namespace System::Net;
using namespace System::Reflection;
using namespace System::Resources;
//using namespace System::Security::Cryptography;
using namespace System::Text;
using namespace System::Text::RegularExpressions;
//using namespace System::Threading;

/*#if __CLR_VER >= 40000000
#define Yield() Thread::Yield()
#else
#define Yield() Thread::Sleep(1)
#endif*/

/*inline static string GetCpuId()			{ return WMI::GetValue(L"Win32_Processor",		L"ProcessorID");  }
inline static string GetBIOSSerial()	{ return WMI::GetValue(L"Win32_BIOS",			L"SerialNumber"); }
inline static string GetMBSerial()		{ return WMI::GetValue(L"Win32_BaseBoard",		L"SerialNumber"); }
inline static string GetHdSerial()		{ return WMI::GetValue(L"Win32_DiskDrive",		L"SerialNumber"); }
//inline static string GetMACAddress()	{ return WMI::GetValue(L"Win32_NetworkAdapter",	L"MACAddress");   }

static string Hash(string input) {
	MD5 ^hasher = MD5::Create();
	array<Byte> ^data = hasher->ComputeHash(Encoding::UTF8->GetBytes(input));
	StringBuilder ^s = gcnew StringBuilder();
	for (int i = 0; i < data->Length; i++)
		s->Append(data[i].ToString(L"x2"));
	return s->ToString();
}

string Res::GetUniqueID() {
	if (unique_id == nullptr) {
		try {
			unique_id = Hash(GetCpuId()+L":"+GetBIOSSerial()+L":"+GetMBSerial()+L":"+GetHdSerial());
		} catch (Exception^) {}
	}
	return unique_id;
}*/

array<string> ^Res::GetAvailableLanguages() {
	SortedList<string, string> ^l = gcnew SortedList<string, string>(); // CompareLocale
	Regex ^re = gcnew Regex(L"^[a-z]{2}(-[A-Z]{2})?$", RegexOptions::CultureInvariant);
	string dir = Path::GetDirectoryName(Assembly::GetExecutingAssembly()->Location);

	// Get embedded languages
	ResourceSet ^set = resources->GetResourceSet(System::Globalization::CultureInfo::InvariantCulture, true, false);
	for each (System::Collections::DictionaryEntry ^e in set) {
		string k = (string)e->Key;
		if (re->IsMatch(k))
			l[CultureInfo::GetCultureInfo(k)->NativeName] = k;
	}

	// Get external languages - messages.zip file
	array<string> ^names = Zip::GetFileNames(Path::Combine(dir, L"messages.zip"));
	if (names) {
		for each (string file in names) {
			string k = Path::GetFileName(file);
			if (k->EndsWith(L".txt") && re->IsMatch(k = k->Remove(k->Length-4)))
				l[CultureInfo::GetCultureInfo(k)->NativeName] = k;
		}
	}

	// Get external languages - individual files
	for each (string file in Directory::GetFiles(dir, L"??*.txt*")) {
		string k = Path::GetFileName(file);
		int len = k->Length;
		k = k->EndsWith(L".txt.gz") ? k->Remove(len-7) : (k->EndsWith(L".txt") ? k->Remove(len-4) : Path::GetFileNameWithoutExtension(k));
		if (re->IsMatch(k))
			l[CultureInfo::GetCultureInfo(k)->NativeName] = k;
	}

	// Copy to an array
	array<string> ^a = gcnew array<string>(l->Values->Count);
	l->Values->CopyTo(a, 0);
	return a;
}
Stream ^Res::GetLanguageStream(string lang) {
	string txt = lang+L".txt";
	if (File::Exists(txt))				return File::OpenRead(txt);
	else if (File::Exists(txt+L".gz"))	return gcnew GZipStream(File::OpenRead(txt+L".gz"), CompressionMode::Decompress);
	Stream ^s = Zip::GetStream(Path::Combine(Path::GetDirectoryName(Assembly::GetExecutingAssembly()->Location), L"messages.zip"), txt);
	return s ? s : GetCompressedStream(lang);
}

Stream ^Res::GetStream(string name) { return resources->GetStream(name); }
Stream ^Res::GetCompressedStream(string name) { return gcnew GZipStream(resources->GetStream(name), CompressionMode::Decompress); }
/*Stream ^Res::GetUpdate(string file) {
	Monitor::Enter(lock);
	string url = server+L"filename="+file+L"&locale="+System::Globalization::CultureInfo::InstalledUICulture->Name+L"&id="+GetUniqueID();
	if (first_request) {
		url += L"&initial";
		first_request = false;
	}
	Monitor::Exit(lock);
	HttpWebRequest ^r = (HttpWebRequest^)WebRequest::Create(url);
	r->Referer = referer;
	return r->GetResponse()->GetResponseStream();
}*/
PatchFile ^Res::GetPatch(string name) {
	return Patch::Load(resources->GetStream(name));

	/*Monitor::Enter(patches);
	if (!patches->ContainsKey(name)) {
		patches->Add(name, nullptr);
		Monitor::Exit(patches);

		PatchFile ^p = Patch::Load(resources->GetStream(name));

		Stream ^s;
		try {
			p = Patch::Load(s = GetUpdate(name), p);
		} catch (Exception^) { }
		if (s) s->Close();

		patches[name] = p;
	} else {
		Monitor::Exit(patches);
	}

	while (patches[name] == nullptr) { Yield(); }
	return patches[name];*/
}

/*System::Version ^Res::GetLatestAvailableVersion() {
	System::Version ^v = nullptr;
	Stream ^s;
	try {
		v = gcnew System::Version((gcnew StreamReader(s = GetUpdate(L"LATEST")))->ReadToEnd()->Trim());
	} catch (Exception^) { }
	if (s) s->Close();
	return (v && v > Version::Ver) ? v : nullptr;
}*/
