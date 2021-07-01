:: Prepare and compile all of the resources

@pushd Resources

xml-compact bs7.xsd bs7.bin >NUL
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

@set MANIFEST=%TARGETX%-%TOOLCHAIN%%DEBUG%.manifest
xml-compact %MANIFEST% %MANIFEST%.bin >NUL
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

call gzip bs7.bin
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

::call gzip messages.txt
::@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

@pushd messages
@call zip-all
@popd
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

call gzip eula.txt
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

patch-compiler bootmgr.xml
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

patch-compiler winload.xml
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

patch-compiler winresume.xml
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

rc %RC% /d GUI /fo ..\%OUT%\gui.res app.rc
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

rc %RC% /d CMD /fo ..\%OUT%\cmd.res app.rc
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

resgen Win7BootUpdater.resx ..\%W7BU%.resources
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

@IF "%COMPILE_INSTALLER%" NEQ "1" GOTO :NO_INSTALLER

rc %RC% /d Installer /fo ..\%OUT%\installer.res installer.rc
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

resgen Win7BootUpdater.Installer.resx ..\%W7BU%.Installer.resources
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

@GOTO :DONE

:NO_INSTALLER
resgen Win7BootUpdater.GUI.resx ..\%W7BU%.GUI.resources
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

:DONE
@popd
