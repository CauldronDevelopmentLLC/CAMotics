#!/bin/bash -e

CPUS=$(grep -c ^processor /proc/cpuinfo)

# Prerequisites
sudo apt-get update
sudo apt-get install -y scons build-essential libboost-iostreams-dev \
  libboost-system-dev libboost-filesystem-dev libboost-regex-dev \
  libsqlite3-dev qt4-dev-tools libqt4-dev libqt4-opengl-dev libcairo2-dev git

# Setup
cd /tmp
rm -rf cbang CAMotics ChakraCore

# ChakraCore
git clone --depth=1 https://github.com/CauldronDevelopmentLLC/ChakraCore.git
(
  cd ChakraCore
  ./build.sh --static -j $CPUS
  mkdir tmp
  cd tmp
  for i in $(find ../Build* -name \*.a); do ar x $i; done
  ar rcs ../libChakraCore.a *.o
)
export CHAKRA_CORE_HOME=$PWD/ChakraCore

# C!
git clone --depth=1 https://github.com/CauldronDevelopmentLLC/cbang.git
scons -C cbang -j$CPUS
export CBANG_HOME=$PWD/cbang

# CAMotics
git clone --depth=1 https://github.com/CauldronDevelopmentLLC/CAMotics.git
cd CAMotics
scons -j$CPUS

# Package
scons package
sudo dpkg -i camotics_*.deb

echo Success
