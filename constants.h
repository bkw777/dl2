#ifndef PDD_CONSTANTS_H
#define PDD_CONSTANTS_H

#include <stdint.h>

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

// TPDD return block formats                {fmt,len}
#define RET_READ          0x10
static const uint8_t RET_DIRENT[2]    = {0x11,0x1C};
static const uint8_t RET_STD[2]       = {0x12,0x01}; // shared return format for: error open close delete status write
static const uint8_t RET_VERSION[2]   = {0x14,0x0F}; // TPDD2
static const uint8_t RET_CONDITION[2] = {0x15,0x01}; // TPDD2
static const uint8_t RET_CACHE[2]     = {0x38,0x01}; // TPDD2 shared return format for: cache mem_write cond_list
#define RET_MEM_READ      0x39 // TPDD2
static const uint8_t RET_SYSINFO[2]   = {0x3A,0x06}; // TPDD2
static const uint8_t RET_EXEC[2]      = {0x3B,0x03}; // TPDD2

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
// Normal
#define ERR_SUCCESS       0x00 // 'Operation Complete'
// File
#define ERR_NO_FILE       0x10 // 'File Not Found'
#define ERR_EXISTS        0x11 // 'File Exists'
// Sequence
#define ERR_NO_FNAME      0x30 // 'Missing Filename'
#define ERR_DIR_SEARCH    0x31 // 'Directory Search Error'
#define ERR_BANK          0x35 // 'Bank Error'
#define ERR_PARAM         0x36 // 'Parameter Error'
#define ERR_FMT_MISMATCH  0x37 // 'Open Format Mismatch'
#define ERR_EOF           0x3F // 'End of File'
// Disk I/O
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
// Protect
#define ERR_WRITE_PROTECT 0x50 // 'Write-Protected Disk'
#define ERR_DISK_NOINIT   0x5E // 'Disk Not Formatted'
#define ERR_WP_TPDD1_DISK 0x5F // TPDD2 'Write Protect to 26-3808 Diskette'
// File Territory
#define ERR_DIR_FULL      0x60 // 'Disk Full or Max File Size Exceeded or Directory Full' / TPDD2 'Directory Full'
#define ERR_DISK_FULL     0x61 // 'Disk Full'
#define ERR_FILE_LEN      0x6E // 'File Too Long' (real drive limits to 65534, we exceed for REXCPM)
// Diskette Condition
#define ERR_NO_DISK       0x70 // 'Disk Not Inserted'
#define ERR_DISK_CHG      0x71 // 'Disk Change Error'
// Sensor
#define ERR_NO_INDEX_SIGNAL 0x80
#define ERR_ABNORMAL_TRACK_ZERO 0x81
#define ERR_ABNORMAL_INDEX_SIGNAL 0x82
#define ERR_DEFECTIVE     0x83 // 'Defective Disk' - real drive needs a power-cycle to clear this error - not in the manual

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
static const char FDC_CMDS[] = {FDC_SET_MODE,FDC_CONDITION,FDC_FORMAT,FDC_FORMAT_NV,FDC_READ_ID,FDC_READ_SECTOR,FDC_SEARCH_ID,FDC_WRITE_ID,FDC_WRITE_ID_NV,FDC_WRITE_SECTOR,FDC_WRITE_SECTOR_NV,0x00};

// TPDD1 FDC-mode error codes
// There is no documentation for FDC error codes.
// These are guesses from experimenting.
// These appear in the first hex pair of an 8-byte FDC-mode response.
#define ERR_FDC_SUCCESS        0x00 // 'OK'
#define ERR_FDC_LSN_LO         0x11 // 'Logical Sector Number Below Range'
#define ERR_FDC_LSN_HI         0x12 // 'Logical Sector Number Above Range'
#define ERR_FDC_PSN_HI         0x13 // 'Physical Sector Number Above Range'
#define ERR_FDC_PARAM          0x21 // 'Parameter Invalid, Wrong Type'
#define ERR_FDC_LSSC_LO        0x32 // 'Invalid Logical Sector Size Code'
#define ERR_FDC_LSSC_HI        0x33 // 'Logical Sector Size Code Above Range'
#define ERR_FDC_ID_NOT_FOUND   0x3C // 'ID Not Found'
#define ERR_FDC_S_BAD_PARAM    0x3D // 'Search ID Unexpected Parameter'
#define ERR_FDC_NOT_FORMATTED  0xA0 // 'Disk Not Formatted'
#define ERR_FDC_READ           0xA1 // 'Read Error'
#define ERR_FDC_WRITE_PROTECT  0xB0 // 'Write-Protected Disk'
#define ERR_FDC_COMMAND        0xC1 // 'Invalid Command'
#define ERR_FDC_NO_DISK        0xD1 // 'Disk Not Inserted'
#define ERR_FDC_INTERRUPTED    0xD8 // 'Operation Interrupted'

// TPDD1 FDC Logical Sector Length Codes
static const uint16_t FDC_LOGICAL_SECTOR_SIZE[7] = {64,80,128,256,512,1024,1280};

// TPDD1 Condition bits
#define PDD1_COND_BIT_NOTINS   7 // disk not inserted
#define PDD1_COND_BIT_CHANGED  6 // disk changed
#define PDD1_COND_BIT_WPROT    5 // disk write-protected
#define PDD1_COND_BIT_4        4
#define PDD1_COND_BIT_3        3
#define PDD1_COND_BIT_2        2
#define PDD1_COND_BIT_1        1
#define PDD1_COND_BIT_0        0
#define PDD1_COND_NONE         0x00 // no conditions

// TPDD2 Condition bits
#define PDD2_COND_BIT_7        7
#define PDD2_COND_BIT_6        6
#define PDD2_COND_BIT_5        5
#define PDD2_COND_BIT_4        4
#define PDD2_COND_BIT_CHANGED  3 // disk changed
#define PDD2_COND_BIT_NOTINS   2 // disk not inserted
#define PDD2_COND_BIT_WPROT    1 // disk write protected
#define PDD2_COND_BIT_POWER    0 // low power
#define PDD2_COND_NONE         0x00 // no conditions

// lengths & addresses
#define PDD1_TRACKS           40
#define PDD1_SECTORS          2
#define PDD2_TRACKS           80
#define PDD2_SECTORS          2
#define DIRENTS               40
#define REQ_RW_DATA_MAX       128  // largest chunk size in req_read() req_write()
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
#define PDD2_MEM_READ_MAX     252 // real drive absolute limit
#define PDD2_MEM_WRITE_MAX    127 // real drive absolute limit
#define TPDD_MSG_MAX          256 // largest theoretical packet is 256+3, largest actual is 252+3

// cpu memory map
#define IOPORT_ADDR           0x00
#define IOPORT_LEN            0x1F
#define CPURAM_ADDR           0x80
#define CPURAM_LEN            0x7F
#define GA_ADDR               0x4000
#define GA_LEN                0x03
#define RAM_ADDR              0x8000
#define RAM_LEN               0x0800
#define ROM_ADDR              0xF000
#define ROM_LEN               0x1000

// sector cache
#define PDD2_ID_REL           0x04
#define PDD2_ID_ADDR          (RAM_ADDR+PDD2_ID_REL)
#define PDD2_DATA_REL         0x13
#define PDD2_DATA_ADDR        (RAM_ADDR+PDD2_DATA_REL)
#define PDD2_CACHE_LEN        (PDD2_DATA_REL+SECTOR_DATA_LEN)
#define PDD2_CACHE_LEN_MSB    ((PDD2_CACHE_LEN>>0x08)&0xFF) // 0x05
#define PDD2_CACHE_LEN_LSB    (PDD2_CACHE_LEN&0xFF)      // 0x13

// TPDD2 version data: 41 10 01 00 50 05 00 02 00 28 00 E1 00 00 00
#define VERSION_MSB       0x41
#define VERSION_LSB       0x10
#define SIDES             0x01
#define TRACKS_MSB        ((PDD2_TRACKS>>0x08)&0xFF)
#define TRACKS_LSB        (PDD2_TRACKS&0xFF)
#define SECTOR_SIZE_MSB   ((SECTOR_DATA_LEN>>0x08)&0xFF)
#define SECTOR_SIZE_LSB   (SECTOR_DATA_LEN&0xFF)
#define SECTORS_PER_TRACK 0x02
#define DIRENTS_MSB       ((DIRENTS>>0x08)&0xFF)
#define DIRENTS_LSB       (DIRENTS&0xFF)
#define MAX_FD            0x00      // it's 0 but it means the highest fd# is 0, meaning max 1 open file
#define MODEL_CODE        0xE1      // E1 = TPDD2
#define VERSION_R0        0x00
#define VERSION_R1        0x00
#define VERSION_R2        0x00

// TPDD2 sysinfo data: 80 13 05 00 10 E1
#define SECTOR_CACHE_START_MSB ((PDD2_DATA_ADDR>>0x08)&0xFF) // 0x80
#define SECTOR_CACHE_START_LSB (PDD2_DATA_ADDR&0xFF)         // 0x13
// sysinfo[2] = SECTOR_SIZE_MSB
// sysinfo[3] = SECTOR_SIZE_LSB
#define SYSINFO_CPU_CODE       0x10 // 0x10 = HD6301
// sysinfo[5] = MODEL_CODE

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

// drive command seperators
#define OPR_CMD_SYNC 0x5A
#define FDC_CMD_EOL  0x0D

// compatibility modes
#define DOT_FLOPPY  6
#define DOT_WP2     8
#define MODE_OPR    1
#define MODE_FDC    0

#endif // PDD_CONSTANTS_H
