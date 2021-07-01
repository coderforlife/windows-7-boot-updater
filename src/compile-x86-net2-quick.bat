:: Compile command line program targeting .NET 2.0 and x86, assumes resources are compiled and up-to-date

@set VS90COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\

@set OUT=x86-net2
@set TARGET=x86
@set TARGETX=x86
@set TARGET_FULL=i386
@set TOOLCHAIN=vs90
@set COMPILE_RESOURCES=0
@set COMPILE_CORE=1
@set COMPILE_INSTALLER=0
@set COMPILE_GUI=0
@set COMPILE_CMD=1
@call Compile\all.bat

@set arg0=%0
@if [%arg0:~2,1%]==[:] PAUSE
