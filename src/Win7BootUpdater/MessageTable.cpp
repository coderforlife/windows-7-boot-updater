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

#include "MessageTable.h"

using namespace Win7BootUpdater;

using namespace System;

inline static WORD align4(WORD x) {
	WORD r = x % 4;
	return r == 0 ? x : (x+4-r);
}

static string GetStringW(BYTE *text) {
	WCHAR *s = (WCHAR*)text;
	size_t i = wcslen(s)+1;
	BOOL end = TRUE;
	WCHAR *x = new WCHAR[i];
	do {
		--i;
		x[i] = (end && (end = (s[i] == 0 || s[i] == L'\n' || s[i] == L'\r')) != FALSE) ? 0: s[i];
	} while (i);
	return gcnew String(x);
}

static string GetStringA(BYTE *text) {
	CHAR *s = (CHAR*)text;
	size_t i = strlen(s)+1;
	BOOL end = TRUE;
	WCHAR *x = new WCHAR[i];
	do {
		--i;
		x[i] = (end && (end = (s[i] == 0 || s[i] == '\n' || s[i] == '\r')) != FALSE) ? 0: s[i];
	} while (i);
	return gcnew String(x);
}

/////////////////// Message Table /////////////////////////////////////////////
MessageTable::MessageTable(unsigned char *x, size_t) {
	uint i, j, offset;

	MESSAGE_RESOURCE_DATA *data = (MESSAGE_RESOURCE_DATA*)x;
	this->blocks = gcnew array<Block^>(data->NumberOfBlocks);

	for (i = 0; i < (uint)this->blocks->Length; ++i) {
		MESSAGE_RESOURCE_BLOCK *block = (MESSAGE_RESOURCE_BLOCK*)data->Blocks+i;
		this->blocks[i] = gcnew Block(block->LowId, block->HighId);
		offset = block->OffsetToEntries;
		for (j = 0; j < this->blocks[i]->Count(); ++j) {
			MESSAGE_RESOURCE_ENTRY *entry = (MESSAGE_RESOURCE_ENTRY*)(x+offset);
			this->blocks[i][j] = (entry->Flags & MESSAGE_RESOURCE_UNICODE) ? GetStringW(entry->Text) : GetStringA(entry->Text);
			offset += entry->Length;
		}
	}
}

//MessageTable::~MessageTable() {
//	for (int /*unsigned long*/ i = 0; i < this->blocks->Length; ++i) {
//		if (this->blocks[i]) delete this->blocks[i];
//	}
	//delete[] this->blocks;
//}

unsigned char *MessageTable::compile(size_t *l) {
	uint off = this->blocks->Length*sizeof(MESSAGE_RESOURCE_BLOCK)+sizeof(DWORD);
	size_t sz = off;
	MESSAGE_RESOURCE_DATA *data;
	
	for (uint i = 0; i < (uint)this->blocks->Length; ++i) {
		sz += 2*this->blocks[i]->Count()*sizeof(WORD);
		for (uint j = 0; j < this->blocks[i]->Count(); ++j) {
			sz += align4((WORD)((this->blocks[i][j]->Length+3)*sizeof(WCHAR))); // + 3 for 0D 0A 00
		}
	}

	*l = sz;
	unsigned char *x = (unsigned char*)malloc(sz);
	memset(x, 0, sz);

	data = (MESSAGE_RESOURCE_DATA*)x;
	data->NumberOfBlocks = this->blocks->Length;

	for (uint i = 0; i < (uint)this->blocks->Length; ++i) {
		data->Blocks[i].LowId = this->blocks[i]->LowId();
		data->Blocks[i].HighId = this->blocks[i]->HighId();
		data->Blocks[i].OffsetToEntries = off;
		for (uint j = 0; j < this->blocks[i]->Count(); ++j) {
			MESSAGE_RESOURCE_ENTRY *entry = (MESSAGE_RESOURCE_ENTRY*)(x+off);
			string e = this->blocks[i][j]+L"\r\n";
			entry->Flags = MESSAGE_RESOURCE_UNICODE;
			entry->Length = align4((WORD)((e->Length+1)*sizeof(WCHAR)))+2*sizeof(WORD);
			wcscpy((WCHAR*)entry->Text, as_native(e));
			off += entry->Length;
		}
	}

	return x;
}
MessageTable::Block ^MessageTable::GetBlock(uint id) {
	for each (Block ^b in this->blocks)
		if (b->ContainsId(id))
			return b;
	return nullptr;
}
bool MessageTable::ContainsId(uint id) { return GetBlock(id) != nullptr; }
string MessageTable::default::get(uint id) { Block ^b = GetBlock(id); return b ? b->Entry[id] : nullptr; }
void MessageTable::default::set(uint id, string s) { Block ^b = GetBlock(id); if (b) { b->Entry[id] = s; } else { throw gcnew ArgumentException(); }; }

/////////////////// Message Table Block ///////////////////////////////////////
MessageTable::Block::Block(uint lowId, uint highId) : lowId(lowId) { this->entries = gcnew array<String^>(highId-lowId+1); }
//MessageTable::Block::~Block() { delete[] this->entries; }
uint MessageTable::Block::LowId() { return lowId; }
uint MessageTable::Block::HighId() { return lowId + this->entries->Length - 1; }
bool MessageTable::Block::ContainsId(uint id) { return id >= lowId && id < lowId + this->entries->Length; }
uint MessageTable::Block::Count() { return this->entries->Length; }
void MessageTable::Block::SetLowId(uint id) { this->lowId = id; }
string MessageTable::Block::default::get(uint i) { return entries[i]; }
void MessageTable::Block::default::set(uint i, string entry) { entries[i] = entry; }
string MessageTable::Block::Entry::get(uint id) { return entries[id-lowId]; }
void MessageTable::Block::Entry::set(uint id, string entry) { entries[id-lowId] = entry; }
