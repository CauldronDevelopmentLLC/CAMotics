# a debian based image, i.e. Ubuntu
ARG IMAGE="ubuntu:24.04"

FROM ${IMAGE} AS build

# a list of apt-get packages, this may be subtly
# different on different flavors of debian
ARG DEPS="scons build-essential libqt5websockets5-dev libqt5opengl5-dev qttools5-dev-tools libnode-dev libglu1-mesa-dev pkgconf git ca-certificates python3-six fakeroot"

RUN apt-get update && \
    apt-get install -y --no-install-recommends $DEPS

# clone our stuff into root's home
WORKDIR /root

# copy the repo into the working directory
COPY . CAMotics

# TODO : should this be locked to a commit?
# clone cbang
RUN git clone https://github.com/CauldronDevelopmentLLC/cbang.git

# build cbang
RUN scons -C cbang

# assign the environment variable
ENV CBANG_HOME=/root/cbang

# set our working directory to the camotics checkout
WORKDIR /root/CAMotics

# build the executable for camotics
RUN scons

# package the deb
RUN scons package

RUN ls -altrsh

FROM scratch

COPY --from=build /root/CAMotics/*.deb .
