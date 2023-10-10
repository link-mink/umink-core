#!/bin/bash

C=1
while [ 1 ]
do
    if [ -f /tmp/tmp_file_uuid ]; then
        # generate file
        cd /tmp
        dd if=/dev/urandom of=/tmp/random_file bs=1 count=1024
        split /tmp/random_file -b 256
        CSUM1=$(sha256sum -z /tmp/random_file|cut -d' ' -f1)
        cd -
        UUID=$(cat /tmp/tmp_file_uuid)
        # chunk 00
        echo "Uploading chunk 00"
        mosquitto_pub -h 127.0.0.1 -t "mink.bin/DEBUG_UUID/$UUID/upload" -f /tmp/xaa  -u user -P password
        sleep 1
        # chunk 01
        echo "Uploading chunk 01"
        mosquitto_pub -h 127.0.0.1 -t "mink.bin/DEBUG_UUID/$UUID/upload" -f /tmp/xab  -u user -P password
        sleep 1
        # chunk 02
        echo "Uploading chunk 02"
        mosquitto_pub -h 127.0.0.1 -t "mink.bin/DEBUG_UUID/$UUID/upload" -f /tmp/xac  -u user -P password
        sleep 1
        # chunk 03
        echo "Uploading chunk 03"
        mosquitto_pub -h 127.0.0.1 -t "mink.bin/DEBUG_UUID/$UUID/upload" -f /tmp/xad  -u user -P password
        sleep 2
        CSUM2=$(sha256sum -z /tmp/mink.bin/$UUID|cut -d' ' -f1)
        if [ $CSUM1 != $CSUM2 ]; then
            rm /tmp/mink.bin/$UUID
        fi
        exit 0
    else
        sleep 1
        C=$(( $C + 1 ))
        # max 10 times
        if [ $C == "300" ]; then
            echo "Timeout"
            exit 1
        fi
    fi
done
