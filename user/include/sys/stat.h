#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <bits/types.h>

/* 32 bit stat structure */
struct stat
{
  __dev_t st_dev;		/* Device.  */
  __ino_t __st_ino;			/* 32bit file serial number.	*/
  __nlink_t st_nlink;		/* Link count.  */
  __mode_t st_mode;		/* File mode.  */
  __uid_t st_uid;		/* User ID of the file's owner.	*/
  __gid_t st_gid;		/* Group ID of the file's group.*/
  __dev_t st_rdev;		/* Device number, if device.  */
  __off64_t st_size;			/* Size of file, in bytes.  */
  __blksize_t st_blksize;	/* Optimal block size for I/O.  */
  __blkcnt64_t st_blocks;		/* Number 512-byte blocks allocated. */
  __time_t st_atime;			/* Time of last access.  */
  __syscall_ulong_t st_atimensec;	/* Nscecs of last access.  */
  __time_t st_mtime;			/* Time of last modification.  */
  __syscall_ulong_t st_mtimensec;	/* Nsecs of last modification.  */
  __time_t st_ctime;			/* Time of last status change.  */
  __syscall_ulong_t st_ctimensec;	/* Nsecs of last status change.  */
  __ino64_t st_ino;			/* File serial number.	*/
};

#endif /* _SYS_STAT_H */
