# Setup cross compiler
RUN \
  CROSS=x86_64-w64-mingw32 ;\
  ln -s /usr/bin/${CROSS}-windres /usr/bin/windres &&\
  ln -s /usr/bin/${CROSS}-gcc /usr/bin/${CROSS}-cc &&\
  for i in gcc g++ cc c++ ld strip; do\
    update-alternatives --install /usr/bin/$i $i /usr/bin/${CROSS}-$i 10 &&\
    update-alternatives --set $i /usr/bin/${CROSS}-$i ||\
    exit 1;\
  done

# Fix CMake cross-compile
RUN sed -i 's/^\(.*"-rdynamic".*\)$/#\1/' \
  /usr/share/cmake-*/Modules/Platform/Linux-GNU.cmake

ENV C_INCLUDE_PATH=/mingw64/include CPLUS_INCLUDE_PATH=/mingw64/include
