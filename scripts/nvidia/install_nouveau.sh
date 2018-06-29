#!/bin/bash

if [ "$(id -u)" -ne "0" ];then
    echo "Need root privileges."
    exit 1
fi

export DEBIAN_FRONTEND=noninteractive
POSTOS=`cat /proc/mounts | awk '{if ($2 == "/media/root-ro") print $1}'`

systemctl stop lightdm

if [ $1 == "post" ];then
    echo "Sync driver into disk $POSTOS ...... "

    find /media/root-rw/overlay/ -size 0 | xargs rm -rf
    mount -o remount,rw $POSTOS /media/root-ro
    rsync -avz --progress /media/root-rw/overlay/* /media/root-ro/
    sync

    echo "Sync driver into disk ...... done"
else
    apt-get --reinstall -y --allow-downgrades install \
        libgl1-mesa-glx \
        xserver-xorg-input-all \
        xserver-xorg-core \
        xserver-xorg-video-nouveau

    # repair glx alternative for nouveau
    apt-get -y install --reinstall update-glx glx-diversions nvidia-installer-cleanup glx-alternative-mesa
    apt-get -y install --reinstall libgl1-mesa-glx libgl1-mesa-glx:i386
    apt-get -y install --reinstall libglx-mesa0 libglx-mesa0:i386
    apt-get -y install --reinstall glx-diversions

    echo "Loading kernel modules......"
    modprobe nouveau
fi
