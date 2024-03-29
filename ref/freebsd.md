# Notes for FreeBSD

## Building

* Use gmake instead of make

## TTY Permissions

* Create the file:  
```/usr/local/etc/devd/usbserial.conf```

```
notify 100 {
	match "system"		"DEVFS";
	match "subsystem"	"CDEV";
	match "type"		"CREATE";
	match "cdev"		"ttyU[0-9]+";
	action "chgrp dialer /dev/$cdev ;chmod g+rw /dev/$cdev";
};
```

* Restart the devd service:  
```# service devd restart```

* Add yourself to the dialer group (replace "bkw" with your login name):  
```# pw group mod dialer -m bkw```

* Re-plug the usb-serial adapter.

## Behavior Quirks

Hangs at opening the client tty if the client machine is not connected yet.  
Proceeds as soon as the client machine is connected.
