#!/bin/bash
# Script used to create GPT partitions with random sizes (50–250 MB).

DISK="/dev/sdb"
START=1

for i in $(seq 1 200); do
    SIZE=$((50 + RANDOM % 201))
    END=$((START + SIZE))

    echo "Creating GPT partition $i: ${SIZE}MB"
    parted -s $DISK mkpart primary ${START}MiB ${END}MiB

    START=$END
done
