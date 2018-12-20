#!/bin/sh

export JAVA_HOME=/usr/java6_64

export FB_IP=127.0.0.1
export FB_PORT=19988

export FB_PARENT_BANK_CODE_3=039
export FB_PARENT_COMP_CODE=7770011
export FB_PARENT_ACCOUNT_NUMB=86088800173

export FB_REQ_FILE=35350081.180404104958

export FB_MSG_NUMB_S=0

$JAVA_HOME/bin/java -DANP_FB0100FConv -cp ./fb0100_study.jar FB0100FConv $FB_MSG_NUMB_S 2>&1 > FB0100FConv.out.txt&

sleep 1
tail -f FB0100FConv.out.txt
