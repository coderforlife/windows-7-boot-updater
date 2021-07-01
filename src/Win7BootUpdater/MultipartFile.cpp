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

#include "MultipartFile.h"

using namespace Win7BootUpdater;

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Text;

MultipartFile::MultipartPart::MultipartPart(string id, string type, array<byte> ^data) {
	this->Id = id;
	this->Type = type;
	this->Data = data;
}

string MultipartFile::RandomBoundary() {
	StringBuilder ^sb = gcnew StringBuilder();
	for (int i = 0; i < 36; i++)
		sb->Append(Convert::ToChar(MultipartFile::rand->Next(46, 127))); //33 as lower bound?
	return sb->ToString();
}

MultipartFile::MultipartFile(Stream ^s) { Load(s); }
MultipartFile::MultipartFile() {
	this->multipart_type = MultipartFile::default_multipart_type;
	this->boundary = MultipartFile::RandomBoundary();
	this->parts = gcnew List<MultipartPart^>();
	BuildLookupTable();
}

void MultipartFile::BuildLookupTable() {
	if (this->id_lookup)	this->id_lookup->Clear();
	else					this->id_lookup = gcnew Dictionary<string, int>();
	if (this->type_lookup)	this->type_lookup->Clear();
	else					this->type_lookup = gcnew Dictionary<string, List<int>^>();
	for (int i = 0; i < this->parts->Count; ++i) {
		this->id_lookup->Add(this->parts[i]->Id, i);
		string type = this->parts[i]->Type;
		if (!this->type_lookup->ContainsKey(type))
			this->type_lookup->Add(type, gcnew List<int>());
		this->type_lookup[type]->Add(i);
	}
}

static array<byte> ^ReadLineBytes(Stream ^s) {
	// need to use a system that doesn't over-read
	// assumes ASCII/UTF8/UTF7 encoding (any encoding that represents a \n as 1 byte, so not UTF16)
	// the terminal \r\n or \n remain on the string (\r not supported)
	// returns null if already at the end of the stream

	List<byte> ^bytes = gcnew List<byte>();

	int b;
	while ((b = s->ReadByte()) >= 0) {
		bytes->Add((byte)b);
		if (b == '\n')
			break;
	}

	return (b == -1 && bytes->Count == 0) ? nullptr : bytes->ToArray();
}

static string ReadLine(Stream ^s) {
	// assumes ASCII encoding
	// trims the line
	// returns null if already at the end of the stream
	array<byte> ^b = ReadLineBytes(s);
	return b ? Encoding::ASCII->GetString(b)->Trim() : nullptr;
}

void MultipartFile::Load(Stream ^s) {
	string line;
	this->multipart_type = nullptr;

	// Read header
	while ((line = ReadLine(s)) != nullptr && line != L"") {
		// only care about the Content-Type header
		string line_ = line->ToLower();
		if (line_->StartsWith(L"content-type: multipart/") && !this->multipart_type) {
			this->boundary = nullptr;
			line = line->Substring(24);
			int i = line->IndexOf(L';');
			if (i > 0) {
				this->multipart_type = line->Remove(i);
				line = line->Substring(i+1)->Trim();
				if (line->StartsWith(L"boundary=")) {
					line = line->Substring(9);
					if (line[0] == L'"' || line[0] == L'\'') {
						i = line->IndexOf(line[0], 1);
						if (i > 0) {
							line = line->Substring(1);
							i -= 1;
						}
					} else {
						i = line->IndexOf(L';');
					}
					this->boundary = i > 0 ? line->Remove(i) : line;
				}
			} else {
				this->multipart_type = line;
			}
			if (!this->boundary)
				this->boundary = RandomBoundary();
		}
	}

	//if (!this->multipart_type) { throw gcnew FileFormatException(); }
	if (!this->multipart_type) { throw gcnew FormatException(); }

	string b = L"--"+this->boundary, end = b+L"--";

	// Scan for first part
	while ((line = ReadLine(s)) != nullptr && line != b && line != end); // throw away all extra data before first part

	// Read parts
	this->parts = gcnew List<MultipartPart^>();
	while (line && line != end) {
		// Read header
		string type = nullptr, id = nullptr;
		while ((line = ReadLine(s)) != nullptr && line != L"") {
			// only care about the Content-Type and Content-ID headers
			string line_ = line->ToLower();
			if (line_->StartsWith(L"content-type: ") && !type)	{ type = line->Substring(14)->Trim(); }
			else if (line_->StartsWith(L"content-id: ") && !id)	{ id = line->Substring(12)->Trim(); }
		}
		//if (!type) { throw gcnew FileFormatException(); }
		if (!type) { throw gcnew FormatException(); }
		
		// Read the data
		List<byte> ^data = gcnew List<byte>();
		array<byte> ^bline;
		while ((bline = ReadLineBytes(s)) != nullptr) {
			line = nullptr;
			if (bline->Length > 1 && bline[0] == '-' && bline[1] == '-') {
				try {
					line = Encoding::ASCII->GetString(bline)->Trim();
				} catch (Exception^) { }
			}
			if (line == b || line == end)
				break;
			data->AddRange(bline);
		}
		if (bline == nullptr)
			line = nullptr;
		this->parts->Add(gcnew MultipartPart(id, type, data->ToArray()));
	}

	BuildLookupTable();
}
void MultipartFile::Save(Stream ^s) {
	StreamWriter ^w = gcnew StreamWriter(s, Encoding::ASCII);

	w->WriteLine(L"MIME-Version: 1.0");
	w->WriteLine(L"Content-Type: multipart/"+this->multipart_type+L"; boundary=\""+this->boundary+L"\"");
	w->WriteLine();

	string b = L"--"+this->boundary;
	for (int i = 0; i < this->parts->Count; ++i) {
		MultipartPart ^p = this->parts[i];
		w->WriteLine(b);
		if (p->Id)
			w->WriteLine(L"Content-ID: "+p->Id);
		w->WriteLine(L"Content-Type: "+p->Type);
		w->WriteLine();
		w->Flush();
		s->Write(p->Data, 0, p->Data->Length);
		w->WriteLine();
	}
	w->Write(b+L"--");
	w->Flush();
}

string MultipartFile::MultipartType::get() { return this->multipart_type; }
void MultipartFile::MultipartType::set(string value) { this->multipart_type = value; }

string MultipartFile::Boundary::get() { return this->boundary; }
void MultipartFile::Boundary::set(string value) { this->boundary = String::IsNullOrEmpty(value) ? MultipartFile::RandomBoundary() : value; }

int MultipartFile::Count::get() { return this->parts->Count; }

bool MultipartFile::HasId(string id) { return this->id_lookup->ContainsKey(id); }
bool MultipartFile::HasType(string type) { return this->type_lookup->ContainsKey(type); }

string MultipartFile::Id::get(int i) { return this->parts[i]->Id; }
void MultipartFile::Id::set(int i, string value) {
	string old = this->parts[i]->Id;
	if (old != value) {
		if (value && this->id_lookup->ContainsKey(value)) { throw gcnew ArgumentException(); }
		if (old)	this->id_lookup->Remove(value);
		this->parts[i]->Id = value;
		if (value)	this->id_lookup->Add(value, i);
	}
}

string			MultipartFile::Type::get(int i)					{ return this->parts[i]->Type; }
array<byte> ^	MultipartFile::Data::get(int i)					{ return this->parts[i]->Data; }
string			MultipartFile::Type::get(string id)				{ return this->Type[this->id_lookup[id]]; }
array<byte> ^	MultipartFile::Data::get(string id)				{ return this->Data[this->id_lookup[id]]; }
void MultipartFile::Type::set(int i,		string value)		{ if (!value) { throw gcnew ArgumentNullException(); } this->parts[i]->Type = value; }
void MultipartFile::Data::set(int i,		array<byte> ^value)	{ if (!value) { throw gcnew ArgumentNullException(); } this->parts[i]->Data = value; }
void MultipartFile::Id  ::set(string id,	string value)		{ this->Id[this->id_lookup[id]] = value; }
void MultipartFile::Type::set(string id,	string value)		{ this->Type[this->id_lookup[id]] = value; }
void MultipartFile::Data::set(string id,	array<byte> ^value)	{ this->Data[this->id_lookup[id]] = value; }

string MultipartFile::FirstIdOfType(string type) { return this->Id[this->type_lookup[type][0]]; }

void MultipartFile::Add(string id, string type, array<byte> ^data) {
	if (!type || !data) { throw gcnew ArgumentNullException(); }
	if (id && this->id_lookup->ContainsKey(id)) { throw gcnew ArgumentException(); }

	this->parts->Add(gcnew MultipartPart(id, type, data));
	if (id) this->id_lookup->Add(id, this->parts->Count - 1);
}
void MultipartFile::Add(string type, array<byte> ^data) { this->Add(nullptr, type, data); }

void MultipartFile::Remove(int i) {
	this->parts->RemoveAt(i);
	BuildLookupTable();
}
void MultipartFile::Remove(string id) {
	this->parts->RemoveAt(this->id_lookup[id]);
	BuildLookupTable();
}
