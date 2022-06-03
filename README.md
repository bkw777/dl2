# dlplus
DeskLink+ is a Tandy Portable Disk Drive emulator or "TPDD Server" implimented in C.  
2022 [GGLabs](https://gglabs.us/) has added support for TS-DOS subdirectories!  
[hacky extra options](ref/advanced_options.txt)  
[Serial Cable](http://tandy.wiki/Model_T_Serial_Cable)

Docs from the past versions of this program. They don't exactly match this version any more.  
[README.txt](README.txt) from dlplus by John R. Hogerhuis  
[dl.do](dl.do) from dl 1.0-1.3 the original "DeskLink for \*nix) by Steven Hurd
<!-- [Original source](http://bitchin100.com/files/linux/dlplus.zip) -->

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

## list all available TPDD client installers, and then bootstrap one of them
```
dl -l
dl -b TS-DOS.100
```

## bootstrap a [REXCPM](http://bitchin100.com/wiki/index.php?title=REXCPM)
```
unzip REXCPMV21_b18.ZIP
dl -b ./rxcini.DO ;dl -u
```
## fun
The "ROOT  " and "PARENT" labels are not hard coded in TS-DOS. You can set them to other things. Sadly, this does not extend as far as being able to use ".." for "PARENT". TS-DOS thinks it's an invalid filename (even though it DISPLAYS it in the file list just fine. If it would just go ahead and send the command to "open" it, it would work.) However, plenty of other things that are all better than "ROOT  " and "PARENT" do work.
```
ROOT_LABEL=/ PARENT_LABEL=^ dl
```
or you can confuse someone...  
```
ROOT_LABEL='C:\' PARENT_LABEL='' dl -u
```
