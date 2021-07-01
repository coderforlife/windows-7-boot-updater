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

#include "Bootskin.h"

#include "Animation.h"
#include "MultipartFile.h"
#include "Resources.h"
#include "WinXXX.h"

using namespace Win7BootUpdater;

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Drawing;
using namespace System::IO;
using namespace System::IO::Compression;
using namespace System::Text;
using namespace System::Text::RegularExpressions;
using namespace System::Xml;

#define CLAMP(x, min, max) (x > max) ? max : ((x < min) ? min : x)
#define TO_HEX(x) x.ToString(L"X2")
#define FROM_HEX(s) Convert::ToInt32(s, 16)

inline static string ToStr(Color c) {
	return TO_HEX(c.R) + TO_HEX(c.G) + TO_HEX(c.B);
}
inline static Color ToColor(string s) {
	return Color::FromArgb(FROM_HEX(s->Substring(0, 2)), FROM_HEX(s->Substring(2, 2)), FROM_HEX(s->Substring(4, 2)));
}
inline static Color ToColor(XmlNode ^n) {
	return (n == nullptr) ? Color::Black : ToColor(n->InnerText);
}

BootSkin::BootSkin() {
	this->winload = gcnew BootSkinFile(false);
	this->winresume = gcnew BootSkinFile(true);
	this->Reset();
}
BootSkinFile ^BootSkin::WinXXX::get(unsigned long i) { return (i == 0) ? winload : ((i == 1) ? winresume : nullptr); }
BootSkinFile ^BootSkin::Winload::get() { return winload; }
BootSkinFile ^BootSkin::Winresume::get() { return winresume; }

BootSkinFile::BootSkinFile(bool winresume) {
	this->winresume = winresume;
	this->msgs = gcnew array<string>(2);
	this->fonts = gcnew array<Drawing::Font^>(2);
	this->textColors = gcnew array<SolidBrush^>(2);
	this->positions = gcnew array<int>(2);
	this->Reset();
}

void BootSkin::Reset() {
	winload->Reset();
	winresume->Reset();
}
void BootSkinFile::Reset() {
	this->defaultAnim = !winresume;
	this->winloadAnim = winresume;
	this->activity = nullptr;

	this->bg = nullptr;

	this->msgCount = 2;
	this->msgs[0] = WinXXX::CopyrightDefault;
	this->msgs[1] = WinXXX::GetDefaultStartupMsg(winresume)->Trim();
	FontFamily ^font = gcnew FontFamily(DefaultFont);
	this->fonts[0] = gcnew Drawing::Font(font, 11, FontStyle::Regular, GraphicsUnit::Point);
	this->fonts[1] = gcnew Drawing::Font(font, 18, FontStyle::Regular, GraphicsUnit::Point);
	this->textColors[0] = gcnew SolidBrush(Color::FromArgb(0x7F, 0x7F, 0x7F));
	this->textColors[1] = gcnew SolidBrush(Color::White);
	this->positions[0] = 718;
	this->positions[1] = 523;
	this->msgBgColor = gcnew SolidBrush(Color::Black);
	this->bgColor = gcnew SolidBrush(Color::Black);
}

static MemoryStream ^Copy(Stream ^in) {
	array<byte> ^buf = gcnew array<byte>(10240); // 10kb at a time
	MemoryStream ^ms = gcnew MemoryStream();

	int n;
	while ((n = in->Read(buf, 0, 10240)) > 0) ms->Write(buf, 0, n);

	in->Close();
	ms->Position = 0;
	return ms;
}

static string BytesToString(array<Byte> ^data, int len, Encoding ^default_encoding = Encoding::ASCII) {
	if (len == 0) return "";

	Encoding ^encoding = nullptr;

	Byte c = data[0];
	if (len > 2 && c == 0xEF && data[1] == 0xBB && data[2] == 0xBF)
		encoding = Encoding::UTF8;
	else if (len > 1) {
		if (c == 0xFE && data[1] == 0xFF)
			encoding = Encoding::BigEndianUnicode;
		else if (c == 0xFF && data[1] == 0xFE)
			encoding = Encoding::Unicode;
		else if (len > 3) {
			if (c == 0x00 && data[1] == 0x00 && data[2] == 0xFE && data[3] == 0xFF)
				encoding = gcnew UTF32Encoding(true, true); // TODO: verify
			else if (c == 0xFF && data[1] == 0xFE && data[2] == 0x00 && data[3] == 0x00)
				encoding = Encoding::UTF32;
			else if (c == 0x2B && data[1] == 0x2F && data[2] == 0x76 && (data[3] == 0x38 || data[3] == 0x39 || data[3] == 0x2B || data[3] == 0x2F))
				encoding = Encoding::UTF7;
		}
	}

	if (encoding == nullptr)
	{
		return default_encoding->GetString(data, 0, len);
	}
	else
	{
		int preamble_len = encoding->GetPreamble()->Length;
		return encoding->GetString(data, preamble_len, len - preamble_len);
	}
}

void BootSkin::Load(string winload, string winresume) {
	this->Reset();
	Win7BootUpdater::WinXXX::GetProperties(winload, this->winload);
	Win7BootUpdater::WinXXX::GetProperties(winresume ? winresume : winload, this->winresume);
}

string BootSkin::Load(string file) { return Load(gcnew FileStream(file, FileMode::Open, FileAccess::Read, FileShare::ReadWrite), false); }
string BootSkin::Load(Stream ^data) { return Load(data, false); }
string BootSkin::Load(Stream ^data, bool uncompressed) {
	try
	{
		MemoryStream ^x = Copy(data);

		// Determine file type (XML, Multipart, or a gzcompressed one of the following)
		array<Byte> ^start_bytes = gcnew array<Byte>(512);
		int count = x->Read(start_bytes, 0, 512);
		string start = BytesToString(start_bytes, count);
		x->Position = 0;

		//Regex ^is_xml = gcnew Regex(L"\\A\\s*<[?]xml[^?]*[?]>\\s*(<!--(-->|([^-]+-)+->)\\s*)*<\w+");
		Regex ^is_bs7_xml = gcnew Regex(L"\\A\\s*<[?]xml[^?]*[?]>\\s*(<!--(-->|([^-]+-)+->)\\s*)*<BootSkin7\\s+version=[\"']1[\"']\\s*>");
		Regex ^is_multipart_1 = gcnew Regex(L"\\AMIME-Version:[^\\S\n]+1.0[^\\S\n]*$", RegexOptions::IgnoreCase | RegexOptions::Multiline);
		Regex ^is_multipart_2 = gcnew Regex(L"^Content-Type:[^\\S\n]+multipart/\\w+;[^\\S\n]*boundary=", RegexOptions::IgnoreCase | RegexOptions::Multiline);

		if (is_bs7_xml->IsMatch(start))
		{
			return Load(x, nullptr);
		}
		else if (is_multipart_1->IsMatch(start) && is_multipart_2->IsMatch(start))
		{
			MultipartFile ^f = gcnew MultipartFile(x);
			return (f->Count <= 0) ? UI::GetMessage(Msg::ErrorLoadingBootSkin, "Invalid File") :
				Load(gcnew MemoryStream(f->HasId(L"bs7") ? f->Data[L"bs7"] : (f->HasType(L"application/xml") ? f->Data[f->FirstIdOfType(L"application/xml")] : f->Data[0])), f);
		}
		else if (!uncompressed) // assume gzcompressed xml or multipart
		{
			return Load(gcnew DeflateStream(x, CompressionMode::Decompress), true);
		}
		else
		{
			return UI::GetMessage(Msg::ErrorLoadingBootSkin, "Unknown format");
		}
	}
	catch (Exception^ ex) { return UI::GetMessage(Msg::ErrorLoadingBootSkin, ex->Message); }
}
string BootSkin::Load(Stream ^data, MultipartFile ^f) {
	XmlReader ^reader = nullptr;
	try {
		if (settings == nullptr) {
			// Prepare the XML Reader
			settings = gcnew XmlReaderSettings();
			settings->IgnoreWhitespace = true;
			settings->IgnoreComments = true;
			settings->ValidationType = ValidationType::Schema;

			// Parse the Schema
			settings->Schemas->Add(nullptr, gcnew XmlTextReader(Res::GetCompressedStream(L"bs7")));
		}

		// Load the XML file and validate it
		reader = XmlReader::Create(data, settings);

		// Create an XmlDocument (with DOM interface)
		XmlDocument ^xml = gcnew XmlDocument();
		xml->PreserveWhitespace = false;
		xml->Load(reader);
		XmlElement ^root = xml->DocumentElement;
		double version = Double::Parse(root->GetAttribute(L"version"));
		if (version != 1.0) { return UI::GetMessage(Msg::TheBootSkinIsNotAnUnderstoodVersion); }

		this->Reset();

		XmlElement ^xml_winload = (XmlElement^)root->SelectSingleNode(L"Winload");
		this->winload->Load(xml_winload, f);

		XmlElement ^xml_winresume = (XmlElement^)root->SelectSingleNode(L"Winresume");
		this->winresume->Load(xml_winresume ? xml_winresume : xml_winload, f);
	} catch (Schema::XmlSchemaException ^xmlSchEx) {
		return UI::GetMessage(Msg::TheFileCouldNotBeValidated, xmlSchEx->Message);
	} catch (XmlException ^xmlEx) {
		return UI::GetMessage(Msg::TheFileCouldNotBeRead, xmlEx->Message);
	} catch (Exception ^ex) {
		return UI::GetMessage(Msg::ErrorLoadingBootSkin, ex->Message);
	} finally {
		if (reader) reader->Close();
	}
	return nullptr;
}
void BootSkinFile::Load(XmlElement ^n, MultipartFile ^f) {
	this->BackColor = ToColor(n->SelectSingleNode(L"BackgroundColor"));

	XmlElement ^anim = (XmlElement^)n->SelectSingleNode(L"Animation");
	bool do_winloadanim = winresume;
	if (anim) {
		string s = anim->GetAttribute(L"source");
		if (String::IsNullOrEmpty(s) || s->Equals(L"embedded")) {
			string cid = anim->GetAttribute(L"cid");
			array<Byte> ^png = String::IsNullOrEmpty(cid) ? Convert::FromBase64String(anim->InnerText) : f->Data[cid];
			this->Anim = Animation::CreateFromData(png);
			//delete png;
			do_winloadanim = false;
		} else if (s->Equals(L"winload")) {
			do_winloadanim = true;
		} else if (s->Equals(L"default")) {
			do_winloadanim = false;
		}
	}
	if (do_winloadanim) {
		if (!winresume)
			throw gcnew Exception(UI::GetMessage(Msg::TheAnimationSourceCannotBeWinloadForWinload));
		winloadAnim = true;
	}

	XmlElement ^background = (XmlElement^)n->SelectSingleNode(L"Background");
	if (background) {
		string cid = background->GetAttribute(L"cid");
		array<Byte> ^png = String::IsNullOrEmpty(cid) ? Convert::FromBase64String(background->InnerText) : f->Data[cid];
		this->bg = Animation::CreateFromData(png);
		//delete png;
	}

	XmlElement ^messages = (XmlElement^)n->SelectSingleNode(L"Messages");
	if (messages) {
		this->MessageBackColor = ToColor(messages->SelectSingleNode(L"BackgroundColor"));
		int count = 0;
		for each (XmlElement ^n in messages->SelectNodes(L"Message")) {
			++count;
			unsigned int id = UInt32::Parse(n->GetAttribute(L"id"))-1;
			this->Message[id] = n->SelectSingleNode(L"Text")->InnerText;
			this->Position[id] = Int32::Parse(n->SelectSingleNode(L"Position")->InnerText);
			this->TextColor[id] = ToColor(n->SelectSingleNode(L"TextColor"));
			this->TextSize[id] = Int32::Parse(n->SelectSingleNode(L"TextSize")->InnerText);
		}
		this->MessageCount = count;
	} else {
		this->MessageCount = 0;
	}
}

void BootSkin::Save(Stream ^data) { Save(data, false); }
string BootSkin::Save(string file) { return Save(file, false); }
string BootSkin::Save(string file, bool old_format) {
	try {
		Save(gcnew FileStream(file, FileMode::Create, FileAccess::Write), old_format);
	} catch (Exception ^) {
		return UI::GetMessage(Msg::TheFileCouldNotBeOpenedForWriting, file);
	}
	return nullptr;
}
void BootSkin::Save(Stream ^data, bool old_format) {
	if (old_format) {
		Save(data, nullptr);
		return;
	}

	MultipartFile ^f = gcnew MultipartFile();
	f->Add(L"bs7", L"application/xml", gcnew array<byte>(0)); // add dummy empty array so that the bs7 entry is first

	MemoryStream ^mem = gcnew MemoryStream();
	Save(mem, f);
	f->Data[L"bs7"] = mem->ToArray();

	f->Save(data);
}
void BootSkin::Save(Stream ^data, MultipartFile ^f) {
	XmlTextWriter ^xml = gcnew XmlTextWriter(data, Encoding::UTF8);
	xml->WriteStartDocument();
	xml->WriteStartElement(L"BootSkin7");
	xml->WriteAttributeString(L"version", L"1");

	xml->WriteStartElement(L"Winload");
	this->winload->Save(xml, f);
	xml->WriteEndElement(); // Winload

	xml->WriteStartElement(L"Winresume");
	this->winresume->Save(xml, f);
	xml->WriteEndElement(); // Winresume

	xml->WriteEndElement(); // BootSkin7
	xml->WriteEndDocument();
	xml->Close();
}

void BootSkinFile::Save(XmlTextWriter ^xml, MultipartFile ^f) {
	if (!defaultAnim) {
		xml->WriteStartElement(L"Animation");
		if (winloadAnim) {
			xml->WriteAttributeString(L"source", L"winload");
		} else {
			array<Byte> ^png = Animation::GetPngData(this->Anim);
			if (f) {
				string cid = (winresume ? L"wr" : L"wl") + L"-anim";
				f->Add(cid, L"image/png", png);
				xml->WriteAttributeString(L"cid", cid);
			} else {
				xml->WriteBase64(png, 0, png->Length);
			}
			//delete png;
		}
		xml->WriteEndElement(); // Animation
	}

	xml->WriteElementString(L"BackgroundColor", ToStr(this->BackColor));

	if (this->UsesBackgroundImage()) {
		xml->WriteStartElement(L"Background");
		array<Byte> ^png = Animation::GetPngData(this->Background);
		if (f) {
			string cid = (winresume ? L"wr" : L"wl") + L"-bg";
			f->Add(cid, L"image/png", png);
			xml->WriteAttributeString(L"cid", cid);
		} else {
			xml->WriteBase64(png, 0, png->Length);
		}
		//delete png;
		xml->WriteEndElement(); // Background
	} else {
		xml->WriteStartElement(L"Messages");
		xml->WriteElementString(L"BackgroundColor", ToStr(this->MessageBackColor));
		for (unsigned int i = 0; i < (unsigned int)this->MessageCount; ++i) {
			xml->WriteStartElement(L"Message");
			xml->WriteAttributeString(L"id", (i+1).ToString());
			xml->WriteElementString(L"Text", this->Message[i]);
			xml->WriteElementString(L"Position", this->Position[i].ToString());
			xml->WriteElementString(L"TextColor", ToStr(this->TextColor[i]));
			xml->WriteElementString(L"TextSize", this->TextSize[i].ToString());
			xml->WriteEndElement(); // Message
		}
		xml->WriteEndElement(); // Messages
	}
}

bool BootSkinFile::IsWinresume() { return this->winresume; }

bool BootSkinFile::IsDefaultAnim() { return defaultAnim; }
void BootSkinFile::UseDefaultAnim() { activity = nullptr; defaultAnim = true; winloadAnim = false; }
bool BootSkinFile::IsWinloadAnim() { return winloadAnim; }
void BootSkinFile::UseWinloadAnim() { if (winresume) { activity = nullptr; winloadAnim = true; defaultAnim = false; } }
bool BootSkinFile::AnimIsNotSet() { return defaultAnim || winloadAnim; }
Image ^BootSkinFile::Anim::get() { return activity; }
void BootSkinFile::Anim::set(Image ^value) { activity = value; defaultAnim = false; winloadAnim = false; }

bool BootSkinFile::UsesBackgroundImage() { return bg != nullptr; }
Image ^BootSkinFile::Background::get() { return bg; }
void BootSkinFile::Background::set(Image ^value) { bg = value; }

int BootSkinFile::MessageCount::get() { return msgCount; }
void BootSkinFile::MessageCount::set(int value) { msgCount = CLAMP(value, 0, 2); }

string BootSkinFile::Message::get(unsigned long i) { return msgs[i]; }
void BootSkinFile::Message::set(unsigned long i, string value) { msgs[i] = value; }

Font ^BootSkinFile::Font::get(unsigned long i) { return fonts[i]; }

int BootSkinFile::TextSize::get(unsigned long i) { return (int)(fonts[i]->Size+0.1); }
void BootSkinFile::TextSize::set(unsigned long i, int value) { this->fonts[i] = gcnew Drawing::Font(this->fonts[i]->FontFamily, (float)value, this->fonts[i]->Style, GraphicsUnit::Point); }
array<int> ^BootSkinFile::TextSizes::get() { return gcnew array<int>{(int)(fonts[0]->Size+0.1), (int)(fonts[1]->Size+0.1)}; }

Color BootSkinFile::TextColor::get(unsigned long i) { return textColors[i]->Color; }
Brush^BootSkinFile::TextBrush::get(unsigned long i) { return textColors[i]; }
void BootSkinFile::TextColor::set(unsigned long i, Color value) { textColors[i]->Color = value; }
array<Color> ^BootSkinFile::TextColors::get() { return gcnew array<Color>{textColors[0]->Color, textColors[1]->Color}; }

Color BootSkinFile::MessageBackColor::get() { return msgBgColor->Color; }
Brush^BootSkinFile::MessageBackBrush::get() { return msgBgColor; }
void BootSkinFile::MessageBackColor::set(Color value) { msgBgColor->Color = value; }

int BootSkinFile::Position::get(unsigned long i) { return positions[i]; }
void BootSkinFile::Position::set(unsigned long i, int value) { positions[i] = value; }
array<int> ^BootSkinFile::Positions::get() { return gcnew array<int>{positions[0], positions[1]}; }

Color BootSkinFile::BackColor::get() { return bgColor->Color; }
Brush^BootSkinFile::BackBrush::get() { return bgColor; }
void BootSkinFile::BackColor::set(Color value) { bgColor->Color = WinXXX::GetClosestBGColor(value); }

