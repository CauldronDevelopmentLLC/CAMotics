RUN apt-get update && \
  apt-get install -y --no-install-recommends \
  gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64 libarchive-dev \
  libcurl4-openssl-dev libgpgme11-dev dirmngr


# Build and install pacman
RUN wget --quiet https://sources.archlinux.org/other/pacman/pacman-5.0.1.tar.gz
RUN tar xf pacman-5.0.1.tar.gz
RUN cd pacman-5.0.1 && \
   ./configure --prefix=/ && \
   make && \
   make install && \
   cd .. && \
   rm -rf pacman-5.0.1.tar.gz pacman-5.0.1


# Configure pacman for mingw64
RUN echo "\n\
[mingw64]\n\
SigLevel = Required\n\
Server = http://repo.msys2.org/mingw/x86_64" >> /etc/pacman.conf
RUN pacman-key --init
RUN pacman-key -r 5F92EFC1A47D45A1
RUN pacman-key --lsign-key 5F92EFC1A47D45A1
RUN pacman --noconfirm -Sy --force --asdeps


# Install mingw64 packages with pacman
RUN pacman --noconfirm -S mingw-w64-x86_64-freetype mingw-w64-x86_64-freeglut \
  mingw-w64-x86_64-openssl mingw-w64-x86_64-fftw mingw-w64-x86_64-python2 \
  mingw-w64-x86_64-python2-pygtk mingw-w64-x86_64-gcc \
  mingw-w64-x86_64-sqlite3

ENV PATH=/mingw64/bin:$PATH
