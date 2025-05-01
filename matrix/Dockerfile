# a debian based image, i.e. Ubuntu
# set to scratch so it will fail immediately if the build arg is not set
ARG IMAGE="scratch"
FROM ${IMAGE} AS base


########################## 
FROM base AS build

# required apt packages which may be slightly different on different flavors of debian
ARG DEPS=""

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y $DEPS

# clone our stuff into root's home
WORKDIR /root

############################################
# clone and build cbang
RUN git clone --depth=1 https://github.com/CauldronDevelopmentLLC/cbang.git
# apply a fix for
# https://github.com/CauldronDevelopmentLLC/cbang/issues/161
COPY matrix/patch/cbang_v8.patch cbang/
RUN cd cbang && \
    git checkout d4dd5f52b0d5ee499ec5683f5930b01fdb24884b && \
    git apply cbang_v8.patch

# `v8.h` is provided by libnode-dev
RUN scons -C cbang v8_compress_pointers=false
# assign the environment variable for cbang
ENV CBANG_HOME=/root/cbang

#################################################
# copy the current checkout of camotics into the working directory
# note that this requires that the docker build context is the root of
# the camotics repo i.e. `docker build -f matrix/Dockerfile .`
COPY . CAMotics

# set our working directory to the camotics checkout
WORKDIR /root/CAMotics

# build the executable for camotics
RUN scons

# package the deb
RUN scons package



#################################
# test the debian in an image without the build dependencies
# to make sure that we listed the install dependencies correctly
FROM base AS test

# copy our built package
COPY --from=build /root/CAMotics/camotics*.deb .

# install the deb, this will always complain about the missing dependencies
RUN apt-get update && apt-get install aptitude -y && dpkg -i camotics*.deb || true

RUN apt show camotics && aptitude why-not camotics || true

# this should resolve the missing dependencies if we listed them correctly
RUN apt-get install -f -y

# this simple smoke test shouldn't crash
RUN camotics --version

# now that we've passed the smoke test rename the deb
# apply the distro information to the deb, i.e. `ubuntu-20.04`
COPY ./matrix/rename.py .
# TODO : should this be in the `debian/control` info?
RUN apt-get install python3 lsb-release -y && python3 rename.py

#########################
# copy the deb into a scratch image so we can eject it in the build command
FROM scratch

# copy from test stage so that it doesn't skip that stage
COPY --from=test camotics*.deb .

