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

// This is a stub so that the C# code can properly merge with the C++/CLI code

using namespace System;

[STAThread]
int main() {
	array<String^> ^args_orig = Environment::GetCommandLineArgs();
	int l = args_orig->Length - 1;
	array<String^> ^args = gcnew array<String^>(l);
	if (l > 0) Array::Copy(args_orig, 1, args, 0, l);
	return Win7BootUpdater::CUI::Program::Main(args);
}
