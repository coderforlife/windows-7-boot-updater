:: Compile the installer program

@pushd Win7BootUpdater.Installer

csc %CSC% /addmodule:..\%OUT%\AllPublic.obj /out:..\%W7BU%.Installer.netmodule /recurse:*.cs
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

cl /c /clr /LN /FU..\%W7BU%.Installer.netmodule main.cpp
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

@popd

link /FIXED /OUT:%OUT%\installer.exe /SUBSYSTEM:WINDOWS,6.1 ^
%OUT%\installer.res /ASSEMBLYRESOURCE:%W7BU%.resources,Win7BootUpdater.resources,PRIVATE /ASSEMBLYRESOURCE:%W7BU%.Installer.resources,Win7BootUpdater.Installer.resources,PRIVATE ^
%OUT%\*.obj %W7BU%.Installer.netmodule
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

@if NOT "%TARGET%"=="x86" GOTO end
CorFlags /nologo %OUT%\installer.exe /32BIT+
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

:end

@echo.
@echo *************** Installer Resources *************

@pushd Resources

copy /B /Y ..\%OUT%\installer.exe .

call gzip installer.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

resgen Win7BootUpdater.GUI.resx ..\%W7BU%.GUI.resources
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

@popd
