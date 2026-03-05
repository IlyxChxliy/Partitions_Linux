#!/bin/bash
# Script used to create the maximum number of logical partitions in MBR.

DISK="/dev/sdc"

(
for i in $(seq 1 200); do
    echo n
    echo l
    echo
    echo +50M
done
echo w
) | fdisk "$DISK"
