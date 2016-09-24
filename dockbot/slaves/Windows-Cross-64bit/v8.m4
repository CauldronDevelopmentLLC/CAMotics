ENV V8_REL=3.14.5
RUN git clone -b $V8_REL https://chromium.googlesource.com/v8/v8.git
RUN cd v8 && \
  sed -i "s/'-Werror',//" SConstruct && \
  scons --warn=no-all mode=release arch=x64 toolchain=gcc library=static \
    os=win32 I_know_I_should_build_with_GYP=yes

ENV V8_HOME=/v8 V8_LIBPATH=/v8 V8_INCLUDE=/v8/include
