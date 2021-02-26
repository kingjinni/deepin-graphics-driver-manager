#!/bin/bash
systemctl stop lightdm

# run this command again here to avoid disable overlayroot in service not work

. /usr/lib/deepin-graphics-driver-manager/common.sh

overlayroot_disable

# write to tty
journalctl -f -u dgradvrmgr-gltest.service | sed 's/$/\r/g' > /dev/tty1 2>&1 &

# file "/tmp/deepin-prime-gltest" is generated by install prime script 
# cause prime needs some special ENV to work so here to start the special script for gltest
# otherwise start gltest directly
if [[ -e "/tmp/deepin-prime-gltest" ]]; then
    /usr/bin/xinit /tmp/deepin-prime-gltest
else
    /usr/bin/xinit /usr/lib/deepin-graphics-driver-manager/gltest
fi

killall Xorg

nvidia_blacklist_recovery

# run this command again here to avoid disable overlayroot in service not work
# same as above
overlayroot_disable

reboot
