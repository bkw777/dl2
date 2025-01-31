## Storing the ATTR byte from the tpdd client in local xattr

If compiled with -DUSE_XATTR  
`$ make clean all CXXFLAGS=-DUSE_XATTR && sudo make install`  
then the ATTR field of a TPDD directory entry is stored in and retrieved from an extended attribute named "pdd.attr", instead of just hard-coded with 'F' at all times.

On Linux the xattr name is prefixed with "user." to become "user.pdd.attr"  
On Mac the xattr name is suffixed with "#S" to become "pdd.attr#S"  
On FreeBSD the name is unchanged and the namespace used is EXTATTR_NAMESPACE_USER  
The xattr name is not something like "com.dl2.attr" because it is intended to be generic and not tied just to dl2, so other tpdd clients and servers might use the same name and the files would be compatible across different software.  

The attr field is a single byte, and may contain any value, 0x00 to 0xFF.  
The field is normally never shown to users because TRS-80 Model 100 software doesn't use the field and just hard-codes 'F' in that field behind the scenes.  
And because of that, most drive emulators also ignore the field except to just hard-code the same 'F' there at all times.  
But a real drive is actually recording and checking that data with every file access, and other software could use it.

So the idea is to more accurately emulate a real drive by actually recording the attr byte supplied by the client, and actually checking it later.  
If the client is creating a new file, save the given attr value as part of the local file in the fom of an xattr field.  
If looking for an existing file, read the attr for each local file from the xattr, and compare with the attr given by the client along with the filename.  
Only use the default attr value when the xattr doesn't exist.  

Example using [pdd.sh](https://github.com/bkw777/pdd.sh)

First the help reference for the load and save commands.  
You don't normally use the attr argument but there is one.

```
PDD(opr:6.2,F)> help save

 save src_filename [dest_filename] [attr]
    Copy a file from local to disk
PDD(opr:6.2,F)> help load

 load src_filename [dest_filename] [attr]
    Copy a file from disk to local

PDD(opr:6.2,F)> help delete

 rm | del | delete filename [attr]
    Delete filename [attr] from disk

PDD(opr:6.2,F)> 

```

The parameters are simple position dependant, so in order to use the 3rd argument you have to supply the 2nd.

Normally to save a file without also renaming it along the way, you only need to say "save filename"

But if you want to override the default attr and specify an arbitrary attr like X,  
you need to say `save filename filename X`.  
Or you can give "" or '' for the destination filename so: `save filename "" X`  
and in that case it will use the source filename without having to type it out twice.

So to show the attr actually being stored and retrieved:
  
 Create a small text local file `T0.DO`  
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

As with a real drive, both the filename and attr must match in order to access a file.  
filename "foo" with attr "a"  
and  
filename "foo" with attr "b"  
are two different files, basically the same as if one were named "fooa" and one were named "foob"

For instance to load or delete one of these files that has a non-default attr, you have to specify both the filename and the attr :  
`PDD(opr:6.2,F)> rm T3.DO d`

This is working on Linux, Macos, & FreeBSD.

For any platform that isn't supported, or on any filesystem that doesn't have extended attributes, or any new local files that weren't created by a tpdd client, it will just transparently work the old way. Attr will be 'F' or whatever the "-a" commandline flag or the ATTR environment variable says.

## notes

To see xattrs on files:
`getfattr -d -- NAME.BA`  
`getfattr -d -- *`

cp, tar, rsync, etc don't preserve xattrs with files by default.  

They all have options for it, so you can preserve the xattrs in copies, it's just not the default behavior.

Even text editors may need extra config so they don't unlink & create a new file on each save.

In reality, you will never notice or care because everything that you ever want to use with a TPDD emulator, they all happen to always hard-code attr=F at all times, and that is the same thing dl2 will do any time a file has no xattr.  
This is the same as all tpdd emulators which have all been hard-coding 'F' all along anyway since the beginning in 1984.

## misc references
Wrapper to provide a single interface to the different platforms xattr interfaces.  
Interesting but not used because it's c++ not plain c.  
Maybe it can be ported to plain c.  
https://github.com/edenzik/pxattr

Macos info  
https://eclecticlight.co/2019/10/01/how-to-preserve-metadata-stored-in-a-custom-extended-attribute/

