Building from source in Windows is much more difficult because satisfying the dependecies with out the help of a package system is more of a challenge.  You can build everything from source or you can find prebuilt libraries for some of the dependencies.

These instructions assume you are using MSVC 2008 or better on Windows XP or newer.  Building with Cygwin or MinGW is not supported or recommended.  We will build everything from the command line.

# Prerequisites
Build or download binaries of the following, in this order:

  * SCons http://www.scons.org/
  * Perl https://www.perl.org/
  * OpenSSL http://www.openssl.org/
  * Boost http://www.boost.org/
  * Git http://git-scm.com/
  * V8 http://code.google.com/p/v8/ 
  * C! http://cbang.org/

# Preparation
## Create a build directory
You will need to build or install several different packages.  It is recommend that you create a ```build``` diretory somewhere on your system and build or install all the packages under this directory.

## Determining the Build Type
You should determine your system's bit width, either 32 or 64 as this will be an important distinction for several of the packages.  You also need to decide to either build or download only *debug* or *release* mode packages.  Windows cannot mix packages build in different release modes.

## Maintaining the Environment
There are several environment variables that you will need to set in order for the build scripts to be able to find the other packages.  You can enter these on the command line but it is far easier to maintain a ```env.bat``` file which you can run at any time to get your configuration back.

In order for the build scripts to find MSVC, you need to call the appropriate ```vcvars.bat```.  This script configures your environmen to use the correct MSVC compiler.  You can also add this to your ```env.bat``` for example you might have line like this:

    call "%ProgramFiles%\Microsoft Visual Studio 9.0\vc\bin\vcvars32.bat"

Note that there are 32-bit and 64-bit versions of this batch file.

# SCons
Download and install the lastest SCons from here: http://www.scons.org/download.php.  It is not recommend that you build SCons from source.

# OpenSSL
You can find Windows binaries of OpenSSL here: https://www.openssl.org/related/binaries.html

Alternatively you can build from source but this also requires that you install Perl.

 1. Install either [Active Perl](http://www.activestate.com/activeperl/downloads) or [Strawberry Perl](http://strawberryperl.com/).
 2. Download the latest source package from https://www.openssl.org/source/
 2. Unpack the source.
 3. Move in to the source directory on the command line.
 4. Build the package like this:
        perl Configure VC-WIN32
        ms\do_ms
        nmake -f ms\nt.mak 

Note, that the above is for a 32-bit release mode build.  The possible configuration targets are:

 * ```VC-WIN32```
 * ```debug-VC-WIN32```
 * ```VC-WIN64A```
 * ```debug-VC-WIN64A```

Alternative instructions can be found here:

 * http://developer.covenanteyes.com/building-openssl-for-visual-studio/
 * http://p-nand-q.com/programming/windows/building_openssl_with_visual_studio_2013.html

# Boost
It is not actually necessary to build Boost yourself.  If you download, unpack and set the environment variable ```BOOST_SOURCE``` C! will build the parts of boost that it needs automatically.  You can find the latest version here: http://www.boost.org/users/download/#live

# V8
To get V8 you first need to install Git.  You can find Windows binaries here: http://git-scm.com/download/win.

Next you need to install Google's depot tools.

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

 * ```OPENSSL_HOME```
 * ```BOOST_SOURCE```
 * ```V8_HOME```

Check out the latest source code and build like this:

    git clone https://github.com/CauldronDevelopmentLLC/cbang.git
    cd cbang
    scons

Or to build in debug mode change the last line to:

    scons debug=1

Then set ```CBANG_HOME```.

# OpenSCAM
Now you can check out and build OpenSCAM like this:

    git clone https://github.com/CauldronDevelopmentLLC/OpenSCAM.git
    scons

or in debug mode:

    scons debug=1

