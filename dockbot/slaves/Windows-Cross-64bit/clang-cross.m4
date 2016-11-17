# Build clang cross compiler wrapper
RUN git clone --depth=1 https://github.com/cauldrondevelopmentllc/wclang.git
RUN mkdir wclang/build && \
  cd wclang/build && \
  cmake -DCMAKE_INSTALL_PREFIX=/mingw64 .. && \
  make -j $(grep -c ^processor /proc/cpuinfo) && \
  make install && \
  cd ../.. && \
  rm -rf wclang
