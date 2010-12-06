#!/usr/bin/env bash
#
# This program flash firmware on Zenithik ZT-180.
#
# I AM NOT RESPONSIBLE IF YOU DAMAGE YOUR DEVICE,
# FLASH AT YOUR OWN RISK IN.
#

UBOOT=../rom/u-boot.img
KERNEL=../rom/zImage_1024x600
SYSTEM=../rom/system.img
USERDATA=../rom/userdata.img

USBPUSH=./bin/usbpush
USBSTATE=./bin/usbstate
USBREAD=./bin/usbread
#-----------------------------------------------
function die {
	echo $1
	exit 1
}

[ -f ${USBSTATE} ] || die "usbstate not found"
[ -f ${USBPUSH} ]  || die "usbpush not found"
echo "Searching files..."
[ -f ${UBOOT} ]    || die "u-boot not found"
[ -f ${KERNEL} ]   || die "kernel not found"
[ -f ${SYSTEM} ]   || die "system not found"
[ -f ${USERDATA} ] || die "userdata not found"
echo "Checking device state..."
${USBSTATE} 1 || die ""
echo "Running u-boot..."
${USBPUSH} ${UBOOT} 0 0x88 0xffffffff
echo "Waiting for device reconnected..."
sleep 3
echo "Checking device state..."
${USBSTATE} 0x1000000 || die ""
#exit 0
echo "Flashing u-boot..."
${USBPUSH} ${UBOOT}    0x00010200 0 0x40200000
echo "Flashing kernel..."
${USBPUSH} ${KERNEL}   0x00010300 0 0x40200000
echo "Flashing system..."
${USBPUSH} ${SYSTEM}   0x00010400 0 0x40200000
echo "Flashing userdata..."
${USBPUSH} ${USERDATA} 0x00000500 0 0x40200000
echo "Checking device state..."
${USBREAD} 0x6000000f || die ""
echo "Done"
