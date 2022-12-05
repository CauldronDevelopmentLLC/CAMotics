# Install XCode

Installed through App Store.

# Build V8

Same as on Linux but with pointer compression enabled.

# Install Qt

Installed Qt 5.12.12 as admin.  Linked to ``build`` directory.

# OpenSSL

    curl -fsSLO https://www.openssl.org/source/openssl-3.0.7.tar.gz
    tar xf openssl-3.0.7.tar.gz
    cd openssl-3.0.7
    export MACOSX_DEPLOYMENT_TARGET=10.13
    ./config darwin64-x86_64-cc no-shared
    make -j8

# Install worker

    python3 -m pip install --upgrade pip
    pip3 install --user buildbot-worker

Then:
    WORKER=~/Library/Python/3.9/bin/buildbot-worker
    $WORKER create-worker . master.camotics.org:8012 osx-10_11_6-64bit <password>
    $WORKER start
