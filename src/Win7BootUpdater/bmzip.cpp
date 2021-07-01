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

#include "bmzip.h"

// Get the minimum of 2
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

// The character to fill in empty space
#define FILL_BYTE	' '

// Pre-calculated data for decompression and compression
const static USHORT pow2[]  = { 16,    32,    64,    128,   256,   512,   1024,  2048,  4096  };
const static USHORT masks[] = { 0xFFF, 0x7FF, 0x3FF, 0x1FF, 0x0FF, 0x07F, 0x03F, 0x01F, 0x00F };
const static USHORT shift[] = { 12,    11,    10,    9,     8,     7,     6,     5,     4     };


/////////////////// Dictionary Functions //////////////////////////////////////

#define MAX_BYTE	256		// maximum byte value (+1 for 0)
#define MAX_SIZE	4096	// maximum dictionary size
#define NONE		-1		// value for when a value is not present in the dictionary

// An entry within the dictionary
typedef struct _Entry {
	short size;
	USHORT pos[MAX_SIZE];
	short first[MAX_BYTE];
	short last[MAX_BYTE];
} Entry;

// The dictionary
typedef struct _Dictionary {
	Entry entries[MAX_BYTE];
} Dictionary;

// Creates and returns an uninitialized dictionary struct
static Dictionary *Dictionary_Create() { return (Dictionary*)malloc(sizeof(Dictionary)); }

// Destroys a dictionary struct
static void Dictionary_Destroy(Dictionary *d) { free(d); }

// Resets a dictionary struct ready to start a new fragment
static void Dictionary_Reset(Dictionary *d) {
	int i;
	for (i = 0; i < ARRAYSIZE(d->entries); ++i) {
		d->entries[i].size = 0;
		memset(d->entries[i].first, NONE, sizeof(d->entries[i].first));
	}
}

// Adds the symbol at u[pos] to the dictionary
static void Dictionary_Add(Dictionary *d, const BYTE *u, const USHORT pos, const ULONG len) {
	if (len-pos >= 3) {
		const BYTE x = u[pos], y = u[pos+1];
		Entry *e = d->entries+x;
		e->pos[e->size] = pos; // pos will never be >4096
		if (e->first[y] == NONE)
			e->first[y] = e->size;
		e->last[y] = ++e->size; // it is 1 past last
	}
}

// Finds the best symbol in the dictionary for the data at u[pos]
// Returns the length of the string found, or 0 if nothing of length >= 3 was found
// Val is set to the LZ-bootmgr coded value for the string, based on cur
static ULONG Dictionary_Find(const Dictionary *d, const BYTE *u, const ULONG pos, const ULONG len, const ULONG cur, ULONG *val) {
	ULONG max_len = len-pos;
	if (pos > 0 && max_len > 3) {
		const BYTE x = u[pos], y = u[pos+1];
		const Entry *e = d->entries+x;
		int ep = e->first[y];
		if (ep != NONE) { // a match is possible
			ULONG l = 0, o;
			const BYTE z = u[pos+2];
			int last = e->last[y];
			if (max_len > masks[cur]+3u) max_len = masks[cur]+3u;

			// try short repeats
			if (x == z && y == u[pos-1]) {
				if (x == y) { // x == y, x == z, x == u[pos-1]
					// repeating the last byte
					l = 3;
					o = pos-1;
					while (l < max_len && u[pos+l] == x)	{ ++l; }
					--last;
				} else if (pos > 1 && x == u[pos-2]) { // x == z, x == u[pos-2], y == u[pos-1]
					// repeating the last two bytes
					l = 3;
					o = pos-2;
					while (l < max_len && u[pos+l] == y)	{ ++l;
						if(l < max_len && u[pos+l] == x)	{ ++l; }
						else break;
					}
					--last;
				}
			}

			// do an exhaustive search (in the possible range)
			for (; ep < last; ++ep) {
				const ULONG p = e->pos[ep];
				if (u[p+1] == y && u[p+2] == z) {
					ULONG i = ((pos-p)==3 ? p : p+3), j = 3;
					while (j < max_len && u[pos+j] == u[i]) {
						++j;
						if (++i == pos) { i = p; } // allow looping back, can have l > o
					}
					if (j > l) { l = j; o = p; }
				}
			}

			if (l >= 3) {		
				*val = ((pos-o-1) << shift[cur]) | (l-3);
				return l;
			}
		}
	}
	return 0;
}


/////////////////// Compression Functions /////////////////////////////////////

static USHORT CompressFragment(const BYTE *u, ULONG u_len, BYTE *c, ULONG c_len, Dictionary *d) {
	ULONG c_pos = 0, cur = 0, i, pos, len, x, end;
	USHORT u_pos = 0;
	BYTE bits, bytes[16]; // If all are special, then it will fill 16 bytes

	while (c_pos < c_len && u_pos < u_len) {
		for (i = 0, pos = 0, bits = 0; i < 8 && c_pos < c_len && u_pos < u_len; ++i) { // Go through each bit
			bits >>= 1;

			while (cur < ARRAYSIZE(pow2) && pow2[cur] < u_pos) { ++cur; }

			if ((len = Dictionary_Find(d, u, u_pos, u_len, cur, &x)) > 0) {
				bytes[pos++] = x & 0xFF;
				bytes[pos++] = (x >> 8) & 0xFF;
				bits |= 0x80; // set the highest bit
			} else {
				// Copy directly
				len = 1;
				bytes[pos++] = u[u_pos];
			}

			// And new entries
			for (end = u_pos + len; u_pos < end; ++u_pos)
				Dictionary_Add(d, u, u_pos, u_len);
		}
		if (c_pos >= c_len) { break; }
		if (i != 8) {
			// We need to finish moving the value over
			bits >>= 8-i;
		}
		c[c_pos] = bits;
		memcpy(c+c_pos+1, bytes, pos);
		c_pos += pos+1;
	}

	// Return unsuccessful or size
	return (u_pos < u_len) ? 0 : (USHORT)c_pos;
}

ULONG bmzip_compress(const BYTE *u, ULONG u_len, BYTE *c, ULONG c_len) {
	ULONG c_pos = 0, u_pos = 0;
	USHORT c_size, u_size;
	BYTE flags;
	Dictionary *d = Dictionary_Create();

	while (c_pos < c_len-1 && u_pos < u_len) {
		// Compress the next fragment
		Dictionary_Reset(d);
		u_size = (USHORT)MIN(u_len-u_pos, 0x1000);
		c_size = CompressFragment(u+u_pos, u_size, c+c_pos+2, c_len-c_pos-2, d);
		if (c_size == 0) { break; }
		//if (c_size >= u_size) { _tprintf(TEXT("Warning: fragment should be uncompressed, however that flag is unknown (%04hx >= %04hx)\n"), c_size, u_size); }
		//if (c_size > 0xFFF) { _tprintf(TEXT("Warning: fragment is larger than allowed (%04hx >= 0x1000)!\n"), c_size); }
		flags = 0x0B;

		// Save header
		c[c_pos] = 0xFF & (c_size-1);
		c[c_pos+1] = ((flags << 4) & 0xF0) | (((c_size-1) >> 8) & 0x0F);

		// Increment positions
		c_pos += 2+c_size;
		u_pos += u_size;
	}

	Dictionary_Destroy(d);

	// Return unsuccessful or size
	if (u_pos < u_len) {
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return 0;
	}
	c[c_pos++] = 0; // We need an extra null byte at the end (7 does this, Vista does not)
	return c_pos;
}

Bytes bmzip_compress(const Bytes &u) {
	Bytes c = Bytes::alloc((*u)*2); // assume double is as big as compressed data is
	ULONG size = bmzip_compress(~u, *u, ~c, *c);
	if (size == 0) { free(c); return Bytes::Null; }
	c.setCount(size);
	return c;
}


/////////////////// Decompression Functions ///////////////////////////////////

static ULONG DecompressFragment(const BYTE *c, ULONG c_len, BYTE *u, ULONG u_len) {
	ULONG c_pos = 0, u_pos = 0, cur = 0, i, x, len, offset;
	BYTE bits;

	// Process all bytes
	while (c_pos < c_len && u_pos < u_len) {
		// Handle a chunk
		bits = c[c_pos++]; // Bit mask tells us how to handle the next 8 symbols
		for (i = MIN(8, c_len-c_pos); (i = MIN(i, c_len-c_pos)) > 0 && bits; --i, bits >>= 1) { // Go through each bit
			if (bits & 1) { // We have a look-behind compression symbol
				// Update the current power of two available bytes
				while (cur < ARRAYSIZE(pow2) && pow2[cur] < u_pos) { ++cur; }

				// Get the length and offset
				x = c[c_pos] | (c[c_pos+1] << 8);
				len = (x&masks[cur])+3;
				offset = (x>>shift[cur])+1;

				//if (offset != (offset & masks[8-cur])) { _tprintf(TEXT("Warning: offset needs masking\n")); offset &= masks[8-cur]; }
				
				if (offset > u_pos) {
					ULONG diff = offset - u_pos, to_set = len < diff ? len : diff;
					//_tprintf(TEXT("Warning: trying to read from beyond the beginning of the dictionary, will fill with zeroes\n"));
					memset(u+u_pos, FILL_BYTE, to_set);
					u_pos += to_set;
					len -= to_set;
					offset -= to_set;
				}

				if (offset == 1) {
					// Repeat the most recent byte len times
					// This is not necessary but much more efficient for this common case
					memset(u+u_pos, u[u_pos-1], len);
				} else if (offset != 0) {
					// Copy offset bytes at a time from offset ago to fill up len
					while (offset < len) { 
						memcpy(u+u_pos, u+u_pos-offset, offset);
						len -= offset;
						u_pos += offset;
					}

					// Copy the remaining len bytes from offset ago
					if (len > 0) {
						memcpy(u+u_pos, u+u_pos-offset, len);
					}
				}

				c_pos += 2;
				u_pos += len;

			} else {
				// Copy current byte
				u[u_pos++] = c[c_pos++];
			}
		}
		if (i > 0) {
			// Copy the remaining bytes
			memcpy(u+u_pos, c+c_pos, i);
			c_pos += i;
			u_pos += i;
		}
	}
	
	// Return unsuccessful or size
	return c_pos < c_len  ? 0 : u_pos;
}

// The main function for decompression, all the work is done here
ULONG bmzip_decompress(const BYTE *c, ULONG c_len, BYTE *u, ULONG u_len) {
	ULONG c_pos = 0, u_pos = 0, c_size, u_size, flags;

	// Go through every fragment
	while (c_pos < c_len-1 && u_pos < u_len) {
		// Read fragment header
		c_size = (c[c_pos] | (c[c_pos+1] << 8) & 0x0FFF)+1;
		flags = c[c_pos+1] >> 4 & 0x0F; // Currently flags is always 1011 (0xB), I have no idea what these bits mean
		c_pos += 2;

		if (c_pos + c_size > c_len) { // Fragment is longer than the available data
			//_tprintf(TEXT("%u, %u, %u, %1X\n"), c_pos, c_size, c_len, flags);
			SetLastError(ERROR_INVALID_DATA);
			return 0;
		}

		if (flags == 0xB) {
			// Compressed fragment
			u_size = DecompressFragment(c+c_pos, c_size, u+u_pos, u_len-u_pos);
		} else {
			//_tprintf(TEXT("Warning: flags is unexpected value: %1X\n"), flags);

			// Assume no compression, copy fragment directly
			u_size = c_size;
			if (u_pos + u_size >= u_len) { break; } // Fragment is longer than the available space
			memcpy(u+u_pos, c+c_pos, u_size);
		}
		if (u_size == 0) { break; }
		u_pos += u_size;
		c_pos += c_size;
	}

	// Return unsuccessful or size
	if (c_pos < c_len-1) {
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return 0;
	}
	return u_pos;
}

Bytes bmzip_decompress(const Bytes &c) {
	Bytes u = Bytes::alloc((*c)*2); // assume double is as big as uncompressed data is
	ULONG size = bmzip_decompress(~c, *c, ~u, *u);
	if (size == 0) { free(u); return Bytes::Null; }
	u.setCount(size);
	return u;
}
