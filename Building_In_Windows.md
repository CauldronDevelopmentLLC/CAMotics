Building from source in Windows is much more difficult because satisfying the dependecies with out the help of a package system is more of a challenge.  You can build everything from source or you can find prebuilt libraries for some of the dependencies.

These instructions assume you are using MSVC 2008 or better on Windows XP or newer.  Building with Cygwin or MinGW is not supported or recommended.  We will build everything from the command line.

# Prerequisites
## Tools
You will need to isntall the following bulid tools:

  * MSVS 2008 or better https://www.visualstudio.com/
  * Git http://git-scm.com/
  * Python https://www.python.org/
  * SCons http://www.scons.org/

## Libraries
Build or download binaries of the following, in this order:

  * Boost http://www.boost.org/
  * V8 http://code.google.com/p/v8/ 
  * C! http://cbang.org/
  * FreeType http://www.freetype.org/
  * Qt4  http://qt-project.org/
  * Cairo http://cairographics.org/

# Preparation
## Create a build directory
You will need to build or install several different packages.  It is recommend that you create a ```build``` directory somewhere on your system and build or install all the packages under this directory.

## Determine the Build Type
You should determine your system's bit width, either *32-bit* or *64-bit*.  32-bit mode is also refered to as *x32* or *x86* mode and 64-bit mode may be refered to as *x64* or *amd64* mode.  This will be an important distinction for several of the packages.  Note, that it is perfectly reasonable and will not adversely affect performance if you build in 32-bit mode on a 64-bit system.  In fact it is much easier to find 32-bit binaries of the library dependencies.

You also need to decide to either build or download only *debug* or *release* mode packages.  Windows cannot mix packages built with different release modes.

## Maintaining the Environment
There are several environment variables that you will need to set in order for the build scripts to be able to find the other packages.  You can enter these on the command line but it is far easier to maintain a ```env.bat``` file which you can run at any time to restore the build environment.

In order for the build scripts to find MSVC, you need to call the appropriate ```vcvars.bat```.  This script configures your environment to use the correct MSVC compiler.  You can also add this to your ```env.bat``` for example you might have line like this:

    call "%ProgramFiles%\Microsoft Visual Studio 9.0\vc\vcvarsall.bat" x86

Note that there are 32-bit and 64-bit versions of this batch file.

Your ```env.bat``` might look something like this:

```
@echo off

set BUILD_ROOT=C:\build

call "%ProgramFiles(x86)%\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86

set PATH=%PATH%;%BUILD_ROOT%\depot_tools;C:\Python27\Scripts;%ProgramFiles(x86)%\NSIS

set BOOST_SOURCE=%BUILD_ROOT%\boost_1_57_0
set V8_HOME=%BUILD_ROOT%\v8
set CBANG_HOME=%BUILD_ROOT%\cbang
set QTDIR=%BUILD_ROOT%\Qt\4.8.6
set CAIRO_HOME=%BUILD_ROOT%\gtk
set FREETYPE2_HOME=%BUILD_ROOT%\freetype-2.3.5-1
```

# MSVS
You can get a free version of Visual Studio here: https://www.visualstudio.com/.  You do not need any of the optional features.

# Git
You can find Windows binaries here: http://git-scm.com/download/win.  It is recommended that you select the options that installs the extra *Unix tools* and make ```git``` available on the Windows command line.

# Python
Python is needed to run SCons.  You can find binaries here: https://www.python.org/downloads/windows/  You should download and install the latest Python 2.

# SCons
Download and install the lastest SCons from here: http://www.scons.org/download.php.  It is not recommend that you build SCons from source.  After installing add SCons to the PATH.  SCons installs to the Python ```Scripts``` directory.

    set PATH=%PATH%;C:\Python27\Scripts

# Boost
It is not actually necessary to build Boost yourself.  If you download, unpack and set the environment variable ```BOOST_SOURCE``` C! will build the parts of boost that it needs automatically.  You can find the latest version here: http://www.boost.org/users/download/#live

# V8
First you need to install Google's depot tools.

    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git

Next add depot tools to your ```PATH```.  For example:

    set PATH=%PATH%:%CD%/depot_tools

Next get and build libv8 like this:

    gclient
    fetch v8
    cd v8
    git checkout 3.14.5.8
    python build\gyp_v8
    devenv /build Release build\All.sln

If you are creating a 64-bit build then add ```-Dtarget_arch=x64``` to the end of the ```build\gyp_v8``` command.  If you are building in debug mode then change the last line.

# C!
First make sure you have all all of the following environment variables pointing to the root directories of the packages built above:

 * ```BOOST_SOURCE```
 * ```V8_HOME```

Check out the latest source code and build like this:

    git clone https://github.com/CauldronDevelopmentLLC/cbang.git
    cd cbang
    scons

Or to build in debug mode change the last line to:

    scons debug=1

Then set ```CBANG_HOME```.

# FreeType
You can download Windows binaries for FreeType here: http://gnuwin32.sourceforge.net/packages/freetype.htm

# Qt
You can download Windows binaries for Qt4 here: https://www.qt.io/download-open-source/.  Instructions for building from source can be found here: http://doc.qt.io/qt-4.8/install-win.html.  Make sure you install or build Qt with OpenGL support.

# Cairo
You can find prebuilt Windows binaries for cairo here: http://www.gtk.org/download/.  It's easiest to just install [the whole Gtk package](http://win32builder.gnome.org/gtk+-bundle_3.6.4-20130921_win32.zip) because you will need several of the included DLLs later.  Or build instructions here: http://cairographics.org/end_to_end_build_for_win32/

# OpenSCAM
Make sure you have all of the following envrionement variables set correctly:

 * ```BOOST_SOURCE```
 * ```V8_HOME```
 * ```QTDIR```
 * ```CAIRO_HOME```
 * ```FREETYPE2_HOME```

Now you can check out and build OpenSCAM like this:

    git clone https://github.com/CauldronDevelopmentLLC/OpenSCAM.git
    cd OpenSCAM
    scons

or in debug mode:

    scons debug=1

