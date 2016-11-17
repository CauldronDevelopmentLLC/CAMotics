ENV V8_VERSION=5.6.149
RUN mkdir v8
RUN cd v8 && git clone --depth=1 \
  https://chromium.googlesource.com/chromium/tools/depot_tools.git
RUN cd v8 && \
  git clone --depth=1 -b $V8_VERSION https://chromium.googlesource.com/v8/v8.git
RUN cd v8 && export PATH=$PWD/depot_tools:$PATH && \
  gclient config https://chromium.googlesource.com/v8/v8.git --unmanaged && \
  gclient sync --no-history --shallow
RUN cd v8 && export PATH=$PWD/depot_tools:$PATH && \
  cd v8 && \
  ln -sf /usr/bin/clang third_party/llvm-build/Release+Asserts/bin/clang && \
  gn gen out --args='is_debug=false target_cpu="x64" is_component_build=false treat_warnings_as_errors=false v8_use_snapshot=false v8_enable_i18n_support=false enable_basic_printing=false use_sysroot=false is_desktop_linux=false linux_use_bundled_binutils=false clang_use_chrome_plugins=false' && \
  ninja -C out && \
  for i in $(find out -name \*.a | grep -v /test); do \
    ar t $i; \
  done | xargs ar rcs libv8.a

ENV V8_HOME=/v8/v8 V8_LIBPATH=/v8/v8 V8_INCLUDE=/v8/v8/include
