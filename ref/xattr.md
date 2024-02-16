## Storing the ATTR byte from the tpdd client in local xattr

Example:  
 Create a small text file `T0.DO`  
 Save `T0.DO` as `T1.DO` without specifying any attr  
 Save `T0.DO` as `T2.DO` with attr `X`  
 Save `T0.DO` as `T3.DO` with attr `d`  
 Save `T0.DO` as `T4.DO` with attr `' '`  
 List the directory

```
$ printf 'test\r\n' >T0.DO
$ pdd ttyUSB1 "save T0.DO T1.DO;save T0.DO T2.DO X;save T0.DO T3.DO d;save T0.DO T4.DO ' ';ls"
Saving TPDD:T1.DO (F)
[########################################] 100% (6/6 bytes)
Saving TPDD:T2.DO (X)
[########################################] 100% (6/6 bytes)
Saving TPDD:T3.DO (d)
[########################################] 100% (6/6 bytes)
Saving TPDD:T4.DO ( )
[########################################] 100% (6/6 bytes)
--------  Directory Listing  --------
T1.DO                    | F |      6
T2.DO                    | X |      6
T3.DO                    | d |      6
T4.DO                    |   |      6
-------------------------------------
102400 bytes free
$ 
```

The directory listing shows that the attr values were written to "disk" and then read back out.

You can do the same process with a real drive.

As with a real drive, both the filename and attr must match in order to access a file.  
For instance to delete one of these files, you have to specify the attr the same way:  
`$ pdd "rm T3.DO d"`


## misc references
Interesting but not used because it's c++ not plain c.
https://github.com/edenzik/pxattr


https://eclecticlight.co/2019/10/01/how-to-preserve-metadata-stored-in-a-custom-extended-attribute/  
on macos suffix xattr name with "#S"

on linux prefix xattr name with "user."

Base xattr name is "pdd.attr"

On macos it becomes "pdd.attr#S"
On linux it becomes "user.pdd.attr"

On macos might need to be changed to something like "org.dl2.attr#S"
Ideally would like to keep the name generic and not tied to dl2 by name, so that other tpdd server and client software could all use the same xattr name.
