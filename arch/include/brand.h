#ifndef _BRAND_H
#define _BRAND_H

#include <util.h>

#define _VERSION(a, b, c) (((a) << 16) | ((b) << 8) | ((c)))

#define VER_MAJOR(v)   (((v) >> 16) & 0xff)
#define VER_MINOR(v)   (((v) >> 8) & 0xff)
#define VER_RELEASE(v) ((v) & 0xff)

#define KERNEL_CURRENT_VERSION _VERSION(0, 1, 0)

#define KERNEL_BRAND_NAME "Neftis microkernel "         \
  STRINGIFY (VER_MAJOR (KERNEL_CURRENT_VERSION)) "."    \
  STRINGIFY (VER_MINOR (KERNEL_CURRENT_VERSION)) "."    \
  STRINGIFY (VER_RELEASE (KERNEL_CURRENT_VERSION))
  


#define KERNEL_BOOT_STRING KERNEL_BRAND_NAME " (" ARCH_STRING ") - " COMPILER_APPEND "\n"

#endif /* _BRAND_H */
