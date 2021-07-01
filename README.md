Windows 7 Boot Updater
======================

This is a program that makes it easy to update the Windows 7 boot animation and text.

The concepts used by this program were originally developed in a thread at [Seven Forums](https://www.sevenforums.com/customization/106861-how-change-boot-animation-windows-7-a.html) by Joakim, marcusj0015, and myself (thaimin).

This source code and resources for the program are released under the GPLv3. This program may be used as-is for commercial purposes and any files modified with it can also be used. However, under the GPLv3, any changes made to the source code or the resources (including the patch descriptions) must be made publicly available if the program or its modified files are distributed in any manner.

Features:

* Modifies the boot animation, colors, text, and background
* Modifies the resume from hibernation screen as well
* Will create the animation from 105 BMP, PNG, GIF, or TIFF images or use a single (non-animated) image
* Does not require any other applications (like ImageX)
* Designed to be idiot-proof by having numerous checks
* Automatically backs-up the modified files
* Does not require test-signing or no-integrity-checks to be enabled
* Works for all versions of Windows 7: any language, any edition, 32-bit or 64-bit, RTM or SP1
* Translated to English, German, Spanish, Russian, Italian, French, Dutch, Hungarian, Hebrew, and Vietnamese
* Works even if your setup has the hidden "System Reserved" partition
* Can be run as a GUI, command-line program, or installer/uninstaller

This program modifies several system files, and while it has been successfully by numerous people, there are circumstances in which it doesn't work or it causes the system to go into an infinite loop of boot repair. See the [troubleshooting](https://www.coderforlife.com/projects/win7boot/#Troubleshooting) guide for common issues, solutions, and how to repair your system if the worst happens.

Source Code
-----------

The source code is broken into 4 separate projects:

* Win7BootUpdater - this is the core of the program, written in C++/CLR. The interface is primarily exposes through the methods of the statis class in Updater.h. The list of all public interfaces are given in AllPublic.cpp. Can be compiled as a static library or a DLL.
* Win7BootUpdater.CUI - the command line interface program. Written in C#.
* Win7BootUpdater.GUI - the GUI program. Written in C#.
* Win7BootUpdater.Installer - the installer-based version of the program. Written in C#.

The src/Resources folder contains all of the resources used by the programs. It also includes a few tools required for compiling the resources:

* patch-compiler.exe - compiles the XML files describing the patching of system files into binary files. Source for this program is in tools/patch-compiler. This program uses the Ionic.Zlib.dll library to perform compression. That library is not needed for the final program.
* xml-compact.exe - compacts XML files to reduce their size. Source available at [github.com/coderforlife/c4l-utils/tree/master/xml-compact](https://github.com/coderforlife/c4l-utils/tree/master/xml-compact).
* gzip.bat and zip-all.bat - use 7-zip (must be installed on the system) to gzip files or add all txt files to a zip file

Compilation
-----------

It is quite tricky to compile programs that are a mix of C, C++, C++/CLR, and C#. The batch files in the src folder are there to assist in compilation of the program. There are numerous prerequites for compiling though:

* [7-zip](https://www.7-zip.org/download.html) (used to compress various resources)
* [MS Visual Studio 2008 SP1 C++ Express](https://download.microsoft.com/download/E/8/E/E8EEB394-7F42-4963-A2D8-29559B738298/VS2008ExpressWithSP1ENUX1504728.iso) (for various tools)
* [Windows 7 SDK and .NET 4 for x86](https://www.microsoft.com/en-us/Download/8442)
* [Windows AIK for Windows 7 (v3.0)](https://www.microsoft.com/en-us/download/5753)

Testing
-------

The tests folder contains the files used for testing along with batch files for automating some of the tests using a VirtualBox virtual machine. The tests are not fully automated, but do get all of the long tasks done.
