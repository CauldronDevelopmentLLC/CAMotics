Building CAMotics on Windows
============================
This document will attempt to explain the process of setup up a build
environment for CAMotics in Windows.  This process has been tested on 32 and
64 bit Windows 10 with MSVS 2015 Community edition.

# Dependencies
To build CAMotics you must install the following software:

  - Git (built-in)   - http://git-scm.com/
  - Python (2.7.12)  - http://python.org/
  - SCons (2.5.1)    - http://www.scons.org/
  - Qt (5.8)         - http://qt-project.org/
  - NSIS (3.0.1)     - http://nsis.sourceforge.net/
  - V8 (3.14.5)      - http://github.com/CauldronDevelopmentLLC/v8-3.14.5/
  - C!               - http://github.com/CauldronDevelopmentLLC/cbang/

The versions used when writing this document are noted above.

Git can be installed as part of the MSVS 2015 install.  Install Python, SCons
and 32-bit or 64-bit Qt using their installers.

# Install NSIS
    wget http://prdownloads.sourceforge.net/nsis/nsis-3.01-setup.exe
    nsis-3.01-setup.exe

# Create env.bat
Create the directory ``c:\build`` then in that directory create the file
``env.bat`` with the following contents:

    set PATH=C:\Python27;C:\Python27\Scripts;C:\Program Files\NSIS;%PATH%
    set V8_HOME=c:\build\v8-3.14.5
    set V8_LIBPATH=c:\build\v8-3.14.5\obj\release
    set CBANG_HOME=c:\build\cbang

For 32-bit builds add:

    set TARGET_ARCH=x86
    set QTDIR=c:\Qt\5.8\msvc2015

For 64-bit builds add:

    set TARGET_ARCH=x86_64
    set QTDIR=c:\Qt\5.8\msvc2015_64

# Build
Open a ``VS2015 x86 Native Tools Command Prompt`` from the start menu and run:

    cd c:\build
    env.bat

## Build v8
Get the source:

    git clone --depth=1 https://github.com/CauldronDevelopmentLLC/v8-3.14.5.git

Build 32-bit v8:

    scons -C v8-3.14.5 mode=release arch=ia32 library=static

Or for 64-bit v8:

    scons -C v8-3.14.5 mode=release arch=x64 library=static

## Build cbang
Build cbang with:

    git clone --depth=1 http://github.com/CauldronDevelopmentLLC/cbang.git
    scons -C cbang disable_local="libevent re2" optimize=1

## Build CAMotics
Build CAMotics with:

    git clone --depth=1 http://github.com/CauldronDevelopmentLLC/CAMotics.git
    scons -C CAMotics optimize=1
    scons -C CAMotics package
