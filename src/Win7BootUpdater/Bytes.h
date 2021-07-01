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

#pragma once

class Bytes {
protected:
	size_t count;
	unsigned char* data;

public:
	// NULL for a Bytes object (using NULL is very inefficient, but works)
	static const Bytes Null;

	static Bytes alloc(size_t count, bool zero = false);
	static Bytes copy(const unsigned char* x, size_t count);
	static Bytes from(const wchar_t *x);
	
	Bytes();
	Bytes(const Bytes& x);
	Bytes(unsigned char* x, size_t count);
	~Bytes();

	Bytes& operator =(const Bytes& x);
	Bytes& operator =(unsigned char* x); // count must be set on its own

	size_t setCount(size_t count);
	Bytes find(const Bytes& target) const;
	Bytes find(const unsigned char *target, size_t count) const;
	Bytes find(const Bytes& target, byte wildcard) const;
	Bytes find(const unsigned char *target, size_t count, byte wildcard) const;
	void set(const Bytes &target);
	Bytes duplicate() const;

	//bool Equals(const Bytes& x) const;
	//int Compare(const Bytes& x) const;
	//bool Equals(const unsigned char* x) const;
	//int Compare(const unsigned char* x) const;

	bool operator ==(const Bytes& x) const;
	/*bool operator !=(const Bytes& x) const;
	bool operator >(const Bytes& x) const;*/
	//NOT DEFINED: bool operator <(const Bytes& x) const;
	//NOT DEFINED: bool operator >=(const Bytes& x) const;
	//NOT DEFINED: bool operator <=(const Bytes& x) const;

	bool operator ==(const void* x) const;
	/*bool operator !=(const void* x) const;
	bool operator >(const void* x) const;
	bool operator <(const void* x) const;
	bool operator >=(const void* x) const;
	bool operator <=(const void* x) const;*/

	bool operator ==(const size_t x) const;
	/*bool operator !=(const size_t x) const;
	bool operator >(const size_t x) const;
	bool operator <(const size_t x) const;
	bool operator >=(const size_t x) const;
	bool operator <=(const size_t x) const;*/

	bool operator ==(const int x) const;
	/*bool operator !=(const int x) const;
	bool operator >(const int x) const;
	bool operator <(const int x) const;
	bool operator >=(const int x) const;
	bool operator <=(const int x) const;*/

	bool operator !() const;
	//NOT DEFINED: bool operator &&(const X& b) const;
	//NOT DEFINED: bool operator ||(const X& b) const;

	Bytes operator +(const size_t& x) const;
	Bytes operator -(const size_t& x) const;
	Bytes operator +(const int& x) const;
	Bytes operator -(const int& x) const;
	size_t operator -(const void* x) const;
	//NOT DEFINED: Bytes operator +() const;
	//NOT DEFINED: Bytes operator -() const;
	//NOT DEFINED: Bytes operator *(const X& b) const;
	//NOT DEFINED: Bytes operator /(const X& b) const;
	//NOT DEFINED: Bytes operator %(const X& b) const;
	//NOT DEFINED: Bytes& operator ++();	//prefix
	//NOT DEFINED: Bytes operator ++(int);	//suffix
	//NOT DEFINED: Bytes& operator --();	//prefix
	//NOT DEFINED: Bytes operator --(int);	//suffix

	unsigned char* operator ~() const; // explicit cast to unsigned char*
	//NOT DEFINED: Bytes operator &(const X& b) const;
	//NOT DEFINED: Bytes operator |(const X& b) const;
	//NOT DEFINED: Bytes operator ^(const X& b) const;
	//NOT DEFINED: Bytes operator <<(const X& b) const;
	//NOT DEFINED: Bytes operator >>(const X& b) const;

	//NOT DEFINED: Bytes& operator +=(const size_t& x);
	//NOT DEFINED: Bytes& operator -=(const size_t& x);
	//NOT DEFINED: Bytes& operator *=(const X& b);
	//NOT DEFINED: Bytes& operator /=(const X& b);
	//NOT DEFINED: Bytes& operator %=(const X& b);
	//NOT DEFINED: Bytes& operator &=(const X& b);
	//NOT DEFINED: Bytes& operator |=(const X& b);
	//NOT DEFINED: Bytes& operator ^=(const X& b);
	//NOT DEFINED: Bytes& operator <<=(const X& b);
	//NOT DEFINED: Bytes& operator >>=(const X& b);

	//NOT DEFINED: const unsigned char operator [](const size_t x) const; // the cast to unsigned char* takes care of this
	//NOT DEFINED: unsigned char operator [](size_t x);
	size_t operator *() const; // get the count
	size_t *operator &(); // get a pointer to the count
	//NOT DEFINED: T2* operator ->();
	//NOT DEFINED: R operator->*(T2);

	const unsigned char *operator ()(const size_t x) const; // pointer to a particular offset
	unsigned char *operator ()(size_t x); // pointer to a particular offset
	//NOT DEFINED: T2& operator ,(T2& b) const;

	operator bool() const;
	operator unsigned char*() const;
	operator void*() const;
};
