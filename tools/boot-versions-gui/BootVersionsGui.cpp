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

#include "stdafx.h"
#include "BootVersionsGui.h"

#include "boot-versions.h"

using namespace bootversionsgui;

using namespace System;
using namespace System::Drawing;
using namespace System::IO;
using namespace System::Windows::Forms;

BootVersionsGui::BootVersionsGui() {
	this->SuspendLayout();
	this->ClientSize = Drawing::Size(384, 362);
	this->ShowIcon = false;
	this->Text = L"Boot Versions";

	this->copy = gcnew Button();
	this->copy->Text = L"Copy Information";
	this->copy->Size = Drawing::Size(192, 23);
	this->copy->Click += gcnew EventHandler(this, &BootVersionsGui::CopyInfo);
	this->Controls->Add(this->copy);

	this->save = gcnew Button();
	this->save->Text = L"Save Information";
	this->save->Size = Drawing::Size(192, 23);
	this->save->Location = Point(192, 0);
	this->save->Click += gcnew EventHandler(this, &BootVersionsGui::SaveInfo);
	this->Controls->Add(this->save);

	TextBox ^text = gcnew TextBox();
	text->Font = gcnew Drawing::Font(FontFamily::GenericMonospace, 8);
	text->Anchor = AnchorStyles::Top | AnchorStyles::Bottom | AnchorStyles::Left | AnchorStyles::Right;
	text->Location = Drawing::Point(0, 23);
	text->Multiline = true;
	text->ReadOnly = true;
	text->ScrollBars = System::Windows::Forms::ScrollBars::Both;
	text->Size = Drawing::Size(384, 362-23);
	text->WordWrap = false;
	text->Text = this->text = GetBootVersionInfo();
	this->Controls->Add(text);

	this->ResumeLayout();
}

void BootVersionsGui::OnResize(EventArgs ^e) {
	if (this->copy && this->save) {
		Drawing::Size sz = this->ClientSize;
		this->copy->Width = sz.Width / 2;
		this->save->Width = sz.Width - this->copy->Width;
		this->save->Left = this->copy->Width;
	}
	Form::OnResize(e);
}

void BootVersionsGui::CopyInfo(Object^, EventArgs^) { Clipboard::SetText(this->text); }

void BootVersionsGui::SaveInfo(Object^, EventArgs^) {
	SaveFileDialog^ d = gcnew SaveFileDialog();
	d->Filter = L"Text Files|*.txt|All Files|*.*";
	d->RestoreDirectory = true;
	if (d->ShowDialog() == ::DialogResult::OK) {
		String ^file = d->FileName;
		if (!String::IsNullOrEmpty(file)) {
			File::WriteAllText(file, this->text);
		}
	}
}