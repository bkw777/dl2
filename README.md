# dlplus
DeskLink+ is a Tandy Portable Disk Drive emulator or "TPDD Server" implimented in C.

[Original README](README.txt)

[Documentation for the original DeskLink](dl.do) (No longer exactly matches this program)

[Original source](http://bitchin100.com/files/linux/dlplus.zip)

[Serial Cable](http://tandy.wiki/Model_T_Serial_Cable)

## install
```
make clean all && sudo make install
```

## uninstall
```
sudo make uninstall
```

## run the TPDD server
```
dl
```

## bootstrap the default TPDD client (TEENY) onto the portable
```
dl -b
```

## list all available TPDD client installers, and then bootstrap one of them (TS-DOS for Model 100)
```
dl -h
dl -b=TS-DOS.100
```

## news
### 20200830 Added TS-DOS loaders provided by Kurt McCullum
 TS-DOS.100 TS-DOS.200 TS-DOS.NEC

### 20191231 Expanded bootstrap to support multiple client apps and machine models.
Included bootstrap files:<br>
 TEENY.100 TEENY.200 TEENY.NEC<br>
 DSKMGR.100 DSKMGR.200 DSKMGR.K85 DSKMGR.M10

The "-b" syntax has changed. The new syntax is:<br>
```
dl -b=file
```
...where "file" is one of the above bootstrap files.<br>
"-h" includes a listing of all available included bootstrap files<br>
"-b" with no "=filename" defaults to TEENY.100<br>
"-b" also accepts a full or relative path to any arbitrary file, not limited to a TPDD client installer.<br>
Examples:<br>
```
dl -b
dl -b=TEENY.NEC
dl -b=./Downloads/XPTERM.100
dl -b=~/Documents/TRS-80/M100SIG/Lib-03-TELCOM/XMDPW5.100
```

No loader-specific code in dl.c any more. Each bootstrap file has it's own external files for the pre-install and post-install text.

See /usr/local/share/doc/dl for the full manuals for TEENY and DSKMGR.

### 20191228 Added Model 200 support
To bootstrap TEENY onto a Tandy 200, run:<br>
```
dl -b=200
```

### 20191226 Added "bootstrap" -b option
To bootstrap TEENY onto a Tandy 100 or 102, just run:<br>
```
dl -b
```
