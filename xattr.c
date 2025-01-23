// Instead of hard-coding 'F' for the ATTR field at all times,
// store the actual attr byte from the client in xattr.

// dl.c always sets attr first (default or from client) then calls one of these.
// If !USE_XATTR, then these are all no-op and attr simply left unchanged.
// If USE_XATTR, then these may or may not copy xattr to attr, or attr to xattr,
// depending on get vs set and depending on if the xattr exists.

#ifdef USE_XATTR

#if defined(__FreeBSD__)
#include <sys/extattr.h>
#else
#include <sys/xattr.h>
#endif

#include "xattr.h"

#ifndef XATTR_NAME
#define XATTR_NAME "pdd.attr"
#endif

const char* xattr_name =
#if defined(__linux__)
	 "user." XATTR_NAME
#elif defined(__APPLE__)
	XATTR_NAME "#S"
#else
	XATTR_NAME
#endif
;

void dl_getxattr(const char* path, uint8_t* value) {
#if defined(__linux__)
	getxattr(path, xattr_name, value, 1);
#elif defined(__APPLE__)
	getxattr(path, xattr_name, value, 1, 0, 0);
#elif defined(__FreeBSD__)
	extattr_get_file(path, EXTATTR_NAMESPACE_USER, xattr_name, value, 1);
#endif
}

void dl_fgetxattr(int fd, uint8_t* value) {
#if defined(__linux__)
	fgetxattr(fd, xattr_name, value, 1);
#elif defined(__APPLE__)
	fgetxattr(fd, xattr_name, value, 1, 0, 0);
#elif defined(__FreeBSD__)
	extattr_get_fd(fd, EXTATTR_NAMESPACE_USER, xattr_name, value, 1);
#endif
}

void dl_fsetxattr(int fd, const uint8_t* value) {
#if defined(__linux__)
	fsetxattr(fd, xattr_name, value, 1, 0);
#elif defined(__APPLE__)
	fsetxattr(fd, xattr_name, value, 1, 0, 0);
#elif defined(__FreeBSD__)
	extattr_set_fd(fd, EXTATTR_NAMESPACE_USER, xattr_name, value, 1);
#endif
}

#endif // USE_XATTR

