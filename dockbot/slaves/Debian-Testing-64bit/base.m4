FROM debian:testing

include(debian.m4)
include(boost-1.59.0.m4)
include(buildbot.m4)

ENV GCC_VERSION 4.9.4
include(gcc.m4)
