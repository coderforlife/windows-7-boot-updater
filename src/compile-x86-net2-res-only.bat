:: This compiles new resources and inserts them into the program without actually compiling the code.

@set VS90COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\

@set OUT=x86-net2
@set TARGET=x86
@set TARGETX=x86
@set TARGET_FULL=i386
@set TOOLCHAIN=vs90
@set COMPILE_RESOURCES=1
@set COMPILE_CORE=0
@set COMPILE_INSTALLER=0
@set COMPILE_GUI=0
@set COMPILE_CMD=0
@call Compile\all.bat

@pushd Win7BootUpdater.Installer
cl /c /clr /LN /FU..\%W7BU%.Installer.netmodule main.cpp
@popd

link /FIXED /OUT:%OUT%\installer.exe /SUBSYSTEM:WINDOWS,6.1 ^
%OUT%\installer.res /ASSEMBLYRESOURCE:%W7BU%.resources,Win7BootUpdater.resources,PRIVATE /ASSEMBLYRESOURCE:%W7BU%.Installer.resources,Win7BootUpdater.Installer.resources,PRIVATE ^
%OUT%\*.obj %W7BU%.Installer.netmodule

CorFlags /nologo %OUT%\installer.exe /32BIT+

@pushd Resources
copy /B /Y ..\%OUT%\installer.exe .
call gzip installer.exe
resgen Win7BootUpdater.GUI.resx ..\%W7BU%.GUI.resources
@popd


@pushd Win7BootUpdater.GUI
cl /c /clr /LN /FU..\%W7BU%.GUI.netmodule main.cpp
@popd

link /FIXED /OUT:%W7BU%.exe /SUBSYSTEM:WINDOWS,6.0 ^
%OUT%\gui.res /ASSEMBLYRESOURCE:%W7BU%.resources,Win7BootUpdater.resources,PRIVATE /ASSEMBLYRESOURCE:%W7BU%.GUI.resources,Win7BootUpdater.GUI.resources,PRIVATE ^
%OUT%\*.obj %W7BU%.GUI.netmodule

CorFlags /nologo %W7BU%.exe /32BIT+


@pushd Win7BootUpdater.CUI
cl /c /clr /LN /FU..\%W7BU%.CUI.netmodule main.cpp
@popd

link /FIXED /OUT:%W7BU%Cmd.exe /SUBSYSTEM:CONSOLE,6.0 ^
%OUT%\cmd.res /ASSEMBLYRESOURCE:%W7BU%.resources,Win7BootUpdater.resources,PRIVATE ^
%OUT%\*.obj %W7BU%.CUI.netmodule

CorFlags /nologo %W7BU%Cmd.exe /32BIT+

@set arg0=%0
@if [%arg0:~2,1%]==[:] PAUSE
