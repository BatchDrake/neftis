#ifndef _MEMFS_H
#define _MEMFS_H

#include <atomik.h>
#include <u_util.h>

#define MEMFILE_TYPE_REG 0
#define MEMFILE_TYPE_DIR 1

struct memfs_file;

struct memfs_dir
{
  PTR_LIST (struct memfs_file, file);
};

struct memfs_reg
{
  size_t size;
  void *data;
};

struct memfs_file
{
  int   type;
  char *name;
  
  union
  {
    struct memfs_dir as_dir;
    struct memfs_reg as_reg;
  };
};

typedef struct mf_handle
{
  int id;
  busword_t owner;
  busword_t ptr;
  struct memfs_file *file;
}
mf_handle_t;

#endif /* _MEMFS_H */
