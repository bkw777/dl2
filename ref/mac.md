# Notes for Macos / OSX

## TTY Device
There is no predictable likely default tty device name like `ttyUSB0` on Linux, so there can be no default built-in, so you must always supply the tty device name via the commandline or the environment variable CLIENT_TTY.

Each serial tty device has two interfaces, a `/dev/tty.foo` and a `/dev/cu.foo`  
Either one usually works, but you always want to use the cu.\* version for this simply because it guarantees the process has exclusive access to the device while it's open (No other process can corrupt the data).

There is usually at least one bluetooth device that shows up as a serial device, so you need to ignore those.

`$ ls /dev/cu.* |grep -v Bluetooth`

Or just start writing the command and use tab-completion to show the cu.* devices and pick one:

```
$ dl -v -u /dev/cu.
cu.Bluetooth-Incoming-Port cu.usbserial-AL03RAXP
$ dl -v -u /dev/cu.usbserial-AL03RAXP
...
```
