FROM ubuntu:precise

# Install prerequisites
RUN apt-get update
RUN apt-get install -y --no-install-recommends wget git subversion scons \
  build-essential binutils-dev fakeroot xz-utils libx11-dev libxext-dev \
  libxtst-dev libgl1-mesa-dev libpng-dev libbz2-dev libglu-dev valgrind \
  pkg-config clang

# Setup ncores
RUN echo "grep -c ^processor /proc/cpuinfo" >/usr/bin/ncores
RUN chmod +x /usr/bin/ncores

# Get Boost
ENV BOOST_PKG=boost_1_55_0 BOOST_SOURCE=$HOME/boost_1_55_0
RUN wget --quiet http://downloads.sourceforge.net/project/boost/boost/1.55.0/boost_1_55_0.tar.bz2
RUN tar xjf $BOOST_PKG.tar.bz2 $BOOST_PKG/libs/regex $BOOST_PKG/libs/filesystem $BOOST_PKG/libs/system $BOOST_PKG/libs/iostreams $BOOST_PKG/boost

# Build OpenSSL
ENV OPENSSL_HOME=$HOME/openssl-1.0.2d
RUN wget --quiet https://www.openssl.org/source/openssl-1.0.2d.tar.gz
RUN tar xzf openssl-1.0.2d.tar.gz
RUN cd openssl-1.0.2d ; ./config no-shared && make

# Build V8
ENV V8_HOME=$HOME/v8
ENV PATH=$PATH:/depot_tools
RUN git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
RUN gclient
RUN fetch --no-history v8
RUN cd v8; git checkout 3.14.5.8
RUN make -C v8 x64.release werror=no -j$(ncores)
RUN mkdir v8/lib
RUN cp v8/out/x64.release/obj.target/tools/gyp/*.a v8/lib/

# Build Expat
RUN wget --quiet http://sourceforge.net/projects/expat/files/expat/2.1.0/expat-2.1.0.tar.gz
RUN tar xf expat-2.1.0.tar.gz
RUN cd expat-2.1.0; ./configure --enable-static
RUN make -C expat-2.1.0 -j$(ncores)
RUN make -C expat-2.1.0 -j$(ncores) install

# Build Freetype2
RUN wget --quiet http://sourceforge.net/projects/freetype/files/freetype2/2.6/freetype-2.6.tar.bz2
RUN tar xf freetype-2.6.tar.bz2
RUN cd freetype-2.6; ./configure --enable-static
RUN make -C freetype-2.6 -j$(ncores)
RUN make -C freetype-2.6 -j$(ncores) install
RUN ldconfig
ENV FREETYPE_CFLAGS=-I/usr/local/include/freetype2
ENV FREETYPE_LIBS="-L/usr/local/lib -lfreetype"

# Build FontConfig
RUN wget --quiet http://www.freedesktop.org/software/fontconfig/release/fontconfig-2.11.94.tar.bz2
RUN tar xf fontconfig-2.11.94.tar.bz2
RUN cd fontconfig-2.11.94; ./configure --enable-static --sysconfdir=/etc \
  --localstatedir=/var --disable-docs
RUN make -C fontconfig-2.11.94 -j$(ncores)
RUN make -C fontconfig-2.11.94 -j$(ncores) install

# Build ICU
RUN wget --quiet http://download.icu-project.org/files/icu4c/55.1/icu4c-55_1-src.tgz
RUN tar xf icu4c-55_1-src.tgz
RUN cd icu/source; ./configure --enable-static
RUN make -C icu/source -j$(ncores)
RUN make -C icu/source -j$(ncores) install

# Build Pixman
RUN wget --quiet http://cairographics.org/releases/pixman-0.32.6.tar.gz
RUN tar xf pixman-0.32.6.tar.gz
RUN cd pixman-0.32.6; ./configure --enable-static
RUN make -C pixman-0.32.6 -j$(ncores)
RUN make -C pixman-0.32.6 -j$(ncores) install

# Build Cairo
RUN wget --quiet http://cairographics.org/releases/cairo-1.14.2.tar.xz
RUN tar xf cairo-1.14.2.tar.xz
RUN cd cairo-1.14.2; ./configure --enable-static
RUN make -C cairo-1.14.2 -j$(ncores)
RUN make -C cairo-1.14.2 -j$(ncores) install

# Build Qt 4.8
ENV QT_HOME=$HOME/qt-4.8.7
RUN wget --quiet http://download.qt-project.org/official_releases/qt/4.8/4.8.7/qt-everywhere-opensource-src-4.8.7.tar.gz
RUN tar xzf qt-everywhere-opensource-src-4.8.7.tar.gz
RUN mv qt-everywhere-opensource-src-4.8.7 $QT_HOME

RUN wget --quiet http://lug.mtu.edu/debian/pool/main/q/qt4-x11/qt4-x11_4.8.7%2bdfsg-3.debian.tar.xz
RUN cd $QT_HOME; tar xf ../qt4-x11_4.8.7+dfsg-3.debian.tar.xz
RUN cd $QT_HOME; for i in debian/patches/*.diff; do patch -p1 <$i; done
RUN sed -i 's/-licuuc -licui18n/-licui18n -licuuc -licudata -ldl/' $QT_HOME/config.tests/unix/icu/icu.pro

RUN cd $QT_HOME; ./configure -confirm-license -opensource -debug \
 -openssl-linked -no-phonon -no-phonon-backend -no-webkit -no-openvg \
 -nomake demos -nomake examples -optimized-qmake -opengl desktop -static \
 -qt-libpng -qt-libtiff -qt-libjpeg -fast -no-qt3support -no-qt3support -icu \
 -I/$OPENSSL_HOME/include -L/$OPENSSL_HOME
RUN make -C $QT_HOME -j$(ncores)
RUN make -C $QT_HOME install -j$(ncores)
ENV PATH=/usr/local/Trolltech/Qt-4.8.7/bin:$PATH
ENV PKG_CONFIG_PATH=/usr/local/Trolltech/Qt-4.8.7/lib/pkgconfig/

# Build Qt 5.5
ENV QT5_URL=http://download.qt.io/official_releases/qt/5.5/5.5.0/submodules/
ENV QT5_PKG=opensource-src-5.5.0
RUN wget --quiet $QT5_URL/qt5-$QT5_PKG.tar.xz
RUN wget --quiet $QT5_URL/qtbase-$QT5_PKG.tar.xz
RUN wget --quiet $QT5_URL/qtimageformats-$QT5_PKG.tar.xz
RUN tar xf qt5-$QT5_PKG.tar.xz
RUN tar xf qtbase-$QT5_PKG.tar.xz
RUN tar xf qtimageformats-$QT5_PKG.tar.xz
ENV QT5_HOME=qt-everywhere-$QT5_PKG
RUN mv qtbase-$QT5_PKG $QT5_HOME/qtbase
RUN mv qtimageformats-$QT5_PKG $QT5_HOME/qtimageformats

ENV LD_LIBRARY_PATH=/usr/local/lib
RUN cd $QT5_HOME; ./configure -confirm-license -opensource -release -no-openvg \
  -nomake tests -nomake examples -opengl desktop -qt-libpng -qt-libjpeg -icu \
  -no-sql-mysql -no-sql-sqlite -qt-xcb
RUN make -C $QT5_HOME -j$(ncores)
RUN make -C $QT5_HOME -j$(ncores) install

ENV PATH=/usr/local/Qt-5.5.0/bin:$PATH
ENV PKG_CONFIG_PATH=/usr/local/Qt-5.5.0/lib/pkgconfig

ENV DOCKER_GUEST=true
