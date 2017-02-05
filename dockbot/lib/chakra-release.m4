define(`CHAKRA_REPO', https://github.com/CauldronDevelopmentLLC/ChakraCore)
define(`CHAKRA_ROOT', ChakraCore-ifelse(ARCH_BITS, 32, `x86', `x64'))

WGET(CHAKRA_REPO/releases/download/CHAKRA_VERSION/CHAKRA_ROOT.zip)

RUN unzip CHAKRA_ROOT.zip &&\
  cp CHAKRA_ROOT/include/* MINGW_ROOT/include/ &&\
  cp CHAKRA_ROOT/ChakraCore.lib MINGW_ROOT/lib/libChakraCore.a &&\
  cp CHAKRA_ROOT/*.dll MINGW_ROOT/bin/
