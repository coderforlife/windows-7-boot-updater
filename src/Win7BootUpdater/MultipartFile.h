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

namespace Win7BootUpdater {
	ref class MultipartFile {
		static System::Random ^rand = gcnew System::Random();
		static string RandomBoundary();

		static ref class MultipartPart {
		public:
			MultipartPart(string id, string type, array<byte> ^data);
			string Id, Type;
			array<byte> ^Data;
		};

		literal string default_multipart_type = L"related";

		string multipart_type, boundary;
		System::Collections::Generic::List<MultipartPart^> ^parts;
		System::Collections::Generic::Dictionary<string, int> ^id_lookup;
		System::Collections::Generic::Dictionary<string, System::Collections::Generic::List<int>^> ^type_lookup;

		void BuildLookupTable();

	public:
		MultipartFile(System::IO::Stream ^s);
		MultipartFile();

		void Load(System::IO::Stream ^s);
		void Save(System::IO::Stream ^s);

		property string MultipartType { string get(); void set(string); }
		property string Boundary { string get(); void set(string); }

		property int Count { int get(); }
		
		bool HasId(string id);
		bool HasType(string type);

		property string Id[int] { string get(int); void set(int, string); }
		property string Type[int] { string get(int); void set(int, string); }
		property array<byte> ^Data[int] { array<byte> ^get(int); void set(int, array<byte>^); }

		property string Id[string] { /*string get(string);*/ void set(string, string); }
		property string Type[string] { string get(string); void set(string, string); }
		property array<byte> ^Data[string] { array<byte> ^get(string); void set(string, array<byte>^); }

		string FirstIdOfType(string type);

		void Add(string id, string type, array<byte> ^data);
		void Add(string type, array<byte> ^data);
		void Remove(int i);
		void Remove(string id);
	};
}
