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
	ref class MessageTable sealed {
		ref class Block sealed {
		private:
			uint lowId;
			array<string> ^entries;
		public:
			Block(uint lowId, uint highId);
			//~Block();
			uint Count();
			virtual property string default[uint] {
				string get (uint i);
				void set (uint i, string entry);
			}

			uint LowId();
			uint HighId();
			bool ContainsId(uint id);
			void SetLowId(uint id);
			virtual property string Entry[uint] {
				string get (uint id);
				void set (uint id, string entry);
			}
		};

		array<Block^> ^blocks;
		Block ^GetBlock(uint id);
	public:
		MessageTable(unsigned char *x, size_t l);
		//~MessageTable();
		unsigned char *compile(size_t *l);
		
		bool ContainsId(uint id);
		virtual property string default[uint] {
			string get (uint id);
			void set (uint id, string s);
		}
	};
}