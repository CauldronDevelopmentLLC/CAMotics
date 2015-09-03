#!/bin/bash

BUILD=true
CMD=/host/build.sh

if [ "$DEBUG" == "" ]; then DEBUG=0; fi
if [ "$DOCKERFILE" == "" ]; then DOCKERFILE=Dockerfile.qt4; fi
if [ "$DOCKERTAG" == "" ]; then DOCKERTAG=camotics-ubuntu-precise; fi

for i in "$@"; do
    case $i in
        -d) DEBUG=1 ;;
        -r) BUILD=false ;;
        -s) CMD=/bin/bash ;;
        -5) DOCKERFILE=Dockerfile.qt5 ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "  -d         Build in debug mode."
            echo "  -r         Run docker but don't build the docker image."
            echo "  -s         Run a docker shell."
            echo "  -h|--help  Print this help screen and exit."
            exit 0
        ;;
    esac
done


if [ "$DOCKER_GUEST" == "true" ]; then
    if [ ! -d /host ]; then
        echo "/host not mounted"
        exit 1
    fi

    NCORES=$(grep -c ^processor /proc/cpuinfo)

    cd

    export CBANG_HOME=$PWD/cbang

    export GCC=/usr/bin/clang
    export GXX=/usr/bin/clang++

    rm -rf cbang &&
    git clone --depth=1 https://github.com/CauldronDevelopmentLLC/cbang.git &&
    scons -C cbang -j$NCORES debug=$DEBUG compiler=clang &&

    rm -rf CAMotics &&
    git clone --depth=1 \
      https://github.com/CauldronDevelopmentLLC/CAMotics.git &&
    scons -C CAMotics -j$NCORES mostly_static=1 prefer_dynamic=m debug=$DEBUG compiler=clang &&
    scons -C CAMotics -j$NCORES mostly_static=1 prefer_dynamic=m package &&

    cp CAMotics/camo{tics,sim,opt,probe} CAMotics/camotics_*.deb /host/ &&

    echo Success

else
    cd $(dirname "$0")

    if $BUILD; then
        docker build -f $DOCKERFILE --rm -t $DOCKERTAG . || exit 1
    fi

    docker run -e DEBUG=$DEBUG -e DOCKER_GUEST=true -v "$PWD":/host -it --rm \
      $DOCKERTAG $CMD
fi
