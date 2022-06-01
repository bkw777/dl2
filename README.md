# dlplus
DeskLink+ is a Tandy Portable Disk Drive emulator or "TPDD Server" implimented in C.  
2022 [GGLabs](https://gglabs.us/) has added support for TS-DOS subdirectories.

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

## manual
```
dl -h
```

## run the TPDD server, serving files from the current directory
```
dl
```

## list all available TPDD client installers, and then bootstrap one of them.
```
dl -l
dl -b TS-DOS.100
```

## bootstrap a [REXCPM](http://bitchin100.com/wiki/index.php?title=REXCPM)
```
unzip REXCPMV21_b18.ZIP
dl -b ./rxcini.DO ;dl -u
```

## Advanced
See [ref/advanced.options.txt]()
