#!/bin/bash

sudo apt-get update &&
sudo apt-get install -y scons build-essential libbz2-dev zlib1g-dev \
  libexpat1-dev libssl-dev libboost-iostreams-dev libboost-system-dev \
  libboost-filesystem-dev libboost-regex-dev libsqlite3-dev libv8-dev \
  qt4-dev-tools libqt4-dev libqt4-opengl-dev libcairo2-dev git &&

cd /tmp &&
rm -rf cbang CAMotics &&

git clone https://github.com/CauldronDevelopmentLLC/cbang.git &&
scons -C cbang -j$(grep -c ^processor /proc/cpuinfo) &&
export CBANG_HOME=$PWD/cbang &&

git clone https://github.com/CauldronDevelopmentLLC/CAMotics.git &&
cd CAMotics &&
scons -j$(grep -c ^processor /proc/cpuinfo) &&

scons package &&
sudo dpkg -i camotics_*.deb &&

echo Success || echo Failed
