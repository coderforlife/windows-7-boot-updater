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

#include "WMI.h"

using namespace Win7BootUpdater;

using namespace System;
using namespace System::Management;

string WMI::GetValue(string clazz, string prop) {
	ManagementClass ^mc = gcnew ManagementClass(clazz);
	for each (ManagementObject ^mo in mc->GetInstances()) {
		object o = mo->Properties[prop]->Value;
		if (o) return o->ToString()->Trim();
	}
	return nullptr;
}

/*static ManagementObject ^FindMatch(string clazz, string prop, object value) {
	ManagementClass ^mc = gcnew ManagementClass(clazz);
	for each (ManagementObject ^mo in mc->GetInstances())
		if (value->Equals(mo[prop]))
			return mo;
	return nullptr;
}*/

object WMI::QueryFirst(string query, string value) {
	for each (ManagementObject ^mo in Query(query))
		return mo[value];
	return nullptr;
}

ManagementObject ^WMI::QueryFirst(string query) {
	for each (ManagementObject ^mo in Query(query))
		return mo;
	return nullptr;
}

ManagementObjectCollection ^WMI::Query(string query) {
	return (gcnew ManagementObjectSearcher(query))->Get();
}
