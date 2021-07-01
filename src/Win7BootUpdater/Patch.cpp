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

#include "Patch.h"

#include "UI.h"

#include "Utilities.h"
#include "Bytes.h"

#include "PDB.h"


using namespace Win7BootUpdater;
using namespace Win7BootUpdater::Patches;

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::IO::Compression;
using namespace System::Reflection;
using namespace System::Text;

#define ALIGNMENT 4

template<typename T> static bool Equal(array<T> ^a, array<T> ^b) {
	if (a->Length != b->Length) return false;
	EqualityComparer<T> ^q = EqualityComparer<T>::Default;
	for (int i = 0; i < a->Length; i++)
		if (!q->Equals(a[i], b[i]))
			return false;
	return true;
}

static array<bool> ^FalseBoolArray(int l, array<ushort> ^pos) {
	array<bool> ^x = gcnew array<bool>(l);
	Array::Clear(x, 0, l);
	if (pos) {
		for (int i = 0; i < pos->Length; ++i) {
			int k = pos[i];
			x[k] = x[k+1] = x[k+2] = x[k+3] = true;
		}
	}
	return x;
}

static void CheckPos(array<Byte> ^x, ... array<ushort> ^pos) {
	UInt32 len = (UInt32)x->Length;
	for (int i = 0; i < pos->Length; ++i)
		if (pos[i]+sizeof(uint) > len) { throw gcnew Exception(UI::GetMessage(Msg::LoadingPatchFailed)); }
}

UInt16 Patch::Type::get() {
	return (UInt16)this->GetType()->GetField(L"Type", BindingFlags::Static | BindingFlags::Public | BindingFlags::DeclaredOnly)->GetValue(this);
}

static array<Byte> ^AsBytes(String ^s, int max) {
	array<Byte> ^b = Encoding::Unicode->GetBytes(s);
	int cb = b->Length;
	if (max <= 0) max = s->Length;
	max = (max+1)*2;
	Array::Resize(b, max);
	Array::Clear(b, cb, max-cb);
	return b;
}

static array<Byte> ^GetWildcard(array<Byte> ^_x, Byte %wildcard, ... array<ushort> ^wildcardPos) { // the wildcardPos is the start of every 4 bytes
	int i, j = 0;
	bool n[256];
	ZeroMemory(n, sizeof(n));
	wildcard = 0;
	array<Byte> ^x = (array<Byte>^)_x->Clone();
	for (i = 0; i < x->Length; ++i) { // find all of the values in the list
		if (j < wildcardPos->Length && i == wildcardPos[j]) { ++j; i += 3; }
		else
			n[x[i]] = true;
	}
	for (i = 0; i < 256; ++i) { if (!n[i]) { wildcard = (Byte)i; break; } } // find the wildcard
	if (i == 256) { return nullptr; } // no possible wildcards
	for (j = 0; j < wildcardPos->Length; ++j) { // memset(x+wildcardPos[j], wildcard, 4);
		x[wildcardPos[j]  ] = wildcard;
		x[wildcardPos[j]+1] = wildcard;
		x[wildcardPos[j]+2] = wildcard;
		x[wildcardPos[j]+3] = wildcard;
	}
	return x;
}


///////////////////////////////////////////////////////////////////////////////
///// Reading functions
///////////////////////////////////////////////////////////////////////////////
static array<char> ^ReadSection(BinaryReader ^b) {
	char x = b->ReadSByte(), y = b->ReadSByte();
	if (x == 0 && y != 0) return Sections::All[y];
	//b->BaseStream->Seek(-2, SeekOrigin::Current); // network streams don't support seek
	//return (array<char>^)b->ReadBytes(IMAGE_SIZEOF_SHORT_NAME);
	array<char> ^a = gcnew array<char>(IMAGE_SIZEOF_SHORT_NAME);
	a[0] = x; a[1] = y;
	b->Read((array<Byte>^)a, 2, IMAGE_SIZEOF_SHORT_NAME-2);
	return a;
}
inline static array<Byte> ^ReadBytes(BinaryReader ^b) { return b->ReadBytes(b->ReadUInt16()); }
static array<UInt16> ^ReadUInt16s(BinaryReader ^b) {
	UInt16 l = b->ReadUInt16();
	array<UInt16> ^a = gcnew array<UInt16>(l);
	for (UInt16 i = 0; i < l; ++i)
		a[i] = b->ReadUInt16();
	return a;
}
/*static String ^ReadString(BinaryReader ^b) {
	List<wchar_t> ^s = gcnew List<wchar_t>();
	wchar_t c = 0;
	do {
		s->Add(c = b->ReadChar());
	} while (c);
	return gcnew String(s->ToArray());
}*/


///////////////////////////////////////////////////////////////////////////////
///// Persistent Data functions
///////////////////////////////////////////////////////////////////////////////
#pragma unmanaged
typedef struct _PersistentDataHeader {
	ushort id;
	ushort len; // includes the size of the header, so minimum is sizeof(PersistentDataHeader)
} PersistentDataHeader;
static void *GetPersistentData(PEFile *f, ushort id, ushort *len) {
	if (!f->hasExtraData()) { return NULL; }
	DWORD sz = 0;
	unsigned char *x = (unsigned char*)f->getExtraData(&sz), *end = x + sz;
	if (!x)	{ return NULL; }
	PersistentDataHeader *hdr = (PersistentDataHeader*)x;
	while ((unsigned char*)hdr < end && hdr->len >= sizeof(PersistentDataHeader) && hdr->id != id) {
		hdr = (PersistentDataHeader*)((unsigned char*)hdr+hdr->len);
	}

	size_t pos = (unsigned char*)hdr - x;
	bool found = (unsigned char*)hdr < end && pos + hdr->len <= sz && hdr->len >= sizeof(PersistentDataHeader) && hdr->id == id;
	if (found) {
		*len = hdr->len - sizeof(PersistentDataHeader);
		return hdr+1;
	}
	return NULL;
}
static bool SetPersistentData(PEFile *f, ushort id, void *data, ushort len) { // if data==NULL and len==0 then the entry is removed
	bool remove = data == NULL && len == 0;
	DWORD sz = 0;
	unsigned char *x = (unsigned char*)f->getExtraData(&sz), *end = x + sz;
	if (!x)	{ return false; }
	PersistentDataHeader *hdr = (PersistentDataHeader*)x;
	while ((unsigned char*)hdr < end && hdr->len >= sizeof(PersistentDataHeader) && hdr->id != id) {
		hdr = (PersistentDataHeader*)((unsigned char*)hdr+hdr->len);
	}

	if ((unsigned char*)hdr >= end) { return false; } // something isn't formatted right

	size_t pos = (unsigned char*)hdr - x;
	bool found = pos + hdr->len <= sz && hdr->len >= sizeof(PersistentDataHeader) && hdr->id == id;
	if (!remove && (pos + len + sizeof(PersistentDataHeader) - (found ? hdr->len : 0)) > sz) { return false; }

	if (remove) {
		if (found) {
			memmove(x+pos, x+pos+hdr->len, sz-pos-hdr->len);
			memset(x+sz-hdr->len, 0, hdr->len);
		}
	} else {
		PersistentDataHeader h;
		h.id = id;
		h.len = len + sizeof(PersistentDataHeader);
		if (found && h.len != hdr->len) {
			if (h.len > hdr->len) {
				memmove(x+pos+h.len, x+pos+hdr->len, sz-pos-h.len);
			} else { // h.len < hdr->len
				memmove(x+pos+h.len, x+pos+hdr->len, sz-pos-hdr->len);
				memset(x+sz-hdr->len+h.len, 0, hdr->len-h.len);
			}
		}
		memcpy(x+pos, &h, sizeof(PersistentDataHeader));
		memcpy(x+pos+sizeof(PersistentDataHeader), data, len);
	}
	return true;
}
#pragma managed


///////////////////////////////////////////////////////////////////////////////
///// Updating functions
///////////////////////////////////////////////////////////////////////////////
inline static void SetDword(array<Byte> ^a, UInt16 pos, uint d) {
	//for (int i = 0; i < 4; ++i)
	//	a[pos+i] = 0xFF & (d >> i*8);
	a[pos  ] = 0xFF & d;
	a[pos+1] = 0xFF & (d >> 8);
	a[pos+2] = 0xFF & (d >> 16);
	a[pos+3] = 0xFF & (d >> 24);
}
inline static uint GetDword(array<Byte> ^a, UInt16 pos) {
	//uint d = 0xFF & a[pos];
	//for (int i = 1; i < 4; ++i)
	//	d |= (0xFF & a[pos+i]) << i*8;
	//return d;
	return a[pos] | a[pos+1] << 8 | a[pos+2] << 16 | a[pos+3] << 24;
}
inline static bool WriteAtAndRR(PEFile *f, array<Byte> ^x, uint p, IMAGE_SECTION_HEADER *sect) {
	uint va = sect->VirtualAddress + p - sect->PointerToRawData;
	return f->set(NATIVE(x), p) && f->removeRelocs(va, va+x->Length-1);
}
static bool UpdateBytes(PEFile *f, array<char> ^section, Byte wildcard, array<bool> ^already_changed, array<Byte> ^target, array<Byte> ^value) {
	// Read the section and find the string
	IMAGE_SECTION_HEADER *sect = f->getSectionHeader(as_native(section));
	if (sect == NULL)						{ return false; }
	uint size = sect->SizeOfRawData, pntr = sect->PointerToRawData;
	Bytes data(f->get(pntr), size), found = data.find(NATIVE(target), wildcard);
	if (found == NULL)						{ return false; }
	uint pos = (uint)(found-data)+pntr; // this is the position within the file of the target

	// Get data for the wildcards
	for (int i = 0; i < value->Length; i++)
		if (!already_changed[i] && target[i] == wildcard)
			value[i] = found[i];

	// Save the new value in it's place
	return WriteAtAndRR(f, value, pos, sect);
}
static array<Byte> ^RetrieveBytes(PEFile *f, array<char> ^section, Byte wildcard, array<Byte> ^target) {
	// Read the section and find the string
	IMAGE_SECTION_HEADER *sect = f->getSectionHeader(as_native(section));
	if (sect == NULL)						{ return nullptr; }
	uint size = sect->SizeOfRawData, pntr = sect->PointerToRawData;
	Bytes data(f->get(pntr), size), found = data.find(NATIVE(target), wildcard);
	if (found == NULL)						{ return nullptr; }

	// Return the found data
	return Utilities::GetManagedArray(found, target->Length);
}
static array<uint> ^ReadValues(PEFile *f, array<char> ^section, Byte wildcard, array<Byte> ^target, array<ushort> ^pos) {
	array<Byte> ^data = RetrieveBytes(f, section, wildcard, target);
	if (data == nullptr)				{ return nullptr; }
	array<uint> ^values = gcnew array<uint>(pos->Length);
	for (int i = 0; i < pos->Length; ++i)
		values[i] = GetDword(data, pos[i]);
	return values;
}

///////////////////////////////////////////////////////////////////////////////
///// Direct
///////////////////////////////////////////////////////////////////////////////
Types::Direct::Direct(BinaryReader ^b) {
	section = ReadSection(b);
	wildcard = b->ReadByte();
	target = ReadBytes(b);
	value = ReadBytes(b);
	already_changed = FalseBoolArray(target->Length, nullptr);
	if (target->Length != value->Length) { throw gcnew Exception(UI::GetMessage(Msg::LoadingPatchFailed)); }
}
bool Types::Direct::Apply(PEFile *f) { return UpdateBytes(f, section, wildcard, already_changed, target, value); }
bool Types::Direct::IsApplied(PEFile *f) { return RetrieveBytes(f, section, value[0], value) != nullptr; }


///////////////////////////////////////////////////////////////////////////////
///// Dwords
///////////////////////////////////////////////////////////////////////////////
Types::Dwords::Dwords(BinaryReader ^b) {
	section = ReadSection(b);
	pos = ReadUInt16s(b);
	wildcard = b->ReadByte();
	target = ReadBytes(b);
	CheckPos(target, pos);
	already_changed = FalseBoolArray(target->Length, pos);
}
UInt16 Types::Dwords::Count() { return (UInt16)pos->Length; }
bool Types::Dwords::Apply(PEFile *f, array<uint> ^values) {
	if (values->Length != pos->Length)	{ return false; }
	array<Byte> ^data = (array<Byte>^)target->Clone();
	for (int i = 0; i < pos->Length; ++i)
		SetDword(data, pos[i], values[i]);
	return UpdateBytes(f, section, wildcard, already_changed, target, data);
}
array<uint> ^Types::Dwords::GetValues(PEFile *f) { return ReadValues(f, section, wildcard, target, pos); }


///////////////////////////////////////////////////////////////////////////////
///// String
///////////////////////////////////////////////////////////////////////////////
Types::String::String(BinaryReader ^b) {
	section = ReadSection(b);
	pos = b->ReadUInt16();
	wildcard = b->ReadByte();
	target = ReadBytes(b);
}
bool Types::String::FindTarget(PEFile *f, array<byte> ^%target, uint *pos, int *data_i, uint *off, string %value) {
	// Read the text section and find the target
	IMAGE_SECTION_HEADER *sect = f->getSectionHeader(as_native(section));
	if (sect == NULL)	{ return false; }
	uint size = sect->SizeOfRawData, pntr = sect->PointerToRawData;
	Bytes x(f->get(pntr), size), found = x.find(NATIVE(this->target), wildcard);
	if (found == NULL)	{ return false; }
	*pos = (uint)(found-x); // position within text
	target = Utilities::GetManagedArray(found, this->target->Length);
	DWORD addr = GetDword(target, this->pos);
	if (f->is64bit()) // 64 bit uses a position-relative va which we need to convert to a relative va
		addr += sect->VirtualAddress + *pos + this->pos + 4; // f->getNtHeaders64()->OptionalHeader.ImageBase
	else // 32 bit uses a absolute va which we need to convert to a relative va
		addr -= f->getNtHeaders32()->OptionalHeader.ImageBase;

	// Find the data section
	sect = f->getSectionHeaderByRVA(addr, data_i);
	if (sect == NULL)	{ return false; }
	DWORD offset = addr - sect->VirtualAddress;
	*off = sect->PointerToRawData + offset;
	value = gcnew System::String((wchar_t*)f->get(*off));
	return (uint)value->Length <= sect->SizeOfRawData - offset;
}
bool Types::String::DoInPlacePatch(PEFile *f, string value, uint off, uint max) {
	return f->set(NATIVE(AsBytes(value, max)), off) ? true : false; // RemoveRelocs() is not used here
}
bool Types::String::DoMovePatch(PEFile *f, string value, array<byte> ^target, uint pos, int data_i) {
	array<Byte> ^str = AsBytes(value, -1);
	uint str_sz = str->Length;

	// Expand the data section (possibly)
	IMAGE_SECTION_HEADER *data = f->getExpandedSectionHdr(data_i, str_sz), *code = f->getSectionHeader(as_native(section));
	if (data == NULL || code == NULL)		{ return false; }

	uint base = f->is64bit() ? 0 : f->getNtHeaders32()->OptionalHeader.ImageBase; // 64-bit is not needed: (uint)f->getNtHeaders64().OptionalHeader.ImageBase
	uint vs = data->Misc.VirtualSize;
	uint str_pos = data->PointerToRawData + vs,	str_va = data->VirtualAddress + vs;
	uint cll_pos = code->PointerToRawData + pos,cll_va = code->VirtualAddress + pos + this->pos + 4;

	// Add the new string
	if (!f->set(NATIVE(str), str_pos))		{ return false; } // RemoveRelocs() is not used here

	// Update the reference to the string
	SetDword(target, this->pos, f->is64bit() ? (str_va-cll_va) : (str_va+base)); // xx 48 8D 15 [?? ?? ?? ??] xx (rel rel va) vs. xx 68 [?? ?? ?? ??] xx (abs rel va)
	if (!f->set(NATIVE(target), cll_pos))	{ return false; } // RemoveRelocs() is not used here

	// Update the virtual size
	data->Misc.VirtualSize += str_sz;

	return true;
}
bool Types::String::Apply(PEFile *f, string value) {
	array<byte> ^target;
	uint pos, off;
	int data_i;
	string cur_value;
	if (!FindTarget(f, target, &pos, &data_i, &off, cur_value)) return false;
	int max = cur_value->Length, len = value->Length;
	return (len > max) ? DoMovePatch(f, value, target, pos, data_i) : DoInPlacePatch(f, value, off, max);
}
string Types::String::GetValue(PEFile *f) {
	array<byte> ^target;
	uint pos, off;
	int data_i;
	string value;
	return FindTarget(f, target, &pos, &data_i, &off, value) ? value : nullptr;
}


///////////////////////////////////////////////////////////////////////////////
///// AddFunction
///////////////////////////////////////////////////////////////////////////////
Types::AddFunction::AddFunction(BinaryReader ^b) {
	section = ReadSection(b);
	wildcard = b->ReadByte();
	target = ReadBytes(b);
	call = ReadBytes(b);
	callPos = b->ReadUInt16();
	func = ReadBytes(b);
	patchPos = ReadUInt16s(b);
	funcPos = ReadUInt16s(b);

	CheckPos(call, callPos);
	CheckPos(func, patchPos);
	CheckPos(func, funcPos);

	// Get the names of the functions if available
	funcNames = gcnew array<array<byte>^>(funcPos->Length);
	for (int i = 0; i < funcPos->Length; i++) {
		if (GetDword(func, funcPos[i]) == 0) {
			array<byte>^ name = ReadBytes(b);
			// Make sure the names are null-terminated
			int len = name->Length;
			if (name[len - 1] != 0) {
				array<byte>^ temp = gcnew array<byte>(len+1);
				temp->CopyTo(temp, 0);
				temp[len] = 0;
				name = temp;
			}
			funcNames[i] = name;
		}
	}

	// Merge patchPos and funcPos into a single sorted array
	int i = 0, j = 0;
	allPos = gcnew array<ushort>(patchPos->Length + funcPos->Length);
	while (i < patchPos->Length && j < funcPos->Length) {
		if (patchPos[i] < funcPos[j])	allPos[i+j] = patchPos[i++];
		else							allPos[i+j] = funcPos[j++];
	}
	while (i < patchPos->Length)		allPos[i+j] = patchPos[i++];
	while (j < funcPos->Length)			allPos[i+j] = funcPos[j++];

	// Wildcard-ify the call and the function
	w_call = GetWildcard(call, call_wildcard, callPos);
	w_func = GetWildcard(func, func_wildcard, allPos);
	if (!w_call || !w_func)				{ throw gcnew Exception(UI::GetMessage(Msg::LoadingPatchFailed)); }
}
ushort Types::AddFunction::Id() {
	// Adapted from a simple hash function from Robert Sedgwicks Algorithms in C book.
	unsigned int a = 63689, hash = 0;
	int i;
	for (i = 0; i < section->Length; ++i) { hash = hash * a + section[i]; a *= 378551; }
	for (i = 0; i < target->Length;	 ++i) { hash = hash * a + target[i];  a *= 378551; }
	return (ushort)((((hash)&0xffff) + ((hash)>>16))&0xffff); // fit into a short
}
bool Types::AddFunction::Apply(PEFile *f, array<uint> ^values) {
	if (values->Length != patchPos->Length)			{ return false; }
	
	// Find the section that we will be modifying and adding to
	IMAGE_SECTION_HEADER *sect = f->getExpandedSectionHdr(as_native(section), func->Length);
	IMAGE_SECTION_HEADER *out = sect;
	if (sect == NULL)	{
		// The target section cannot be expanded by enough to fit the code so we
		// need to add a new section
		out = f->createSection(".w7bu", func->Length, CHARS_CODE_SECTION);
		if (out == NULL) { return false; } // failed to create/expand auxilary section
		sect = f->getSectionHeader(as_native(section));
		if (sect == NULL) { return false; } // target section even doesn't exist
	}

	// Read the section and find the target
	Bytes data(f->get(sect->PointerToRawData), sect->SizeOfRawData);
	Bytes found = data.find(NATIVE(target), wildcard);
	if (found == NULL)								{ return false; }
	uint pos = (uint)(found-data); // this is the position in the section of the target
	uint va_call = sect->VirtualAddress + pos;

	// Save the wildcarded values
	if (wildcard != target[0]) {
		ushort n = 0;
		byte vals[32];
		for (ushort i = 1; i < target->Length; ++i)
			if (target[i] == wildcard)
				vals[n++] = found[i];
		if (!SetPersistentData(f, Id(), vals, n))	{ return false; }
	}
	
	// Align addr of function to a uint boundary if space allows
	// addr is relative to the section it is in
	uint addr = out->Misc.VirtualSize, align = addr % ALIGNMENT;
	if (align) { // should align
		align = ALIGNMENT - align; // amount to move forward
		if (out->SizeOfRawData - addr - align >= (uint)func->Length)
			addr += align;
		else
			align = 0;
	}
	uint va_func = out->VirtualAddress + addr;

	// Create and save the new value in it's place
	array<Byte> ^call = (array<Byte>^)this->call->Clone();
	SetDword(call, callPos, va_func - va_call - 5);
	if (!WriteAtAndRR(f, call, sect->PointerToRawData+pos, sect)) { return false; }
	
	// Add the new function
	array<Byte> ^func = (array<Byte>^)this->func->Clone();
	for (int i = 0; i < patchPos->Length; ++i)
		SetDword(func, patchPos[i], values[i]);
	PDB *pdb = NULL;
	for (int i = 0; i < funcPos->Length; ++i) {
		uint va = GetDword(func, funcPos[i]);
		if (va == 0) { // Get virtual addresses from name going through debug information
			if (!pdb) { pdb = PDB::Get(f); if (!pdb) { return false; } }
			va = pdb->getFunctionVA((char*)as_native(funcNames[i]));
			if (!pdb->hasSectionHeaders()) { va += sect->VirtualAddress; } // assume it is in the original section
		}
		// Relative distance from call to function, from the end of the call
		SetDword(func, funcPos[i], va - va_func - funcPos[i] - 4);
	}
	if (pdb) { delete pdb; }
	if (!f->set(NATIVE(func), out->PointerToRawData+addr))	{ return false; } // no RemoveRelocs since the function is outside the scope

	// Update the virtual size
	out->Misc.VirtualSize += func->Length + align;

	return true;
}
bool Types::AddFunction::Revert(PEFile *f) {
	// Find the call target
	IMAGE_SECTION_HEADER *sect = f->getSectionHeader(as_native(section));
	if (sect == NULL)								{ return true; }
	Bytes data(f->get(sect->PointerToRawData), sect->SizeOfRawData);
	Bytes call_found = data.find(NATIVE(w_call), call_wildcard);
	if (call_found == NULL)							{ return true; }
	uint call_pos = (uint)(call_found-data); // this is the position in .text of the targets

	// Find the func target
	IMAGE_SECTION_HEADER *out = sect;
	Bytes func_found = data.find(NATIVE(w_func), func_wildcard);
	if (func_found == NULL) {
		out = f->getSectionHeader(".w7bu");
		if (out == NULL)							{ return true; }
		data = Bytes(f->get(out->PointerToRawData), out->SizeOfRawData);
		func_found = data.find(NATIVE(w_func), func_wildcard);
		if (func_found == NULL)						{ return true; }
	}
	uint func_pos = (uint)(func_found-data);

	// Write the original
	uint a = sect->VirtualAddress + call_pos;
	if (!f->removeRelocs(a, a+w_call->Length, true)){ return false; }
	array<Byte> ^target = (array<Byte>^)this->target->Clone();
	if (wildcard != target[0]) { // reverse wildcard values
		ushort n = 0, t = 0, id = Id();
		byte *vals = (byte*)GetPersistentData(f, id, &t);
		if (vals == NULL)							{ return false; }
		ushort i;
		for (i = 1; i < target->Length; ++i) {
			if (target[i] == wildcard) {
				if (n >= t) { break; }
				target[i] = vals[n++];
			}
		}
		if (n != t || i != target->Length)			{ return false; }
		if (!SetPersistentData(f, id, NULL, 0))		{ return false; }
	}
	if (!f->set(NATIVE(target), sect->PointerToRawData + call_pos))	{ return false; }

	// Zero-out the function
	if (!f->zero(w_func->Length, out->PointerToRawData + func_pos))	{ return false; }

	// Reduce the virtual size to not include any trailing zeroes
	uint sz = out->Misc.VirtualSize-func_pos, i;
	byte* end = f->get(out->PointerToRawData + func_pos);
	for (i = sz; i > 0 && !end[i-1]; --i);
	out->Misc.VirtualSize -= sz-i;

	return true;
}
array<uint> ^Types::AddFunction::GetValues(PEFile *f) { return ReadValues(f, section, func_wildcard, w_func, patchPos); }


///////////////////////////////////////////////////////////////////////////////
///// Structure: PatchVersion
///////////////////////////////////////////////////////////////////////////////
PatchVersion::PatchVersion(BinaryReader ^b) {
	min = b->ReadUInt64();
	max = b->ReadUInt64();
	switch (b->ReadUInt16()) {
	case Types::Direct::Type:		patch = gcnew Types::Direct(b);			break;
	case Types::Dwords::Type:		patch = gcnew Types::Dwords(b);			break;
	case Types::String::Type:		patch = gcnew Types::String(b);			break;
	case Types::AddFunction::Type:	patch = gcnew Types::AddFunction(b);	break;
	default: throw gcnew Exception(UI::GetMessage(Msg::LoadingPatchFailed));
	}
}
UInt64 PatchVersion::Min::get() { return min; }
UInt64 PatchVersion::Max::get() { return max; }
bool PatchVersion::NoMax::get() { return max == 0; }
Patch ^PatchVersion::Get() { return patch; }

///////////////////////////////////////////////////////////////////////////////
///// Structure: PatchPlatform
///////////////////////////////////////////////////////////////////////////////
PatchPlatform::PatchPlatform(BinaryReader ^b) {
	platform = b->ReadUInt16();
	UInt16 l = b->ReadUInt16();
	versions = gcnew array<PatchVersion^>(l);
	for (int i = 0; i < l; ++i)
		versions[i] = gcnew PatchVersion(b);
}
UInt16 PatchPlatform::Type::get() { return platform; }
array<PatchVersion^> ^PatchPlatform::Get(UInt64 version) {
	List<PatchVersion^> ^results = gcnew List<PatchVersion^>();
	for each (PatchVersion ^v in versions)
		if (v->Min <= version && (v->NoMax || v->Max >= version))
			results->Add(v);
	return results->ToArray();
}
array<Patch^> ^PatchPlatform::GetPatches(UInt64 version) {
	List<Patch^> ^results = gcnew List<Patch^>();
	for each (PatchVersion ^v in versions)
		if (v->Min <= version && (v->NoMax || v->Max > version))
			results->Add(v->Get());
	return results->ToArray();
}

///////////////////////////////////////////////////////////////////////////////
///// Structure: PatchEntry
///////////////////////////////////////////////////////////////////////////////
PatchEntry::PatchEntry(BinaryReader ^b) {
	id = b->ReadUInt16();
	UInt16 l = b->ReadUInt16();
	platforms = gcnew array<PatchPlatform^>(l);
	for (int i = 0; i < l; ++i)
		platforms[i] = gcnew PatchPlatform(b);
}
UInt16 PatchEntry::Id::get() { return id; }
PatchPlatform ^PatchEntry::Get(UInt16 platform) {
	for each (PatchPlatform ^p in platforms)
		if (p->Type == platform)
			return p;
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///// Structure: PatchFile
///////////////////////////////////////////////////////////////////////////////
void PatchFile::Init1(Stream ^s) {
	BinaryReader ^b = gcnew BinaryReader(s);
	if (b->ReadUInt16() != PATCH_MAGIC) { throw gcnew Exception(UI::GetMessage(Msg::LoadingPatchFailed)); }
	format_major = b->ReadUInt16();
	format_minor = b->ReadUInt16();
	if (format_major != 0 || format_minor > 3) { throw gcnew Exception(UI::GetMessage(Msg::ThePatchVersionIsNewerThanThisProgramCanUnderstand)); }
	file_major = b->ReadUInt16();
	file_minor = b->ReadUInt16();
	compression = b->ReadUInt16();
}
void PatchFile::Init2(Stream ^s) {
	switch (compression) {
		case Compressions::None: break;
		case Compressions::GZip:	s = gcnew GZipStream   (s, CompressionMode::Decompress); break;
		case Compressions::Deflate:	s = gcnew DeflateStream(s, CompressionMode::Decompress); break;
		default: throw gcnew Exception(UI::GetMessage(Msg::LoadingPatchFailed));
	}
	BinaryReader ^b = gcnew BinaryReader(s);
	UInt16 l = b->ReadUInt16();
	entries = gcnew array<PatchEntry^>(l);
	for (int i = 0; i < l; ++i)
		entries[i] = gcnew PatchEntry(b);
	b->Close();
}
PatchFile::PatchFile(Stream ^s) { Init1(s); Init2(s); }
PatchFile::PatchFile(Stream ^s, UInt16 min_major, UInt16 min_minor) {
	Init1(s);
	if (file_major < min_major || file_major == min_major && file_minor <= min_minor) { throw gcnew Exception(UI::GetMessage(Msg::PatchOutOfDate)); }
	Init2(s);
}
array<PatchEntry^> ^PatchFile::Get(UInt16 id) {
	List<PatchEntry^> ^results = gcnew List<PatchEntry^>();
	for each (PatchEntry ^e in entries)
		if (e->Id == id)
			results->Add(e);
	return results->ToArray();
}
array<Patch^> ^PatchFile::Get(UInt16 id, UInt16 platform, UInt64 version) {
	List<Patch^> ^results = gcnew List<Patch^>();
	for each (PatchEntry ^e in Get(id)) {
		PatchPlatform ^p = e->Get(platform);
		if (p != nullptr)
			results->AddRange(p->GetPatches(version));
	}
	return results->ToArray();
}
array<Patch^> ^PatchFile::Get(PEFile *f, UInt16 id) { return Get(id, f->getFileHeader()->Machine, f->getFileVersion()); }
int PatchFile::CompareTo(PatchFile ^other) {
	if (other == nullptr) { return int::MaxValue; }
	return (other->file_major == this->file_major) ? this->file_minor.CompareTo(other->file_minor) : this->file_major.CompareTo(other->file_major);
}
UInt16 PatchFile::Major::get() { return file_major; }
UInt16 PatchFile::Minor::get() { return file_minor; }
String ^PatchFile::Version::get() { return file_major + L"." + file_minor; }


///////////////////////////////////////////////////////////////////////////////
///// Patch Loading Shortcuts
///////////////////////////////////////////////////////////////////////////////
PatchFile ^Patch::Load(Stream ^s)										{ return gcnew PatchFile(s); }
PatchFile ^Patch::Load(Stream ^s, PatchFile ^min)						{ return gcnew PatchFile(s, min->Major, min->Minor); }
PatchFile ^Patch::Load(Stream ^s, ushort min_major, ushort min_minor)	{ return gcnew PatchFile(s, min_major, min_minor); }


///////////////////////////////////////////////////////////////////////////////
///// Patching Shortcut Functions
///////////////////////////////////////////////////////////////////////////////
// Apply
bool PatchFile::Apply(PEFile *f) {
	PatchPlatform ^pp;
	UInt16 platform = f->getFileHeader()->Machine; //(UInt16)(f->is64bit() ? Platforms::AMD64 : Platforms::I386);
	UInt64 version = f->getFileVersion();
	for each (PatchEntry ^e in this->entries)
		if ((pp = e->Get(platform)) != nullptr)
			for each (Patch ^p in pp->GetPatches(version))
				if (p->Type == Types::Direct::Type)
					if (!((Types::Direct^)p)->Apply(f)) return false;
	return true;
}
bool PatchFile::ApplyIgnoringApplied(PEFile *f) {
	PatchPlatform ^pp;
	UInt16 platform = f->getFileHeader()->Machine; //(UInt16)(f->is64bit() ? Platforms::AMD64 : Platforms::I386);
	UInt64 version = f->getFileVersion();
	for each (PatchEntry ^e in this->entries)
		if ((pp = e->Get(platform)) != nullptr)
			for each (Patch ^p in pp->GetPatches(version))
				if (p->Type == Types::Direct::Type)
					if (!((Types::Direct^)p)->Apply(f) && !((Types::Direct^)p)->IsApplied(f)) return false;
	return true;
}
bool PatchFile::Apply(PEFile *f, UInt16 id) {
	for each (Patch ^p in Get(f, id)) {
		if (p->Type == Types::Direct::Type) {
			if (!((Types::Direct^)p)->Apply(f)) return false;
		} else if (p->Type == Types::AddFunction::Type) {
			if (!((Types::AddFunction^)p)->Apply(f)) return false;
		}
	}
	return true;
}
bool PatchFile::Apply(PEFile *f, UInt16 id, array<uint> ^values) {
	for each (Patch ^p in Get(f, id)) {
		if (p->Type == Types::Dwords::Type) {
			if (!((Types::Dwords^)p)->Apply(f, values)) return false;
		} else if (p->Type == Types::AddFunction::Type) {
			if (!((Types::AddFunction^)p)->Apply(f, values)) return false;
		}
	}
	return true;
}
bool PatchFile::Apply(PEFile *f, UInt16 id, uint value) {
	for each (Patch ^p in Get(f, id)) {
		if (p->Type == Types::Dwords::Type) {
			if (!((Types::Dwords^)p)->Apply(f, value)) return false;
		} else if (p->Type == Types::AddFunction::Type) {
			if (!((Types::AddFunction^)p)->Apply(f, value)) return false;
		}
	}
	return true;
}
bool PatchFile::Apply(PEFile *f, UInt16 id, String ^value) {
	for each (Patch ^p in Get(f, id))
		if (p->Type == Types::String::Type)
			if (!((Types::String^)p)->Apply(f, value)) return false;
	return true;
}
// Revert
bool PatchFile::Revert(PEFile *f, UInt16 id) {
	for each (Patch ^p in Get(f, id))
		if (p->Type == Types::AddFunction::Type)
			if (!((Types::AddFunction^)p)->Revert(f)) return false;
	return true;
}
// Retrieve
bool PatchFile::IsApplied(PEFile *f, ushort id) {
	for each (Patch ^p in Get(f, id)) {
		if (p->Type == Types::Direct::Type) {
			if (!((Types::Direct^)p)->IsApplied(f)) return false;
		} else if (p->Type == Types::Dwords::Type) {
			if (((Types::Dwords^)p)->GetValues(f) == nullptr) return false;
		} else if (p->Type == Types::String::Type) {
			if (((Types::String^)p)->GetValue(f) == nullptr) return false;
		} else if (p->Type == Types::AddFunction::Type) {
			if (((Types::AddFunction^)p)->GetValues(f) == nullptr) return false;
		}
	}
	return true;
}
array<uint> ^PatchFile::GetValues(PEFile *f, ushort id) {
	for each (Patch ^p in Get(f, id)) {
		if (p->Type == Types::Dwords::Type) {
			return ((Types::Dwords^)p)->GetValues(f);
		} else if (p->Type == Types::AddFunction::Type) {
			return ((Types::AddFunction^)p)->GetValues(f);
		}
	}
	return nullptr;
}
string PatchFile::GetValue(PEFile *f, ushort id) {
	for each (Patch ^p in Get(f, id))
		if (p->Type == Types::String::Type)
			return ((Types::String^)p)->GetValue(f);
	return nullptr;
}
bool PatchFile::Get(PEFile *f, ushort id, uint %val) {
	array<uint> ^vals = this->GetValues(f, id);
	if (vals != nullptr && vals->Length > 0) { val = vals[0]; return true; }
	return false;
}
bool PatchFile::Get(PEFile *f, ushort id, string %s) {
	string val = this->GetValue(f, id);
	if (val != nullptr) { val = s; return true; }
	return false;
}
