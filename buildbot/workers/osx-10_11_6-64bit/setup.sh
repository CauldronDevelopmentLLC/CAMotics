#!/bin/bash

# Create build directory
cd ~
mkdir build
cd build


# Build libv8
git clone -b 3.14.5 https://chromium.googlesource.com/v8/v8.git &&
cd v8 &&
sed -i .bak "s/'-Werror',//" SConstruct &&
CCFLAGS=-fpermissive \
    scons --warn=no-all mode=release arch=x64 toolchain=gcc library=static \
    I_know_I_should_build_with_GYP=yes &&
cd ..


# Download boost
export BOOST_VERSION=1.59.0
export BOOST_PKG=$(echo boost_$BOOST_VERSION | tr . _)
wget --quiet http://downloads.sourceforge.net/project/boost/boost/$BOOST_VERSION/$BOOST_PKG.tar.bz2 &&
tar xf $BOOST_PKG.tar.bz2 $BOOST_PKG/libs/regex $BOOST_PKG/libs/filesystem \
  $BOOST_PKG/libs/system $BOOST_PKG/libs/iostreams $BOOST_PKG/boost


# OpenSSL
export OPENSSL_VERSION=1.0.2i
wget https://www.openssl.org/source/openssl-$OPENSSL_VERSION.tar.gz &&
tar xf openssl-$OPENSSL_VERSION.tar.gz &&
cd openssl-$OPENSSL_VERSION &&
./Configure darwin64-x86_64-cc &&
make &&
cd ..


# Freetype
wget http://download.savannah.gnu.org/releases/freetype/freetype-2.7.tar.bz2 &&
tar xf freetype-2.7.tar.bz2 &&
cd freetype-2.7 &&
./configure &&
make &&
sudo make install


# Cairo
wget https://www.cairographics.org/releases/cairo-1.14.6.tar.xz &&
tar xf cairo-1.14.6.tar.xz &&
cd cairo-1.14.6 &&
./configure &&
make &&
sudo make install


# Qt4
wget https://download.qt.io/archive/qt/4.8/4.8.6/qt-opensource-mac-4.8.6-1.dmg
hdiutil attach qt-opensource-mac-4.8.6-1.dmg
mkdir qt-4.8.6
cd qt-4.8.6
for pkg in libraries imports headers plugins tools; do
    gzcat /Volumes/Qt\ 4.8.6/packages/Qt_$pkg.pkg/Contents/Archive.pax.gz | pax -r
done

mkdir pkgconfig
cd pkgconfig


# Qt5
#wget http://download.qt.io/official_releases/online_installers/qt-unified-mac-x64-online.dmg
