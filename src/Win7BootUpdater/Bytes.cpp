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

#include "Bytes.h"

#ifdef __cplusplus_cli
#pragma unmanaged
#endif

const Bytes Bytes::Null;

Bytes Bytes::alloc(size_t count, bool zero) {
	Bytes d((unsigned char*)malloc(count), count);
	if (zero) { memset(d.data, 0, count); }
	return d;
}
Bytes Bytes::copy(const unsigned char* x, size_t count) { Bytes b = alloc(count); memcpy(b, x, count); return b; }
Bytes Bytes::from(const wchar_t *x) { return Bytes((unsigned char*)x, (wcslen(x)+1)*sizeof(wchar_t)); }

Bytes::Bytes() : count(0), data(NULL) {}
Bytes::Bytes(const Bytes& x) : count(x.count), data(x.data) {}
Bytes::Bytes(unsigned char* x, size_t count) : count(count), data(x) {}
Bytes::~Bytes() {}

Bytes& Bytes::operator =(const Bytes& x) { if (this != &x) { count = x.count; data = x.data; } return *this; }
Bytes& Bytes::operator =(unsigned char* x) { data = x; if (x == NULL) { count = 0; } return *this; } // size must be set on its own

size_t Bytes::setCount(size_t count) { return (this->count = count); }
Bytes Bytes::find(const Bytes& target) const { return find(target.data, target.count); }
Bytes Bytes::find(const unsigned char *target, size_t t_count) const {
	if (t_count == 0 || count == 0) return Null;

	Bytes data(this->data, count), d(this->data, count);
	do {
		// Find the first byte of target in data
		d = (unsigned char*)memchr(data, (int)target[0], *d);

		// Could not find start
		if (d == NULL) { break; }

		// Move the next search position one ahead so that we skip the current byte
		d.count = count-(d.data-this->data);
		data = d+1;

		// Too short
		if (d.count < t_count) { return Null; }

		// Compare the current data to target
	} while (memcmp(d, target, t_count) != 0);
	return d;
}

static bool equal(const unsigned char *a, const unsigned char *b, size_t n, unsigned char wildcard) {
	for (size_t i = 0; i < n; ++i) if (a[i] != b[i] && b[i] != wildcard) return false;
	return true;
}
Bytes Bytes::find(const Bytes& target, byte wildcard) const { return find(target.data, target.count, wildcard); }
Bytes Bytes::find(const unsigned char *target, size_t t_count, byte wildcard) const {
	if (t_count == 0 || count == 0) return Null;
	if (target[0] == wildcard) return find(target, t_count);
	
	Bytes data(this->data, count), d(this->data, count);
	do {
		// Find the first byte of target in data
		d = (unsigned char*)memchr(data, (int)target[0], *d);

		// Could not find start
		if (d == NULL) { break; }

		// Move the next search position one ahead so that we skip the current byte
		d.count = count-(d.data-this->data);
		data = d+1;
		
		// Too short
		if (d.count < t_count) { return Null; }

		// Compare the current data to target (using the wildcard)
	} while (!equal(d, target, t_count, wildcard));
	return d;
}
void Bytes::set(const Bytes &target) {
	memcpy(this->data, target.data, (count < target.count) ? count : target.count);
}
Bytes Bytes::duplicate() const {
	Bytes x = alloc(count);
	x.set(*this);
	return x;
}

//#define COMPARE(a, b) (a<((size_t)b) ? -1 : (a==((size_t)b) ? 0 : 1))
//bool Bytes::Equals(const Bytes& x) const { return count == x.count && memcmp(data, x.data, min(count, x.count)) == 0; }
//int Bytes::Compare(const Bytes& x) const {
//	int c = memcmp(data, x.data, min(count, x.count));
//	return (c == 0) ? COMPARE(count, x.count) : c;
//}
//bool Bytes::Equals(const unsigned char* x) const { return memcmp(data, x, count) == 0; }
//int Bytes::Compare(const unsigned char* x) const { return memcmp(data, x, count); }

bool Bytes::operator ==(const Bytes& x) const { return this->data == x.data && this->count == x.count; }
/*bool Bytes::operator !=(const Bytes& x) const { return this->data != x.data || this->count != x.count; }
bool Bytes::operator >(const Bytes& x) const { }
bool Bytes::operator <(const Bytes& x) const { }
bool Bytes::operator >=(const Bytes& x) const { }
bool Bytes::operator <=(const Bytes& x) const { }*/

bool Bytes::operator ==(const void* x) const { return this->data == x; }
/*bool Bytes::operator !=(const void* x) const { return this->data != x; }
bool Bytes::operator >(const void* x) const { return this->data > x; }
bool Bytes::operator <(const void* x) const { return this->data < x; }
bool Bytes::operator >=(const void* x) const { return this->data >= x; }
bool Bytes::operator <=(const void* x) const { return this->data <= x; }*/

bool Bytes::operator ==(const size_t x) const { return (size_t)this->data == x; }
/*bool Bytes::operator !=(const size_t x) const { return (size_t)this->data != x; }
bool Bytes::operator >(const size_t x) const { return (size_t)this->data > x; }
bool Bytes::operator <(const size_t x) const { return (size_t)this->data < x; }
bool Bytes::operator >=(const size_t x) const { return (size_t)this->data >= x; }
bool Bytes::operator <=(const size_t x) const { return (size_t)this->data <= x; }*/

bool Bytes::operator ==(const int x) const { return (int)this->data == x; }
/*bool Bytes::operator !=(const int x) const { return (int)this->data != x; }
bool Bytes::operator >(const int x) const { return (int)this->data > x; }
bool Bytes::operator <(const int x) const { return (int)this->data < x; }
bool Bytes::operator >=(const int x) const { return (int)this->data >= x; }
bool Bytes::operator <=(const int x) const { return (int)this->data <= x; }*/

bool Bytes::operator !() const { return data == NULL; }

size_t Bytes::operator -(const void* x) const { return this->data - (unsigned char*)x; }
Bytes Bytes::operator +(const size_t& x) const { return Bytes(this->data + x, this->count - x); }
Bytes Bytes::operator -(const size_t& x) const { return Bytes(this->data - x, this->count + x); }
Bytes Bytes::operator +(const int& x) const { return Bytes(this->data + x, this->count - x); }
Bytes Bytes::operator -(const int& x) const { return Bytes(this->data - x, this->count + x); }
/*
Bytes& Bytes::operator ++() {}		//prefix
Bytes Bytes::operator ++(int) {}	//suffix
Bytes& Bytes::operator --() {}		//prefix
Bytes Bytes::operator --(int) {}	//suffix
*/
unsigned char* Bytes::operator ~() const { return data; }

size_t Bytes::operator *() const { return count; }
size_t *Bytes::operator &() { return &count; }

const unsigned char *Bytes::operator ()(const size_t x) const { return data+x; }
unsigned char *Bytes::operator ()(size_t x) { return data+x; };

Bytes::operator bool() const { return data != NULL; }
Bytes::operator unsigned char*() const { return data; }
Bytes::operator void*() const { return data; }
