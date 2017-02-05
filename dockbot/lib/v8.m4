define(`V8_ARCH', ifelse(ARCH_BITS, 64, `x64', `ia32'))
define(`V8_ROOT', /v8-3.14.5)

RUN git clone --depth=1 https://github.com/CauldronDevelopmentLLC/v8-3.14.5.git
RUN scons -C V8_ROOT mode=release library=static arch=V8_ARCH
RUN scons -C V8_ROOT mode=debug library=static arch=V8_ARCH

ENV V8_HOME=V8_ROOT
