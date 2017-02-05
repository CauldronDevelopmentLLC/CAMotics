RUN git clone --depth=1 https://github.com/CauldronDevelopmentLLC/ChakraCore.git

RUN echo deb http://ftp.debian.org/debian stable-backports main >> \
    /etc/apt/sources.list
APT(clang-3.8 libunwind8-dev libicu-dev)

RUN cd ChakraCore &&\
  ./build.sh --cc=/usr/bin/clang-3.8 --cxx=/usr/bin/clang++-3.8  --static \
    -j CONCURRENCY &&\
  mkdir tmp &&\
  cd tmp &&\
  for i in $(find ../Build* -name \*.a); do ar x $i; done &&\
  ar rcs ../libChakraCore.a *.o

ENV CHAKRA_CORE_HOME /ChakraCore
