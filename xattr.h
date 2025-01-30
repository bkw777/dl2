#ifndef PDD_XATTR_H
#define PDD_XATTR_H

#include <stdint.h>

#ifdef USE_XATTR

// default xattr_name is #defined in xattr.c
// and overridable at run-time from main.c
extern const char* xattr_name;

void dl_getxattr (const char* path, uint8_t* value);
void dl_fgetxattr (int fd, uint8_t* value);
void dl_fsetxattr (int fd, const uint8_t* value);

#else // USE_XATTR

#define dl_getxattr(x,y)
#define dl_fgetxattr(x,y)
#define dl_fsetxattr(x,y)

#endif // USE_XATTR

#endif // PDD_XATTR_H
