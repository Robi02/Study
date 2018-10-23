#!/bin/sh

export JAVA_HOME=/usr/java6_64

export LISTEN_PORT=19988

$JAVA_HOME/bin/java  -DANP_FB0100Server -cp  ./fb0100_study.jar FB0100Server $LISTEN_PORT  2>&1 > FB0100Server.out.txt&

sleep 1
tail -f FB0100Server.out.txt
