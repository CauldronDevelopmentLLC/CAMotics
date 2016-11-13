RUN apt-get update && apt-get install -y subversion

RUN svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm && \
  cd llvm/tools && \
  svn co http://llvm.org/svn/llvm-project/cfe/trunk clang && \
  cd .. && \
  mkdir build && \
  cd build && \
  cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/usr .. && \
  make && \
  make install && \
  cd .. && \
  rm -rf llvm
