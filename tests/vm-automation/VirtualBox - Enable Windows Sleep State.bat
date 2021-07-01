@echo off
cd C:\Program Files\Oracle\VirtualBox

set config=VBoxInternal/Devices/acpi/0/Config

VBoxManage setextradata "Windows 7 x86" %config%/PowerS1Enabled 1
VBoxManage setextradata "Windows 7 x86" %config%/PowerS4Enabled 1
VBoxManage setextradata "Windows 7 x64" %config%/PowerS1Enabled 1
VBoxManage setextradata "Windows 7 x64" %config%/PowerS4Enabled 1

pause