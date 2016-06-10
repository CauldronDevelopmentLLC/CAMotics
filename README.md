[![Build Status](https://travis-ci.org/CauldronDevelopmentLLC/CAMotics.svg?branch=master)](https://travis-ci.org/CauldronDevelopmentLLC/CAMotics)

![CAMotics Logo][1]

CAMotics is an Open-Source software which can simulate 3-axis NC
machining. It is a fast, flexible and user friendly simulation
software for the DIY and Open-Source community.  CAMotics works on
Linux, OS-X and Windows.

At home manufacturing is one of the next big technology
revolutions. Much like the PC was 30 years ago. There have been major
advances in desktop 3D printing (e.g. ​Maker Bot) yet uptake of desktop
CNCs has lagged despite the availability of ​cheap CNC machines. One of
the major reasons for this is a lack of Open-Source simulation and CAM
(3D model to tool path conversion) software. CAM and NC machine
simulation present some very difficult programming problems, as is
evidenced by 30+ years of academic papers on these topics. Whereas, 3D
printing simulation and tool path generation are much easier. Such
software is essential to using a CNC.

Being able to simulate is a critical part of creating CNC tool
paths. Programming a CNC with out a simulator is cutting with out
measuring; it's both dangerous and expensive. With CAMotics you can
preview the results of your cutting operation before you fire up your
machine. This will save you time and money and open up a world of
creative possibilities by allowing you to rapidly visualize and
improve upon designs with out wasting material or breaking tools.

See http://camotics.org/

# License
GNU General Public License version 2+.  See the file COPYING.

# Building from Source
This section describes how to build CAMotics from source on Debian based
systems such as Ubuntu and Mint Linux.  If you are running Windows or OSX
it is much easier to simply install prebuilt packages which can be found
at http://camotics.org/downloads  There are also prebuilt Debian packages
you can try.

## The Quick and Easy Method
*Note, this currently only works for Debian based systems.*  Cut and paste the
following command in to a terminal:

    curl http://camotics.org/build.sh | sh

You will have to enter your password, possibly a few times.  When it's done
it should print ``Success``.  Then you can run ``camotics``.

# Prerequisites
  - C!     - http://cbang.org/
  - Qt4    - http://qt-project.org/
  - Cairo2 - http://cairographics.org/
  - SCons  - http://www.scons.org/

On Debian based systems all the prerequisites, including thoese needed
by C!, can be installed with the following command line:

    sudo apt-get install scons build-essential libbz2-dev zlib1g-dev \
      libexpat1-dev libssl-dev libboost-iostreams-dev libboost-system-dev \
      libboost-filesystem-dev libboost-regex-dev libsqlite3-dev libv8-dev \
      qt4-dev-tools libqt4-dev libqt4-opengl-dev libcairo2-dev git

## Building C!

Clone the C! git repository, build the software using scons and set the
environment variable CBANG_HOME so the CAMotics build system can find it
later.  **You must install libv8 before this step.**

    git clone https://github.com/CauldronDevelopmentLLC/cbang.git
    scons -C cbang
    export CBANG_HOME=$PWD/cbang

## Building CAMotics

Clone the CAMotics git repository and build the software using scons:

    git clone https://github.com/CauldronDevelopmentLLC/CAMotics.git
    cd CAMotics
    scons

## Building & Installing the Debian Package

In the CAMotics source code directory run:

    scons package
    sudo dpkg -i camotics_*.deb

## Build Warnings/Errors
If you get any build warnings, by default, the build will stop.  If you have
problems building, especially with warnings related to the boost library you
can ignore these warnings by building cbang and/or CAMotics with
`scons strict=0`.  This disables strict checking.  For example:

    scons -C cbang strict=0
    cd CAMotics
    scons strict=0

# Using CAMotics

If you've installed the Debian package you should find CAMotics in your menu
under Other.  Also you can simply run `camotics` on the command line.

If you did not install the package, open a command line, go to the directory
where you built CAMotics and run `./camotics`

# Try the Examples

Try out some of the examples in CAMotics's File -> Examples menu.

## No Icons in Menus in Linux
If you don't see icons in CAMotics menus in Linux try running the following
command and restarting CAMotics:

    gconftool-2 --type boolean --set /desktop/gnome/interface/menus_have_icons true

[1]: https://raw.githubusercontent.com/CauldronDevelopmentLLC/CAMotics/master/images/camotics-logo.png
