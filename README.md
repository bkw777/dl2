# dlplus
DeskLink+ is a Tandy Portable Disk Drive emulator or "TPDD Server" implimented in C.

[Original README](README.txt)

[Documentation for DeskLink](dl.do)

Original source: <http://bitchin100.com/files/linux/dlplus.zip>

Serial Cable: <http://tandy.wiki/Model_100_102_200_600_Serial_Cable>

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

## bootstrap a TPDD client (TEENY) onto the portable
```
dl -b
```

## news
20191228 Added Model 200 support<br>
To bootstrap TEENY onto a Tandy 200, run:<br>
```
dl -b=200
```

20191226 Added "bootstrap" -b option<br>
To bootstrap TEENY onto a Tandy 100 or 102, just run:<br>
```
dl -b
```
