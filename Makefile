#!/usr/bin/make
LDFLAGS=-lusb-1.0
CFLAGS=-Wall
SRCDIR=src
BINDIR=bin

all: 
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCDIR)/usbpush.c -o $(BINDIR)/usbpush
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCDIR)/usbstate.c -o $(BINDIR)/usbstate
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCDIR)/usbread.c -o $(BINDIR)/usbread

clean:
	-rm -f $(BINDIR)/usbpush $(BINDIR)/usbstate $(BINDIR)/usbread

.PHONEY: all clean
