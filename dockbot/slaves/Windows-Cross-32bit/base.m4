FROM debian:testing

include(debian.m4)
include(boost-1.59.0.m4)
include(buildbot.m4)

RUN apt-get update && \
  apt-get install -y --no-install-recommends cmake osslsigncode nsis file m4 \
  gcc-mingw-w64-i686 g++-mingw-w64-i686 libarchive-dev less vim \
  libcurl4-openssl-dev libgpgme11-dev unzip pkg-config gettext


# Build and install pacman
RUN wget --quiet https://sources.archlinux.org/other/pacman/pacman-5.0.1.tar.gz
RUN tar xf pacman-5.0.1.tar.gz
RUN cd pacman-5.0.1 && \
   ./configure --prefix=/ && \
   make && \
   make install && \
   cd .. && \
   rm -rf pacman-5.0.1.tar.gz pacman-5.0.1


# Configure pacman for mingw32
RUN echo "\n\
[mingw32]\n\
SigLevel = Required\n\
Server = http://repo.msys2.org/mingw/i686" >> /etc/pacman.conf
RUN pacman-key --init
RUN pacman-key -r 5F92EFC1A47D45A1
RUN pacman-key --lsign-key 5F92EFC1A47D45A1
RUN pacman --noconfirm -Sy --force --asdeps


# Install mingw32 packages with pacman
RUN pacman --noconfirm -S mingw-w64-i686-freetype mingw-w64-i686-freeglut \
  mingw-w64-i686-openssl mingw-w64-i686-fftw mingw-w64-i686-python2 \
  mingw-w64-i686-python2-pygtk mingw-w64-i686-gcc \
  mingw-w64-i686-sqlite3

ENV PATH=/mingw32/bin:$PATH
