/*
 * This is a component for flashing Zenithik ZT-180 slate.
 * (c) 2010 Yuri Kozlov <yuray@komyakino.ru>
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <libusb-1.0/libusb.h>

#define VENDOR_ID	0x5345
#define PRODUCT_ID	0x1234
#define IN_EP		0x81

void die(const char * str)
{
	printf("Error: %s\n", str);
	fflush(stdout);
	exit(1);
}

int main(int argc, char **argv)
{
	struct libusb_device_handle *hdl;
	u_int32_t state, res = 0 ;
	int transferred = 0 ;
	int ret = 0 ;

	/* Check for correct usage */
	if( argc != 2 ){
		printf("Usage:   sudo usbstate checking_code\n");
		printf("Example: sudo usbstate 1\n");
		exit(2);
	}

	if( libusb_init( NULL ))
		die("can not init usb library");

	libusb_set_debug( NULL, 3 ); //max verbose

	hdl = libusb_open_device_with_vid_pid( NULL, VENDOR_ID, PRODUCT_ID );
	if( hdl == NULL )
		die("cannot find zt-180 device in bootloader mode");

	if( libusb_claim_interface( hdl, 0 ) != 0 )
		die("unable to claim usb interface");

	/* Getting magic */
	state = atof( argv[1] );

	ret = libusb_bulk_transfer( hdl, IN_EP, (unsigned char *)&res, 4, &transferred, 10000 );
	printf("Return code: %i\n", ret);
	printf("Transferred: %i\n", transferred);
	printf("Data: %i\n", res);

	if( ret != 0 ){
		if( ret == LIBUSB_ERROR_TIMEOUT ) puts("Timeout");
		die("error reading from usb\n");
	}
	if( 4 != transferred )
		die("not all data reading from usb\n");

	if( libusb_release_interface( hdl, 0 ) != 0 )
		die("unable to release usb interface");
	libusb_close( hdl );
	libusb_exit( NULL );
	exit(0);
}
