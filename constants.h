#ifndef PDD_CONSTANTS_H
#define PDD_CONSTANTS_H

// TPDD drive firmware/protocol constants

// TPDD request block formats
#define REQ_DIRENT        0x00 // (add 0x40 for TPDD2 bank 1)
#define REQ_OPEN          0x01 // (add 0x40 for TPDD2 bank 1)
#define REQ_CLOSE         0x02 // (add 0x40 for TPDD2 bank 1)
#define REQ_READ          0x03 // (add 0x40 for TPDD2 bank 1)
#define REQ_WRITE         0x04 // (add 0x40 for TPDD2 bank 1)
#define REQ_DELETE        0x05 // (add 0x40 for TPDD2 bank 1)
#define REQ_FORMAT        0x06
#define REQ_STATUS        0x07 // (add 0x40 for undocumented synonym on TPDD2)
#define REQ_FDC           0x08 // TPDD1
#ifdef NADSBOX_EXTENSIONS
	#define REQ_NADSBOX_SEEK       0x09
	#define REQ_NADSBOX_TELL       0x0A
	#define REQ_NADSBOX_SET_EXT    0x0B
#endif
#define REQ_CONDITION     0x0C // TPDD2
#define REQ_RENAME        0x0D // TPDD2 (add 0x40 for bank 1)
#ifdef NADSBOX_EXTENSIONS
	#define REQ_NADSBOX_GET_EXT    0x0E
	#define REQ_NADSBOX_COND_LIST  0x0F // NADSBox but TPDD2 also responds with RET_CACHE
#endif
#define REQ_VERSION       0x23 // TPDD2 Get Version Number
#define REQ_CACHE         0x30 // TPDD2 sector access
#define REQ_MEM_WRITE     0x31 // TPDD2 sector access
#define REQ_MEM_READ      0x32 // TPDD2 sector access
#define REQ_SYSINFO       0x33 // TPDD2 Get System Information
#define REQ_EXEC          0x34 // TPDD2 Execute Program

// TPDD return block formats
#define RET_READ          0x10
#define RET_DIRENT        0x11
#define RET_STD           0x12 // shared return format for: error open close delete status write
#define RET_VERSION       0x14 // TPDD2
#define RET_CONDITION     0x15 // TPDD2
#define RET_CACHE         0x38 // TPDD2 shared return format for: cache mem_write cond_list
#define RET_MEM_READ      0x39 // TPDD2
#define RET_SYSINFO       0x3A // TPDD2
#define RET_EXEC          0x3B // TPDD2


// directory entry request types
#define DIRENT_SET_NAME   0x00
#define DIRENT_GET_FIRST  0x01
#define DIRENT_GET_NEXT   0x02
#define DIRENT_GET_PREV   0x03 // TPDD2
#define DIRENT_CLOSE      0x04 // TPDD2

// file open access modes
#define F_OPEN_NONE       0x00  // used in here, not part of protocol
#define F_OPEN_WRITE      0x01
#define F_OPEN_APPEND     0x02
#define F_OPEN_READ       0x03

// TPDD Operation-mode error codes
#define ERR_SUCCESS       0x00 // 'Operation Complete'
#define ERR_NO_FILE       0x10 // 'File Not Found'
#define ERR_EXISTS        0x11 // 'File Exists'
#define ERR_NO_FNAME      0x30 // 'Missing Filename'
#define ERR_DIR_SEARCH    0x31 // 'Directory Search Error'
#define ERR_BANK          0x35 // 'Bank Error'
#define ERR_PARAM         0x36 // 'Parameter Error'
#define ERR_FMT_MISMATCH  0x37 // 'Open Format Mismatch'
#define ERR_EOF           0x3F // 'End of File'
#define ERR_NO_START      0x40 // 'No Start Mark'
#define ERR_ID_CRC        0x41 // 'ID CRC Check Error'
#define ERR_SECTOR_LEN    0x42 // 'Sector Length Error'
#define ERR_FMT_VERIFY    0x44 // 'Format Verify Error'
#define ERR_NOT_FORMATTED 0x45 // 'Disk Not Formatted'
#define ERR_FMT_INTERRUPT 0x46 // 'Format Interruption'
#define ERR_ERASE_OFFSET  0x47 // 'Erase Offset Error'
#define ERR_DATA_CRC      0x49 // 'DATA CRC Check Error'
#define ERR_SECTOR_NUM    0x4A // 'Sector Number Error'
#define ERR_READ_TIMEOUT  0x4B // 'Read Data Timeout'
#define ERR_SECTOR_NUM2   0x4D // 'Sector Number Error'
#define ERR_WRITE_PROTECT 0x50 // 'Write-Protected Disk'
#define ERR_DISK_NOINIT   0x5E // 'Disk Not Formatted'
#define ERR_DIR_FULL      0x60 // 'Disk Full or Max File Size Exceeded or Directory Full' / TPDD2 'Directory Full'
#define ERR_DISK_FULL     0x61 // 'Disk Full'
#define ERR_FILE_LEN      0x6E // 'File Too Long' (real drive limits to 65534, we exceed for REXCPM)
#define ERR_NO_DISK       0x70 // 'No Disk'
#define ERR_DISK_CHG      0x71 // 'Disk Not Inserted or Disk Change Error' / TPDD2 'Disk Change Error'
#define ERR_DEFECTIVE     0x83 // 'Defective Disk'  (real drive needs a power-cycle to clear this error)

// TPDD1 FDC-mode commands
#define FDC_SET_MODE        'M' // set Operation-mode or FDC-mode
#define FDC_CONDITION       'D' // drive condition
#define FDC_FORMAT          'F' // format disk
#define FDC_FORMAT_NV       'G' // format disk without verify
#define FDC_READ_ID         'A' // read sector ID
#define FDC_READ_SECTOR     'R' // read sector data
#define FDC_SEARCH_ID       'S' // search sector ID
#define FDC_WRITE_ID        'B' // write sector ID
#define FDC_WRITE_ID_NV     'C' // write sector ID without verify
#define FDC_WRITE_SECTOR    'W' // write sector data
#define FDC_WRITE_SECTOR_NV 'X' // write sector data without verify
#define FDC_CMDS {FDC_SET_MODE,FDC_CONDITION,FDC_FORMAT,FDC_FORMAT_NV,FDC_READ_ID,FDC_READ_SECTOR,FDC_SEARCH_ID,FDC_WRITE_ID,FDC_WRITE_ID_NV,FDC_WRITE_SECTOR,FDC_WRITE_SECTOR_NV,0x00}

// TPDD1 FDC-mode error codes
// There is no documentation for FDC error codes.
// These are guesses from experimenting.
// These appear in the first hex pair of an 8-byte FDC-mode response.
#define ERR_FDC_SUCCESS         0 // 'OK'
#define ERR_FDC_LSN_LO         17 // 'Logical Sector Number Below Range'
#define ERR_FDC_LSN_HI         18 // 'Logical Sector Number Above Range'
#define ERR_FDC_PSN_HI         19 // 'Physical Sector Number Above Range'
#define ERR_FDC_PARAM          33 // 'Parameter Invalid, Wrong Type'
#define ERR_FDC_LSSC_LO        50 // 'Invalid Logical Sector Size Code'
#define ERR_FDC_LSSC_HI        51 // 'Logical Sector Size Code Above Range'
#define ERR_FDC_ID_NOT_FOUND   60 // 'ID Not Found'
#define ERR_FDC_S_BAD_PARAM    61 // 'Search ID Unexpected Parameter'
#define ERR_FDC_NOT_FORMATTED 160 // 'Disk Not Formatted'
#define ERR_FDC_READ          161 // 'Read Error'
#define ERR_FDC_WRITE_PROTECT 176 // 'Write-Protected Disk'
#define ERR_FDC_COMMAND       193 // 'Invalid Command'
#define ERR_FDC_NO_DISK       209 // 'Disk Not Inserted'
#define ERR_FDC_INTERRUPTED   216 // 'Operation Interrupted'

// TPDD1 FDC Condition bits
#define FDC_COND_NOTINS       0x80 // bit 7 : disk not inserted
#define FDC_COND_CHANGED      0x40 // bit 6 : disk changed
#define FDC_COND_WPROT        0x20 // bit 5 : disk write-protected
#define FDC_COND_b4           0x10
#define FDC_COND_b3           0x08
#define FDC_COND_b2           0x04
#define FDC_COND_b1           0x02
#define FDC_COND_b0           0x01
#define FDC_COND_NONE         0x00 // no conditions

// TPDD1 FDC Logical Sector Length Codes
#define FDC_LOGICAL_SIZE_CODES {64,80,128,256,512,1024,1280}

// TPDD2 Condition bits
#define PDD2_COND_b7          0x80
#define PDD2_COND_b6          0x40
#define PDD2_COND_b5          0x20
#define PDD2_COND_b4          0x10
#define PDD2_COND_CHANGED     0x08 // bit 3 : disk changed
#define PDD2_COND_NOTINS      0x04 // bit 2 : disk not inserted
#define PDD2_COND_WPROT       0x02 // bit 1 : write protected disk
#define PDD2_COND_POWER       0x01 // bit 0 : low power
#define PDD2_COND_NONE        0x00 // no conditions

// lengths & addresses
#define PDD1_TRACKS           40
#define PDD1_SECTORS          2
#define PDD2_TRACKS           80
#define PDD2_SECTORS          2
#define TPDD_DATA_MAX         260  // largest theoretical packet is 256+3
#define REQ_RW_DATA_MAX       128  // largest chunk size in req_read() req_write()
#define LEN_RET_STD           0x01
#define LEN_RET_DME           0x0B
#define LEN_RET_DIRENT        0x1C
#define TPDD_FILENAME_LEN     24
#define LOCAL_FILENAME_MAX    256
#define SECTOR_ID_LEN         12
#define SECTOR_HEADER_LEN     (SECTOR_ID_LEN+1) // pdd1: lsc+id, pdd2: id+unknown
#define SECTOR_DATA_LEN       1280
//#define OLD_PDD2_HEADER_LEN   4 // for old .pdd2 disk image files
#define SECTOR_LEN            (SECTOR_HEADER_LEN+SECTOR_DATA_LEN)
#define PDD1_IMG_LEN          (PDD1_TRACKS*PDD1_SECTORS*SECTOR_LEN)
#define PDD2_IMG_LEN          (PDD2_TRACKS*PDD2_SECTORS*SECTOR_LEN)
#define SMT_OFFSET            1240
#define PDD1_SMT              0x80
#define PDD2_SMT              0xC0
#define PDD2_ID_ADDR          0x8004
#define PDD2_MEM_READ_MAX     252 // real drive absolute limit
#define PDD2_MEM_WRITE_MAX    127 // real drive absolute limit

// TPDD2 version data: 41 10 01 00 50 05 00 02 00 28 00 E1 00 00 00
#define VERSION_MSB       0x41
#define VERSION_LSB       0x10
#define SIDES             0x01
#define TRACKS_MSB        0x00
#define TRACKS_LSB        0x50
#define SECTOR_SIZE_MSB   0x05
#define SECTOR_SIZE_LSB   0x00
#define SECTORS_PER_TRACK 0x02
#define DIRENTS_MSB       0x00
#define DIRENTS_LSB       0x28
#define MAX_FD            0x00
#define MODEL             0xE1     // E1 = TPDD2
#define VERSION_R0        0x00
#define VERSION_R1        0x00
#define VERSION_R2        0x00

// TPDD2 sysinfo data: 80 13 05 00 10 E1
#define SECTOR_CACHE_START_MSB 0x80
#define SECTOR_CACHE_START_LSB 0x13
#define SECTOR_CACHE_LEN_MSB   0x05
#define SECTOR_CACHE_LEN_LSB   0x00
#define SYSINFO_CPU            0x10 // 0x10 = HD6301
//#define MODEL                  0xE1

// flags
#define FE_FLAGS_NONE          0
#define FE_FLAGS_DIR           1
#define NO_RET                 0
#define ALLOW_RET              1
#define CACHE_LOAD             0
#define CACHE_COMMIT           1
#define CACHE_COMMIT_VERIFY    2
#define MEM_CACHE              0
#define MEM_CPU                1

// KC-85-platform BASIC interpreter EOL & EOF bytes for bootstrap()
#define BASIC_EOL 0x0D
#define BASIC_EOF 0x1A
#define LOCAL_EOL 0x0A

#define OPR_CMD_SYNC 0x5A
#define FDC_CMD_EOL  0x0D

#endif // PDD_CONSTANTS_H
