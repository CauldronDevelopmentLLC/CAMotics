RUN pacman --noconfirm -S mingw-w64-x86_64-qt5
ENV PKG_CONFIG_PATH=/mingw64/lib/pkgconfig QTDIR=/mingw64
