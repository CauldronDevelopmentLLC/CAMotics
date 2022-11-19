## Install packages

    sudo apt-get update
    sudo apt-get install -y wget git scons build-essential binutils-dev \
      fakeroot python3-pip python3-virtualenv debian-keyring \
      debian-archive-keyring ca-certificates libssl-dev

## Install systemd service

Install ``buildbot-worker.service`` on worker, then:

    cd /home/admin/worker
    sudo systemctl enable --now $PWD/buildbot-worker.service
