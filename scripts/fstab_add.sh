```bash
#!/bin/bash

# Генерация записей fstab для /dev/sdb5 ... /dev/sdb54

for i in $(seq 5 54); do
    mp="/mnt/data/sdb$i"
    uuid=$(/sbin/blkid -s UUID -o value /dev/sdb$i)

    if [ -n "$uuid" ]; then
        mkdir -p "$mp"
        echo "UUID=$uuid $mp ext4 defaults,nofail 0 2"
    else
        echo "ERROR: no UUID for /dev/sdb$i" >&2
    fi
done
