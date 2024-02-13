#Using Sardine with dl2 and a disk image of the dictionary disk

Reference: [Manual for Ultimate ROM II](http://www.club100.org/library/librom.html):  

One way to use Sardine is to let Ultimate ROM II load & unload the program from disk into ram on the fly instead of installing permanently in ram. Sardine uses raw sector access commands to read a special dictionary data disk.  
For this to work, UR2 has to be able to load `SAR100.CO` from a normal filesystem disk using normal file/filesystem access, and then `SAR100.CO` needs to be able to use TPDD1 FDC-mode commands to read raw sectors from the special dictionary data disk.  

dl2 supports this by both the **magic files** and **disk image file** features.  
`SAR100.CO` is one of the bundled "magic files", so when UR2 tries to load a file by that name, it always works even if there is no such file in the share path.  
An image of the dictionary disk is also bundled with dl2, and the **-i file.pdd1** option allows SAR100.CO to use raw sector-access commands on the disk image.


1: Run dl with the following commandline arguments,
```
$ dl -vun -i Sardine_American_English.pdd1
```

This tells dl2 to emulate a TPDD1, disable some TPDD2 features and TS-DOS directory support which would confuse `SAR100.CO`, and use the Sardine American English dictionary disk image file for any sector-access commands.  
Both `SAR100.CO` and `Sardine_American_English.pdd1` are bundled with dl2, installed in /usr/local/lib/dl, so you don't have to do anything for SAR100.CO, and for the disk image you don't have to specify the full path.  

2: Enter the UR-2 menu.  
Notice the "SARDIN" entry with the word "OFF" under it.  
Hit enter on SARDIN.  
If you get a prompt about HIMEM, answer Y.  
This loads SAR100.CO into ram.
Now notice the SARDIN entry changed from "OFF" to "ON" under it.

3: Enter T-Word and start a new document and type some text.  

4: Press GRPH+F to invoke Sardine to spell-check the document.  
This will invoke SAR100.CO, which will try to use TPDD1 FDC-mode sector access commands, wich dl2 will respond to with data from the .pdd1 file.  
