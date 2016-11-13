FROM debian:testing

include(debian.m4)
include(boost-1.59.0.m4)
include(buildbot.m4)
include(clang.m4)
include(mingw64.m4)
include(clang-cross.m4)

RUN apt-get update && \
  apt-get install -y --no-install-recommends osslsigncode nsis
