:: The settings for compiling the programs

:: Set to ON to compile with debug symbols
@set DEBUGGING=OFF

@set WIMGAPI=C:\Program Files\Windows AIK\SDKs\WIMGAPI\%TARGET%
:: Only need this for ntdll.h, include it instead in the source directory since the WinDDK is huge
::@set WINDDK=C:\WinDDK\7600.16385.1\lib\wlh\%TARGET_FULL%

::@set EXTRA_INCLUDES=%WIMGAPI%;%WINDDK%
::@set EXTRA_LIBS=%WIMGAPI%;%WINDDK%
@set EXTRA_INCLUDES=%WIMGAPI%
@set EXTRA_LIBS=%WIMGAPI%

@set ZLIB=libpng\adler32.c libpng\compress.c libpng\crc32.c libpng\deflate.c libpng\trees.c libpng\zutil.c
@set LIBPNG=libpng\png.c libpng\pngerror.c libpng\pngget.c libpng\pngmem.c libpng\pngset.c libpng\pngwio.c libpng\pngwrite.c libpng\pngwtran.c libpng\pngwutil.c
@set LIBPNG=%LIBPNG% %ZLIB%

@set NATIVE=bmzip.cpp Bytes.cpp Files.cpp FileSecurity.cpp PEFile.cpp PEFileResources.cpp WIM.cpp Trace.cpp
@set MIXED=Bootmgr.cpp Bcd.cpp Bootres.cpp FileUpdater.cpp MessageTable.cpp Patch.cpp PDB.cpp PEFiles.cpp PngConverter.cpp UI-native.cpp Updater.cpp Utilities.cpp WinXXX.cpp Zip.cpp
@set PURE=Animation.cpp BootSkin.cpp MultipartFile.cpp Resources.cpp UI.cpp Winload.cpp Winresume.cpp WMI.cpp
