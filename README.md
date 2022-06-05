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

## run the TPDD server, verbose, upcase, serving files from the current directory
```
dl -vu
```

## list all available TPDD client installers, and then bootstrap one of them
```
dl -l
dl -vb TS-DOS.100
```

## bootstrap a [REXCPM](http://bitchin100.com/wiki/index.php?title=REXCPM)
```
unzip REXCPMV21_b19.ZIP
dl -vb ./rxcini.DO ;dl -vu
```
## fun
The "ROOT  " and "PARENT" labels are not hard coded in TS-DOS. You can set them to other things. Sadly, this does not extend as far as being able to use ".." for "PARENT". TS-DOS thinks it's an invalid filename (even though it DISPLAYS it in the file list just fine. If it would just go ahead and send the command to "open" it, it would work.) However, plenty of other things that are all better than "ROOT  " and "PARENT" do work.
```
ROOT_LABEL=/ PARENT_LABEL=^ dl
ROOT_LABEL='-root-' PARENT_LABEL='-back-' dl
ROOT_LABEL='0:' PARENT_LABEL='^:' dl
or you can confuse someone...  
ROOT_LABEL='C:\' PARENT_LABEL='UP:' dl
```
## UR-II
Ultimate ROM II ([docs](http://www.club100.org/library/libdoc.html)) ([roms](https://bitchin100.com/wiki/index.php?title=REXsharp#Option_ROM_Images_for_Download)) has a feature where it can load a RAM version of TS-DOS from disk on-the-fly.  

This allows you to keep the TS-DOS executable on the disk instead of in ram, and it is loaded and then discarded on-demand by selecting the TS-DOS menu entry from inside UR2. You don't have to install TS-DOS via the bootstrapper if you have an UR2 option rom installed.  

For that feature to work, a file named DOS100.CO, DOS200.CO, or DOSNEC.CO (depending on your model of computer) must exist on the "disk".  

UR2 doesn't know anything about directories, and just tries to load a file named "DOS___.CO".  

If you had previously loaded TS-DOS and used it to navigate into a subdirectory, and that subdirectory didn't also have a copy of DOS___.CO in it, then UR2 would normally fail to load TS-DOS after that, until you restarted the TPDD server to make it go back to the root share dir.  

This version of dlplus has a special feature to support UR2, so that UR2 may still load DOS100.CO, DOS200.CO, or DOSNEC.CO no matter what subdirectory the server has been navigated to, as long as there is a copy in the root shared directory.  

The [clients/](clients/) directory includes copies of [DOS100.CO](clients/ts-dos/DOS100.CO), [DOS200.CO](clients/ts-dos/DOS200.CO), and [DOSNEC.CO](clients/ts-dos/DOSNEC.CO)  
These are also installed to ```/usr/local/lib/dlplus/clients/ts-dos/``` by ```sudo make install```  
