#!/bin/bash
echo Start by moving into the right directories.
cd /home/pi
mkdir rtkernelsetup
cd rtkernelsetup

echo Obtaining files relevant to the Real Time Extensions.
wget -O linux-headers-5.15.65-llat-v8+_5.15.65-1_arm64.deb -L https://github.com/kdoren/linux/releases/download/rpi_5.15.65/linux-headers-5.15.65-llat-v8+_5.15.65-1_arm64.deb
wget -O linux-image-5.15.65-llat-v8+_5.15.65-1_arm64.deb -L https://github.com/kdoren/linux/releases/download/rpi_5.15.65/linux-image-5.15.65-llat-v8+_5.15.65-1_arm64.deb
wget -O rpi_5.15.65/linux-libc-dev_5.15.65-1_arm64.deb -L https://github.com/kdoren/linux/releases/download/rpi_5.15.65/linux-libc-dev_5.15.65-1_arm64.deb

#wget -O linux-headers-5.15.40-rt43-v7l+_5.15.40-1_armhf.deb -L https://msoe.box.com/shared/static/w4cxpwcv2o5wot80y7ad5wfhqih0rb8h
#wget -O linux-image-5.15.40-rt43-v7l+_5.15.40-1_armhf.deb   -L https://msoe.box.com/shared/static/6f9mwi827az5j0xbinf9yiq8t89tv9sn
#wget -O linux-libc-dev_5.15.40-1_armhf.deb -L https://msoe.box.com/shared/static/kantoak98zd038mac96yopc3yqr4p04o
#wget -O linux-image-5.15.40-llat-v7l+_5.15.40-1_armhf.deb -L https://msoe.box.com/shared/static/tmkbf5al7dy5uwgs44cxxsxkepl2zokx
#wget -O linux-headers-5.15.40-llat-v7l+_5.15.40-1_armhf -L https://msoe.box.com/shared/static/9xay7u7a14x4lp05r8t9b1tzz0mnsh2g

echo Installing the cpufrequtilities which allow us to custimize how the CPU runs.
sudo apt-get install cpufrequtils

echo Install the kernel.
apt install ./linux-image-5.15.65-llat-v8+_5.15.65-1_arm64.deb
KERN=5.15.65-llat-v8+    # 64-bit kernel

echo make the kernel the current operating kernel 
mkdir -p /boot/$KERN/o/
cp -d /usr/lib/linux-image-$KERN/overlays/* /boot/$KERN/o/
cp -dr /usr/lib/linux-image-$KERN/* /boot/$KERN/
[[ -d /usr/lib/linux-image-$KERN/broadcom ]] && cp -d /usr/lib/linux-image-$KERN/broadcom/* /boot/$KERN/
touch /boot/$KERN/o/README
mv /boot/vmlinuz-$KERN /boot/$KERN/
mv /boot/initrd.img-$KERN /boot/$KERN/
mv /boot/System.map-$KERN /boot/$KERN/
cp /boot/config-$KERN /boot/$KERN/
cat >> /boot/config.txt << EOF

[all]
kernel=vmlinuz-$KERN
# initramfs initrd.img-$KERN
os_prefix=$KERN/
overlay_prefix=o/$(if [[ "$KERN" =~ 'v8' ]]; then echo -e "\narm_64bit=1"; fi)
[all]
EOF

echo Finished.
