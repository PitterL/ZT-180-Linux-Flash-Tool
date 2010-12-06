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
	unsigned char * data;

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

	data = malloc(512); // FIXME how much?
	if (!data)
		die("can not allocate memory");

// STAGE1: request device --> bNumConfiguration=1
	ret = libusb_control_transfer(
		hdl,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
		LIBUSB_REQUEST_GET_DESCRIPTOR,
		0x0100, // wValue LIBUSB_DT_DEVICE 
		0x0,    // wIndex
		data,
		LIBUSB_DT_DEVICE_SIZE,
		2000
	);
	if( ret != LIBUSB_DT_DEVICE_SIZE ){
		if( ret == LIBUSB_ERROR_TIMEOUT ) puts("Timeout");
		die("error reading from usb (stage1)\n");
	}

// STAGE2: request configuration --> wTotalLenght=32
	ret = libusb_control_transfer(
		hdl,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
		LIBUSB_REQUEST_GET_DESCRIPTOR,
		0x0200, // wValue LIBUSB_DT_CONFIG
		0x0,    // wIndex
		data,
		LIBUSB_DT_CONFIG_SIZE,
		2000
	);
	if( ret != LIBUSB_DT_CONFIG_SIZE ){
		if( ret == LIBUSB_ERROR_TIMEOUT ) puts("Timeout");
		die("error reading from usb (stage2)\n");
	}

// STAGE3: request full configuration --> wTotalLenght=32
	ret = libusb_control_transfer(
		hdl,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
		LIBUSB_REQUEST_GET_DESCRIPTOR,
		0x0200, // wValue LIBUSB_DT_CONFIG
		0x0,    // wIndex
		data,
		32,
		2000
	);
	if( ret != 32 ){
		if( ret == LIBUSB_ERROR_TIMEOUT ) puts("Timeout");
		die("error reading from usb (stage3)\n");
	}

// STAGE4: request device --> bNumConfiguration=1 (as STAGE1)
	ret = libusb_control_transfer(
		hdl,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
		LIBUSB_REQUEST_GET_DESCRIPTOR,
		0x0100, // wValue LIBUSB_DT_DEVICE 
		0x0,    // wIndex
		data,
		64,     //magic size
		2000
	);
	if( ret != LIBUSB_DT_DEVICE_SIZE ){
		if( ret == LIBUSB_ERROR_TIMEOUT ) puts("Timeout");
		die("error reading from usb (stage4)\n");
	}

// STAGE5: request device --> bNumConfiguration=1 (as STAGE1)
	ret = libusb_control_transfer(
		hdl,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
		LIBUSB_REQUEST_GET_DESCRIPTOR,
		0x0100, // wValue LIBUSB_DT_DEVICE 
		0x0,    // wIndex
		data,
		LIBUSB_DT_DEVICE_SIZE,
		2000
	);
	if( ret != LIBUSB_DT_DEVICE_SIZE ){
		if( ret == LIBUSB_ERROR_TIMEOUT ) puts("Timeout");
		die("error reading from usb (stage5)\n");
	}

// STAGE6: request configuration --> wTotalLenght=32 (as STAGE2)
	ret = libusb_control_transfer(
		hdl,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
		LIBUSB_REQUEST_GET_DESCRIPTOR,
		0x0200, // wValue LIBUSB_DT_CONFIG
		0x0,    // wIndex
		data,
		LIBUSB_DT_CONFIG_SIZE,
		2000
	);
	if( ret != LIBUSB_DT_CONFIG_SIZE ){
		if( ret == LIBUSB_ERROR_TIMEOUT ) puts("Timeout");
		die("error reading from usb (stage6)\n");
	}

// STAGE7: request configuration --> wTotalLenght=32 (as STAGE2)
	ret = libusb_control_transfer(
		hdl,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
		LIBUSB_REQUEST_GET_DESCRIPTOR,
		0x0200, // wValue LIBUSB_DT_CONFIG
		0x0,    // wIndex
		data,
		255,    // magic size
		2000
	);
	if( ret != 32 ){
		if( ret == LIBUSB_ERROR_TIMEOUT ) puts("Timeout");
		die("error reading from usb (stage7)\n");
	}

// STAGE8: request device --> bNumConfiguration=1 (as STAGE1)
	ret = libusb_control_transfer(
		hdl,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
		LIBUSB_REQUEST_GET_DESCRIPTOR,
		0x0100, // wValue LIBUSB_DT_DEVICE 
		0x0,    // wIndex
		data,
		LIBUSB_DT_DEVICE_SIZE,
		2000
	);
	if( ret != LIBUSB_DT_DEVICE_SIZE ){
		if( ret == LIBUSB_ERROR_TIMEOUT ) puts("Timeout");
		die("error reading from usb (stage8)\n");
	}

// STAGE9: request full configuration --> wTotalLenght=32 (as STAGE3)
	ret = libusb_control_transfer(
		hdl,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
		LIBUSB_REQUEST_GET_DESCRIPTOR,
		0x0200, // wValue LIBUSB_DT_CONFIG
		0x0,    // wIndex
		data,
		521,    // magic size
		2000
	);
	if( ret != 32 ){
		if( ret == LIBUSB_ERROR_TIMEOUT ) puts("Timeout");
		die("error reading from usb (stage9)\n");
	}

	free(data);

	// reading and checking state
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
