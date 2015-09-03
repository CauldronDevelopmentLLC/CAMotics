#!/bin/bash

BUILD=true
CMD=/host/build.sh

if [ "$DEBUG" == "" ]; then DEBUG=0; fi
if [ "$DOCKERFILE" == "" ]; then DOCKERFILE=ubuntu/precise/qt4/Dockerfile; fi
if [ "$DOCKERTAG" == "" ]; then DOCKERTAG=camotics-ubuntu-precise; fi

for i in "$@"; do
    case $i in
        -d) DEBUG=1 ;;
        -r) BUILD=false ;;
        -s) CMD=/bin/bash ;;
        -5) DOCKERFILE=ubuntu/precise/qt5/Dockerfile ;;
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

    cd

    NCORES=$(grep -c ^processor /proc/cpuinfo)
    export SCONS_OPTIONS=$PWD/scons-options.py
    (
        echo "mostly_static=1"
        echo "prefer_dynamic='m'"
        echo "debug=$DEBUG"
        echo "compiler='clang'"
        echo "num_jobs=$NCORES"
    ) > $SCONS_OPTIONS

    export CBANG_HOME=$PWD/cbang

    rm -rf cbang &&
    git clone --depth=1 https://github.com/CauldronDevelopmentLLC/cbang.git &&
    scons -C cbang &&

    rm -rf CAMotics &&
    git clone --depth=1 \
      https://github.com/CauldronDevelopmentLLC/CAMotics.git &&
    scons -C CAMotics &&
    scons -C CAMotics package &&

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
