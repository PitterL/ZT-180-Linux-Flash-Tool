/* 
 * Based on qt2410_boot_usb - Ram Loader for Armzone QT2410 Devel Boards
 * (C) 2006 by Harald Welte <hwelte@hmw-consulting.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 
 *  as published by the Free Software Foundation
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * This is a component for flashing Zenithik ZT-180 slate.
 * (c) 2010 Yuri Kozlov <yuray@komyakino.ru>
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>

#include <libusb-1.0/libusb.h>


#define VENDOR_ID	0x5345
#define PRODUCT_ID	0x1234
#define OUT_EP		0x2

// libusb_get_max_packet_size (libusb_device *dev, unsigned char endpoint)
//Returns:   >0
//    the wMaxPacketSize value 
#define CHUNK_SIZE 512

static struct libusb_device_handle *hdl;

static void die(const char * str)
{
	printf("Error: %s\n", str);
	fflush(stdout);
	exit(1);
}
static u_int16_t calc_csum(const unsigned char *data, u_int32_t len)
{
	u_int16_t csum = 0;
	int j;

	for (j = 0; j < len; j ++) {
		csum += data[j];
	}

	return csum;
}

static int push_file(u_int32_t magic1, u_int32_t magic2, u_int32_t addr, void *data, u_int32_t len)
{
	int ret = 0;
	unsigned char *buf, *cur;
	int transferred = 0;
	u_int16_t csum = calc_csum(data, len);

	/* 4 bytes address, 4 bytes length, 4 bytes magic1, 4 bytes magic2, data, 2 bytes csum */
	u_int32_t len_total = len + 18;
	printf("csum = 0x%4x\n", csum);

	buf = malloc(len_total);
	if (!buf)
		return -ENOMEM;

	/* FIXME: endian safeness !!! */
	buf[0] = addr & 0xff;
	buf[1] = (addr >> 8) & 0xff;
	buf[2] = (addr >> 16) & 0xff;
	buf[3] = (addr >> 24) & 0xff;

	buf[4] = len_total & 0xff;
	buf[5] = (len_total >> 8) & 0xff;
	buf[6] = (len_total >> 16) & 0xff;
	buf[7] = (len_total >> 24) & 0xff;

	buf[8] = magic1 & 0xff;
	buf[9] = (magic1 >> 8) & 0xff;
	buf[10] = (magic1 >> 16) & 0xff;
	buf[11] = (magic1 >> 24) & 0xff;

	buf[12] = magic2 & 0xff;
	buf[13] = (magic2 >> 8) & 0xff;
	buf[14] = (magic2 >> 16) & 0xff;
	buf[15] = (magic2 >> 24) & 0xff;

	memcpy(buf+16, data, len);

	buf[len+16] = csum & 0xff;
	buf[len+17] = (csum >> 8) & 0xff;

	printf("send_file: addr = 0x%08x, len = 0x%08x\n", addr, len);

	for (cur = buf; cur < buf+len_total; cur += CHUNK_SIZE) {
		transferred = 0;
		int remain = (buf + len_total) - cur;
		if (remain > CHUNK_SIZE)
			remain = CHUNK_SIZE;
repeat_transfer:
		ret = libusb_bulk_transfer( hdl, OUT_EP, cur+transferred, remain-transferred, &transferred, 2000);
		if( ret == LIBUSB_ERROR_TIMEOUT){
			printf("T"); fflush(stdout);
			goto repeat_transfer;
		} else if( ret != 0 ){
			printf("error writing to usb\n");
			printf("Return code: %i\n", ret);
			printf("Transferred: %i\n", transferred);
			break;
		} else if( remain != transferred ){
			printf("not all data transferred to usb\n");
			ret = -1;
			break;
		}
		printf(".");
	}

	printf("\n");
	free(buf);

	return ret;
}

int main(int argc, char **argv)
{
	char *filename;
	void *prog;
	struct stat st;
	int fd;
	u_int32_t ram_base, magic1, magic2;

	libusb_device **devs;

	if( argc != 5 ){
		printf("Usage:   sudo usbpush filename MAGIC1   MAGIC2 RAM_base\n");
		printf("Example: sudo usbpush u-boot   0x010200 0      0x40200000\n");
		exit(2);
	}

	if( libusb_init( NULL ))
		die("can not init usb library");

	libusb_set_debug( NULL, 3 ); //max verbose

	if( libusb_get_device_list(NULL, &devs) < 0 )
		die("can not get device list");

	hdl = libusb_open_device_with_vid_pid( NULL, VENDOR_ID, PRODUCT_ID );
	if( hdl == NULL )
		die("cannot find zt-180 device in bootloader mode");

	libusb_free_device_list(devs, 1);

	if( libusb_claim_interface( hdl, 0 ) != 0 )
		die("unable to claim usb interface");

	/* Open file */
	filename = argv[1];
	fd = open(filename, O_RDONLY);
	if(fd < 0)
		die("Unable to open file");

	if (fstat(fd, &st) < 0)
		die("Error to access file");

	/* Set magic numbers */
	magic1 = atof(argv[2]);
	magic2 = atof(argv[3]);
	/* Set RAM base */
	ram_base = atof(argv[4]);
	/* mmap kernel image passed as parameter */
	prog = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if( !prog )
		die("mmap error");

	if( push_file( magic1, magic2, ram_base, prog, st.st_size ) < 0)
		die("Error downloading program\n");

	if( libusb_release_interface( hdl, 0 ) != 0 )
		die("unable to release usb interface");
	libusb_close( hdl );
	libusb_exit( NULL );
	exit(0);
}
