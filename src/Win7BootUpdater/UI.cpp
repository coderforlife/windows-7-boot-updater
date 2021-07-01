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

#include "UI.h"

#include "ErrorCodes.h"
#include "Resources.h"

using namespace Win7BootUpdater;

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Globalization;
using namespace System::IO;
using namespace System::Text;

#if __CLR_VER >= 30500000
#define IsNullOrWhiteSpace(s) String::IsNullOrWhiteSpace(s)
#else
inline static bool IsNullOrWhiteSpace(string s) {
	if (String::IsNullOrEmpty(s)) return true;
	for each (char c in s)
		if (!Char::IsWhiteSpace(c)) return false;
	return true;
}
#endif

inline static string ProcessMessage(string msg) { return msg->Trim()->Replace(L"\\n", L"\n"); }

void UI::Init() {
	if (locales != nullptr) return;
	
	array<string> ^names = Res::GetAvailableLanguages();
	locales = gcnew array<CultureInfo^>(names->Length);
	localeToNames = gcnew Dictionary<CultureInfo^, string>();

	messages = gcnew List<string>();
	fallbackMsgs = gcnew List<string>();
	englishMsgs = gcnew List<string>();

	for (int i = 0; i < names->Length; ++i) {
		string n = names[i];
		CultureInfo ^l = CultureInfo::GetCultureInfo(n);
		locales[i] = l;
		localeToNames[l] = n;
	}

	LoadLanguage(L"en-US", englishMsgs);
	Locale = nullptr; // causes it to set it as the system default
}

void UI::LoadLanguage(string name, List<string> ^list) {
	list->Clear();
	StreamReader ^s = gcnew StreamReader(Res::GetLanguageStream(name), Encoding::UTF8);
	string line;
	while ((line = s->ReadLine()) != nullptr)
		list->Add(IsNullOrWhiteSpace(line) ? nullptr : ProcessMessage(line));
	s->Close();
}

array<string> ^UI::GetTranslators() {
	Init();
	if (translators == nullptr) {
		translators = gcnew array<string>(locales->Length);
		for (int i = 0; i < locales->Length; ++i) {
			StreamReader ^s = gcnew StreamReader(Res::GetLanguageStream(localeToNames[locales[i]]), Encoding::UTF8);
			s->ReadLine();
			translators[i] = s->ReadLine();
			s->Close();
		}
	}
	return translators;
}

string UI::GetMessage(Msg id, ... array<object> ^opts) {
	Init();
	int i = (int)id;
	string msg;
	if (i < messages->Count && messages[i] != nullptr)
		msg = messages[i];
	else if (i < fallbackMsgs->Count && fallbackMsgs[i] != nullptr)
		msg = fallbackMsgs[i];
	else
		msg = englishMsgs[i];
	return (opts && opts->Length > 0) ? String::Format(msg, opts) : msg;
}

string UI::GetErrorMessage(uint e, string success) {
	string file = nullptr, res;
	switch (e & 0xF0) {
	case ERROR_BOOTRES_BASE:		file = L"bootres.dll";			break;
	case ERROR_WINLOAD_BASE:		file = L"winload.exe";			break;
	case ERROR_WINLOAD_MUI_BASE:	file = L"winload.exe.mui";		break;
	case ERROR_WINRESUME_BASE:		file = L"winresume.exe";		break;
	case ERROR_WINRESUME_MUI_BASE:	file = L"winresume.exe.mui";	break;
	case ERROR_BOOTMGR_BASE:		file = L"bootmgr";				break;
	}

	if (file != nullptr) {
		switch (e & 0xF) {
		case GEN_ERR_CREATE_BACKUP:		return GetMessage(Msg::FailedToCreateABackupOf, file);
		case GEN_ERR_PEFILE_LOAD:		return GetMessage(Msg::FileCouldNotBeReadAsPE, file);
		case GEN_ERR_PEFILE_INVALID_RES:
			res = L"MESSAGETABLE:1, HTML:";
			switch (e & 0xF0) {
			case ERROR_BOOTRES_BASE:	res = L"RCDATA:1";		break;
			case ERROR_WINLOAD_BASE:	case ERROR_WINLOAD_MUI_BASE:	res += L"OSLOADER.XSL";	break;
			case ERROR_WINRESUME_BASE:	case ERROR_WINRESUME_MUI_BASE:	res += L"RESUME.XSL";	break;
			case ERROR_BOOTMGR_BASE:	res += L"BOOTMGR.XSL";	break;
			}
			return GetMessage(Msg::FileDoesNotContainProperResources, file, res+L", VERSION:1");
		case GEN_ERR_PEFILE_INVALID_VER:return GetMessage(Msg::FileIsNotAccordingToItsVersionInfo, file);
		case GEN_ERR_PEFILE_MOD_RES:	return ((e & 0xF0) == ERROR_BOOTRES_BASE) ?
				GetMessage(Msg::FailedToUpdateTheWIMResource) :
				GetMessage(Msg::FailedToUpdateTheResources, file);
		case GEN_ERR_PEFILE_CLEAR_CERT:	return GetMessage(Msg::FailedToRemoveTheDigitalCertificate, file);
		case GEN_ERR_PEFILE_SAVE:		return GetMessage(Msg::FailedToSaveTheUpdated, file);
		case GEN_ERR_DEACTIVE_SEC:		return GetMessage(Msg::FailedToDeactiveFilesystemSecurity, file);
		case GEN_ERR_RESTORE_SEC:		return GetMessage(Msg::FailedToRestoreFilesystemSecurity, file);
		}
	}

	switch (e) {
	case ERROR_SUCCESS:				return success;

	case ERROR_BOOTRES_ANIM_SAVE:	return GetMessage(Msg::FailedToSaveTheAnimation);
	case ERROR_BOOTRES_WIM_CAPTURE:	return GetMessage(Msg::FailedToCaptureTheWIM);
	case ERROR_BOOTRES_WIM_READ:	return GetMessage(Msg::FailedToReadTheWIM);

	case ERROR_WINLOAD_IS_MUI:		case ERROR_WINRESUME_IS_MUI:
		return GetMessage(Msg::FileLooksLike, file+L".mui", file);
	case ERROR_WINLOAD_MUI_IS_NOT_MUI:	case ERROR_WINRESUME_MUI_IS_NOT_MUI:
		return GetMessage(Msg::FileLooksLike, file->Remove(file->Length-4), file);
	case ERROR_WINLOAD_COPYRIGHT:	case ERROR_WINRESUME_COPYRIGHT:
		return GetMessage(Msg::FailedToUpdateTheCopyright, file);
	case ERROR_WINLOAD_MSG_TBL:		case ERROR_WINRESUME_MSG_TBL:
	case ERROR_WINLOAD_MUI_MSG_TBL:	case ERROR_WINRESUME_MUI_MSG_TBL:
		return GetMessage(Msg::FailedToUpdateTheMessageTableText, file);
	case ERROR_WINLOAD_XSL:			case ERROR_WINRESUME_XSL:
		return GetMessage(Msg::FailedToUpdate, (e == ERROR_WINLOAD_XSL) ? L"osloader.xsl" : L"resume.xsl", file);
	case ERROR_WINLOAD_PROP:		case ERROR_WINRESUME_PROP:
		return GetMessage(Msg::FailedToUpdateTheTextAndBackground, file);
	case ERROR_WINLOAD_HACK:		case ERROR_WINRESUME_HACK:
		return GetMessage(Msg::FailedToDisableBootresWinloadSecurity, file);
	case ERROR_WINLOAD_FONT:		case ERROR_WINRESUME_FONT:
		return GetMessage(Msg::FailedToUpdateTheBootFont, file);

	case ERROR_BOOTMGR_HACK:		return GetMessage(Msg::FailedToDisableBootmgrSecurity);
	case ERROR_BOOTMGR_COMPRESS:	return GetMessage(Msg::FailedToRecompessBootmgr);

	case ERROR_GET_TEMP_FILE:		return GetMessage(Msg::FailedToCreateATemporaryFile);
	case ERROR_GET_TEMP_DIR:		return GetMessage(Msg::FailedToCreateATemporaryDirectory);
	case ERROR_DEL_TEMP_FILE:		return GetMessage(Msg::FailedToDeleteATemporaryFile);

	case ERROR_THROWN:				return GetMessage(Msg::ThereWasAnUncaughtExceptionWhileUpdatingTheFiles);
	default:						return GetMessage(Msg::UnknownError, e);
	}
}


CultureInfo ^UI::Locale::get() { Init(); return locale; }
void UI::Locale::set(CultureInfo ^value) {
	Init();
	if (value == nullptr)
		value = CultureInfo::InstalledUICulture;
	string twoLetter = value->TwoLetterISOLanguageName, full = value->Name;

	// search for best match
	int best = 0;
	for (int i = 0; i < locales->Length; ++i) {
		if (locales[i]->Name == full) { best = i; break; }
		if (locales[i]->TwoLetterISOLanguageName == twoLetter) { best = i; }
	}

	// search for fallback best
	int fbBest = 0;
	for (int i = 0; i < locales->Length; ++i) {
		if (i != best && locales[i]->TwoLetterISOLanguageName == twoLetter) {
			fbBest = i;
			if (locales[i]->Name->Length == 2) { break; }
		}
	}

	// Load the languages
	if (fbBest > 0)
		LoadLanguage(localeToNames[locales[fbBest]], fallbackMsgs);
	else
		fallbackMsgs->Clear();
	locale = locales[best];
	LoadLanguage(localeToNames[locale], messages);
}
array<CultureInfo^> ^UI::AvailableLocales() { Init(); return locales; }
string UI::LocaleDisplayName(CultureInfo ^l) {
	string n = l->NativeName;
	int b = n->IndexOf('(');
	return b > 0 ? n->Remove(b)->Trim() : n;
}

void UI::InitProgress(int x)			{ text = nullptr; max = x; cur = 0; OnChange(); }
int  UI::ProgressMax::get()				{ return max; }
void UI::ProgressMax::set(int x)		{ max = x; OnChange(); }
int  UI::ProgressCurrent::get()			{ return cur; }
void UI::ProgressCurrent::set(int x)	{ cur = x; OnChange(); }
string UI::ProgressText::get()			{ return text; }
void UI::ProgressText::set(string x)	{ text = x; OnChange(); }
int  UI::Inc()							{ ++cur; OnChange(); return cur; }
int  UI::Inc(int x)						{ cur += x; OnChange(); return cur; }
int  UI::Inc(string x)					{ text = x; ++cur; OnChange(); return cur; }
void UI::OnChange() {
	if (max <= 0) max = 1;
	if (cur > max) cur = max;
	if (cur < 0) cur = 0;
	//if (ProgressChanged)
		ProgressChanged(text, cur, max);
}

void UI::ShowError(string text, string caption) { if (ErrorMessenger) ErrorMessenger(text, caption); }
void UI::ShowError(Msg text, string opt, Msg caption) { if (ErrorMessenger) ErrorMessenger(GetMessage(text, opt), GetMessage(caption)); }
void UI::ShowError(string pre, string post, uint err, string caption) {
	string msg = L"";
	if (!String::IsNullOrEmpty(pre)) msg += pre + L"\n";
	msg += GetErrorMessage(err, GetMessage(Msg::NoError));
	if (!String::IsNullOrEmpty(post)) msg += L"\n" + post;
	ShowError(msg, caption);
}
//void UI::ShowMessage(string text, string caption) { if (Messenger) Messenger(text, caption); }
//void UI::ShowMessage(string text, Msg caption) { if (Messenger) Messenger(text, GetMessage(caption)); }
//void UI::ShowMessage(Msg text, Msg caption) { if (Messenger) Messenger(GetMessage(text), GetMessage(caption)); }
//bool UI::AskYesNo(string text, string caption) { return YesNoMessenger && YesNoMessenger(text, caption); }
