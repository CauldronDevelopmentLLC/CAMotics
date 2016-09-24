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


# Setup wine environment 64-bit wine environment
RUN apt-get update && apt-get install -y wine64 wine-binfmt
RUN sed -i 's/bin\/wine/bin\/wine64/' /usr/share/binfmts/wine
RUN echo "\nupdate-binfmts --enable wine" >> $HOME/.bashrc

ENV WINEDEBUG=err-all,warn-all,fixme-all,trace-all
ENV WINESERVER=/usr/lib/wine/wineserver64
ENV WINEPREFIX=/root/.wine
RUN sed -i 's/\(Environment,"PATH".*\)"$/\1;z:\\mingw64\\bin"/' \
  /usr/share/wine/wine/wine.inf
RUN echo "\nwineboot --init" >> $HOME/.bashrc
RUN ln -sf /usr/bin/wine64 /usr/bin/wine


RUN echo "\nalias ls='ls --color'" >> $HOME/.bashrc

# Disable winemenubuilder.exe, services.exe and plugplay.exe
RUN echo "int main(int argc, char *argv[]) {return 0;}" > null.c &&\
  gcc -o /usr/lib/x86_64-linux-gnu/wine/fakedlls/winemenubuilder.exe null.c &&\
  gcc -o /usr/lib/x86_64-linux-gnu/wine/fakedlls/services.exe null.c &&\
  gcc -o /usr/lib/x86_64-linux-gnu/wine/fakedlls/plugplay.exe null.c
