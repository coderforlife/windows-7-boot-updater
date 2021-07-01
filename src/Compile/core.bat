:: Compile the core library

@pushd Win7BootUpdater

@set CL_NATIVE=/c /FI"stdafx-native.h"
@set CL_MIXED=/c /clr /LN /FI"stdafx-mixed.h"
@set CL_PURE=/c /clr:safe /LN /GL /FI"stdafx-pure.h"

@set PURE=%PURE% AllPublic.cpp

cl /c /TC /wd4100 /wd4127 /wd4131 /wd4244 %LIBPNG%
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

cl %CL_NATIVE% %NATIVE%
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

cl %CL_MIXED% %MIXED%
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

cl %CL_PURE% %PURE%
@IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

@popd
