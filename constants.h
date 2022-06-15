#ifndef TPDD_CONSTANTS
#define TPDD_CONSTANTS

// TPDD drive firmware/protocol constants

// TPDD request block formats
#define REQ_DIRENT        0x00
#define REQ_OPEN          0x01
#define REQ_CLOSE         0x02
#define REQ_READ          0x03
#define REQ_WRITE         0x04
#define REQ_DELETE        0x05
#define REQ_FORMAT        0x06
#define REQ_STATUS        0x07
#define REQ_FDC           0x08
#define REQ_SEEK          0x09
#define REQ_TELL          0x0A
#define REQ_SET_EXT       0x0B
#define REQ_CONDITION     0x0C // TPDD2
#define REQ_RENAME        0x0D
#define REQ_REQ_EXT_QUERY 0x0E
#define REQ_COND_LIST     0x0F
#define REQ_PDD2_UNK11    0x11 // TPDD2 unknown function - TPDD2 responds: 3A 06 80 13 05 00 10 E1 36
#define REQ_PDD2_UNK23    0x23 // TPDD2 unknown function - "TS-DOS mystery" TS-DOS uses for to detect TPDD2 - TPDD2 responds, TPDD1 does not.
#define REQ_CACHE_LOAD    0x30 // TPDD2 sector access
#define REQ_CACHE_WRITE   0x31 // TPDD2 sector access
#define REQ_CACHE_READ    0x32 // TPDD2 sector access
#define REQ_PDD2_UNK33    0x33 // TPDD2 same as UNK11

// TPDD return block formats
#define RET_READ          0x10
#define RET_DIRENT        0x11
#define RET_STD           0x12 // shared return format for: error open close delete status write
#define RET_PDD2_UNK23    0x14 // TPDD2 unknown function - "TS-DOS mystery" TS-DOS uses to detect TPDD2
#define RET_CONDITION     0x15 // TPDD2
#define RET_CACHE_STD     0x38 // TPDD2 shared return format for: sector_cache write_cache
#define RET_READ_CACHE    0x39 // TPDD2
#define RET_PDD2_UNK11    0x3A // TPDD2 unknown function
#define RET_PDD2_UNK33    0x3A // TPDD2 same as UNK11

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
#define ERR_CMDSEQ        0x30 // 'Command Parameter Error or Sequence Error'
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

// TPDD1 FDC-mode error codes
// There is no documentation for FDC error codes.
// These are guesses from experimenting.
// These appear in the first hex pair of an 8-byte FDC-mode response.
#define ERR_FDC_SUCCESS         0 // 'OK'
#define ERR_FDC_LSN_LO         17 // 'Logical Sector Number Below Range'
#define ERR_FDC_LSN_HI         18 // 'Logical Sector Number Above Range'
#define ERR_FDC_PSN HI         19 // 'Physical Sector Number Above Range'
#define ERR_FDC_PARAM          33 // 'Parameter Invalid, Wrong Type'
#define ERR_FDC_LSSC_LO        50 // 'Invalid Logical Sector Size Code'
#define ERR_FDC_LSSC_HI        51 // 'Logical Sector Size Code Above Range'
#define ERR_FDC_NOT_FORMATTED 160 // 'Disk Not Formatted'
#define ERR_FDC_READ          161 // 'Read Error'
#define ERR_FDC_WRITE_PROTECT 176 // 'Write-Protected Disk'
#define ERR_FDC_COMMAND       193 // 'Invalid Command'
#define ERR_FDC_NO_DISK       209 // 'Disk Not Inserted'

// fixed lengths
#define TPDD_DATA_MAX      0x80
#define TPDD_FREE_SECTORS  80 // max 80 for TPDD1, 160 for TPDD2
#define LEN_RET_STD        0x01
#define LEN_RET_DME        0x0B
#define LEN_RET_DIRENT     0x1C
#define TPDD_FILENAME_LEN  24
#define LOCAL_FILENAME_MAX 256

// KC-85 platform BASIC interpreter EOL & EOF bytes for bootstrap()
#define BASIC_EOL 0x0D
#define BASIC_EOF 0x1A

#define DIR_FLAG 0x01

#define NO_RET 0
#define ALLOW_RET 1

#endif
