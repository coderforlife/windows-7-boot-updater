:: Compile the GUI program

@pushd Win7BootUpdater.GUI

csc %CSC% /addmodule:..\%OUT%\AllPublic.obj /out:..\%W7BU%.GUI.netmodule /recurse:*.cs
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

cl /c /clr /LN /FU..\%W7BU%.GUI.netmodule main.cpp
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

@popd

link /FIXED /OUT:%W7BU%.exe /SUBSYSTEM:WINDOWS,6.0 ^
%OUT%\gui.res /ASSEMBLYRESOURCE:%W7BU%.resources,Win7BootUpdater.resources,PRIVATE /ASSEMBLYRESOURCE:%W7BU%.GUI.resources,Win7BootUpdater.GUI.resources,PRIVATE ^
%OUT%\*.obj %W7BU%.GUI.netmodule
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

@if NOT "%TARGET%"=="x86" GOTO end
CorFlags /nologo %W7BU%.exe /32BIT+
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

:end