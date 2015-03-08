#ifndef _MISC_TAR_H
#define _MISC_TAR_H

#define TAR_BLKSIZE 512

struct posix_header
{                              /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
                                /* 500 */
};

#define isodigit(c) ((c) >= '0' && (c) < '8')

int tar_file_walk (const void *base, uint32_t size, const char *path_prefix, int (*cb) (const char *path, const void *base, uint32_t size, uint32_t mode, void *opaque), void *opaque);
int tar_file_lookup (const void *base, uint32_t size, const char *file, const void **pbase, uint32_t *psize);

#endif /* _MISC_TAR_H */
