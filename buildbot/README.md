## Master Setup
Install software:

    sudo apt-get install -y python3-pip python3-virtualenv git

Create and enter an Python virtual environment in this directory:

    virtualenv .
    source bin/activate

Install buildbot in the virtual environment with:

    pip3 install buildbot buildbot-www buildbot-waterfall-view \
      buildbot-console-view buildbot-wsgi_dashboards boto3

Build the web plugin:

    npm install
    npm run build

Create and start the master:

    buildbot create-master
    buildbot start

## Worker Setup
Install software:

    sudo apt-get install -y python3-pip python3-virtualenv git

Create and enter an Python virtual environment:

    virtualenv worker
    cd worker
    source bin/activate

Install buildbot in the virtual environment with:

    pip3 install buildbot-worker

Create the worker

    buildbot-worker create-worker . localhost:8012 <worker> <password>

Where ``<worker>`` and ``<password>`` are the worker name and password and
defined on the master.  The worker name will be the same as it's directory name
in the ``workers`` directory.

Start the worker

    buildbot-worker start

Follow other worker specific setup directions in the worker ``README.md``.
