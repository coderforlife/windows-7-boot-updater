:: Compile the command line program

@pushd Win7BootUpdater.CUI

csc %CSC% /addmodule:..\%OUT%\AllPublic.obj /out:..\%W7BU%.CUI.netmodule /recurse:*.cs
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

cl /c /clr /LN /FU..\%W7BU%.CUI.netmodule main.cpp
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

@popd

link /FIXED /OUT:%W7BU%Cmd.exe /SUBSYSTEM:CONSOLE,6.0 ^
%OUT%\cmd.res /ASSEMBLYRESOURCE:%W7BU%.resources,Win7BootUpdater.resources,PRIVATE ^
%OUT%\*.obj %W7BU%.CUI.netmodule
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

@if NOT "%TARGET%"=="x86" GOTO end
CorFlags /nologo %W7BU%Cmd.exe /32BIT+
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

:end