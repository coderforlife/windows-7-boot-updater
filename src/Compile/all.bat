:: Compile all resources and programs
:: Must have many things set before using this file:
::   OUT - the output directory
::   TARGET - the compilation target (e.g. x86), given to vcvarsall.bat
::   TARGETX - the compilation target (e.g. x86), used in VC tool command lines
::   TARGET_FULL - the full name of the compilation target (e.g. i386)
::   TOOLCHAIN - toolchain version (vs90 or vs100)
::   VS90COMNTOOLS or VS100COMNTOOLS - location of the toolchain
::   COMPILE_... - if set to 1, that option is compiled
::     where ... is: RESOURCES, CORE, INSTALLER, GUI, or CMD

@call Compile\settings.bat

@set ORIG_PATH="%PATH%"
@set INCLUDE=""
@set LIB=""
@set LIBPATH=""

@if "%TOOLCHAIN%"=="vs90" (
  @call "%VS90COMNTOOLS%\..\..\VC\vcvarsall.bat" %TARGET%
  @set CL_TC=
) else (
  @call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" %TARGET%
  @set CL_TC=/GL
)

@echo on


@if "%DEBUGGING%"=="ON" (
  @set OUT=%OUT%-debug
  @set DEBUG=-debug
  @set RC_DBG=/d _DEBUG
  @set CL_DBG=/Zi /MDd /D _DEBUG /Fd..\%OUT%-debug\
  @set LNK_DBG=/DEBUG 
  @set CSC_DBG=/debug /d:DEBUG
) else (
  @set DEBUG=
  @set RC_DBG=/d NDEBUG
  @set CL_DBG=/MD /D NDEBUG
  @set LNK_DBG=/OPT:REF /OPT:ICF
  @set CSC_DBG=/o+
)
:: /DEFAULTLIB:msvcmrtd.lib
:: /DEFAULTLIB:msvcmrt.lib

@mkdir %OUT% >nul 2>&1

@set W7BU=%OUT%\Win7BootUpdater

@set ORIG_PROMPT=%PROMPT%
@set PROMPT=$G

set RC=%RC_DBG% /d GUI /d %TARGETX% /d %TOOLCHAIN%
set CL=/nologo %CL_DBG% %CL_TC% /W4 /wd4201 /wd4480 /O2 /GS /EHa /MP /D _UNICODE /D UNICODE /D INTEGRATED /Fo..\%OUT%\
set INCLUDE=%EXTRA_INCLUDES%;%INCLUDE%
set LINK=/nologo %LNK_DBG% /LTCG /CLRIMAGETYPE:IJW /MANIFEST:NO /MACHINE:%TARGETX%
set LIB=%EXTRA_LIBS%;%LIB%
set CSC=/nologo %CSC_DBG% /w:4 /d:INTEGRATED /target:module
::/platform:%TARGETX%

@IF "%COMPILE_RESOURCES%" NEQ "1" GOTO :AFTER_RESOURCES
@echo.
@echo *************** Resources ***********************
@call Compile\resources.bat
@IF %ERRORLEVEL% NEQ 0 GOTO :END

:AFTER_RESOURCES
@IF "%COMPILE_CORE%" NEQ "1" GOTO :AFTER_CORE
@echo.
@echo *************** Core ****************************
@call Compile\core.bat
@IF %ERRORLEVEL% NEQ 0 GOTO :END

:AFTER_CORE
@IF "%COMPILE_INSTALLER%" NEQ "1" GOTO :AFTER_INSTALLER
@echo.
@echo *************** Installer ***********************
@call Compile\installer.bat
@IF %ERRORLEVEL% NEQ 0 GOTO :END

:AFTER_INSTALLER
@IF "%COMPILE_GUI%" NEQ "1" GOTO :AFTER_GUI
@echo.
@echo *************** GUI *****************************
@call Compile\gui.bat
@IF %ERRORLEVEL% NEQ 0 GOTO :END

:AFTER_GUI
@IF "%COMPILE_CMD%" NEQ "1" GOTO :AFTER_CMD
@echo.
@echo *************** CMD *****************************
@call Compile\cmd.bat
@IF %ERRORLEVEL% NEQ 0 GOTO :END

:AFTER_CMD
@echo.
@echo Compiled Successfully!

::@echo.
::@echo *************** Cleanup *************************
::@call Compile\cleanup.bat
::@IF %ERRORLEVEL% NEQ 0 GOTO :END

:END
@echo.
@set PROMPT=%ORIG_PROMPT%
@set PATH="%ORIG_PATH%"
