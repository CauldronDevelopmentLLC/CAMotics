#!/bin/bash

cd "$(dirname "%0")"

for MODE in debug release; do
    if [ -e $MODE/twistd.pid ]; then
        PID=$(cat $MODE/twistd.pid)
        echo "Stopping $MODE on PID $PID"
        kill $PID
        wait $PID
    fi

    echo Starting $MODE
    (
        cd $MODE;
        . ../env
        twistd -y slave.tac
    ) && echo "OK" || echo "Failed"
done
