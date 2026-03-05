#!/bin/bash
# Script used to create logical partitions with random sizes (50–250 MB).

DISK="/dev/sdc"

(
for i in $(seq 1 200); do
    SIZE=$((50 + RANDOM % 201))
    echo n
    echo l
    echo
    echo +${SIZE}M
done
echo w
) | fdisk "$DISK"
