/*
 *    Entry point for the Atomik's filesystem daemon
 *    Copyright (C) 2015  Gonzalo J. Carracedo
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <atomik.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <serv/fs.h>
#include <memfs.h>

/* This phony filesystem is implemented without . or .. */

struct memfs_file *root;
PTR_LIST (mf_handle_t, openfile);

mf_handle_t *
mf_handle_new (busword_t owner, struct memfs_file *file)
{
  mf_handle_t *new;

  if ((new = calloc (1, sizeof (mf_handle_t))) == NULL)
    return NULL;

  new->owner = owner;
  new->file  = file;
  new->id    = -1;
  new->ptr   = 0;
  
  return new;
}

void
mf_handle_destroy (mf_handle_t *handle)
{
  free (handle);
}

#define _KILLSLASH(c) ((c) == '/' ? '\0' : (c))

static int
namei_strcmp (const char *s1, const char *s2)
{
  for (; _KILLSLASH (*s1) == _KILLSLASH (*s2); ++s1, ++s2)
    if (*s1 == 0)
      return 0;
  
  return *(unsigned char *) s1 < *(unsigned char *) s2 ? -1 : 1;
}

static struct memfs_file *
memfs_file_dir_lookup (const struct memfs_file *dir, const char *name, int *error)
{
  int i;

  if (dir->type != MEMFILE_TYPE_DIR)
  {
    *error = ENOTDIR;
    return NULL;
  }
  
  for (i = 0; i < dir->as_dir.file_count; ++i)
    if (dir->as_dir.file_list[i] != NULL)
      if (namei_strcmp (dir->as_dir.file_list[i]->name, name) == 0)
        return dir->as_dir.file_list[i];

  *error = ENOENT;
  return NULL;
}

struct memfs_file *
memfs_namei (const struct memfs_file *root, const char *path, int *error)
{
  const char *next_slash;
  
  struct memfs_file *file;

  if (*path++ != '/')
  {
    *error = ENOENT;
    return NULL;
  }

  if (*path == '\0')
  {
    if (root->type != MEMFILE_TYPE_DIR)
    {
      *error = ENOTDIR;
      return NULL;
    }

    return (struct memfs_file *) root;
  }
  
  if ((next_slash = strchr (path, '/')) == NULL)
    next_slash = path + strlen (path);

  if ((file = memfs_file_dir_lookup (root, path, error)) == NULL)
    return NULL;

  /* File path is not over yet */
  if (*next_slash)
    file = memfs_namei (file, next_slash, error);

  return file;
}

struct memfs_file *
memfs_file_reg_new (const char *name, void *data, size_t size)
{
  struct memfs_file *new;

  if ((new = malloc (sizeof (struct memfs_file))) == NULL)
    return NULL;

  if ((new->name = strdup (name)) == NULL)
  {
    free (new);

    return NULL;
  }

  new->type = MEMFILE_TYPE_REG;
  
  new->as_reg.data = data;
  new->as_reg.size = size;

  return new;
}

struct memfs_file *
memfs_file_dir_new (const char *name)
{
  struct memfs_file *new;

  if ((new = malloc (sizeof (struct memfs_file))) == NULL)
    return NULL;

  if ((new->name = strdup (name)) == NULL)
  {
    free (new);

    return NULL;
  }

  new->type = MEMFILE_TYPE_DIR;
  
  memset (&new->as_dir, 0, sizeof (struct memfs_dir));
  
  return new;
}

int
memfs_file_dir_append (struct memfs_file *dir, struct memfs_file *file)
{
  int id;
  
  if (dir->type != MEMFILE_TYPE_DIR)
  {
    errno = ENOTDIR;
    return -1;
  }
  
  return PTR_LIST_APPEND_CHECK (dir->as_dir.file, file);
}

void
memfs_file_destroy (struct memfs_file *file)
{
  int i;

  if (file->type == MEMFILE_TYPE_DIR)
  {
    for (i = 0; i < file->as_dir.file_count; ++i)
      if (file->as_dir.file_list[i] != NULL)
        memfs_file_destroy (file->as_dir.file_list[i]);

    if (file->as_dir.file_list != NULL)
      free (file->as_dir.file_list);
  }

  free (file);
}

struct memfs_file *
memfs_file_dir_new_under (struct memfs_file *root, const char *dir)
{
  struct memfs_file *file;

  if ((file = memfs_file_dir_new (dir)) == NULL)
    return NULL;

  if (PTR_LIST_APPEND_CHECK (root->as_dir.file, file) == -1)
  {
    memfs_file_destroy (file);

    return NULL;
  }

  return file;
}

struct memfs_file *
memfs_file_reg_new_under (struct memfs_file *root, const char *name, void *data, size_t size)
{
  struct memfs_file *file;

  if ((file = memfs_file_reg_new (name, data, size)) == NULL)
    return NULL;

  if (PTR_LIST_APPEND_CHECK (root->as_dir.file, file) == -1)
  {
    memfs_file_destroy (file);

    return NULL;
  }

  return file;
}

#define CHKPTR(stmt) if ((stmt) == NULL) goto fail
#define CHKINT(stmt) if ((stmt) == -1) goto fail


void
__debug_fs_tree (struct memfs_file *file, int padding)
{
  int i;

  for (i = 0; i < padding; ++i)
    puts ("  ");

  if (file->type == MEMFILE_TYPE_DIR)
    puts ("\033[1;34m");
  
  puts (file->name);

  if (file->type == MEMFILE_TYPE_DIR)
  {
    puts ("\033[0m/\n");
    for (i = 0; i < file->as_dir.file_count; ++i)
      if (file->as_dir.file_list[i] != NULL)
        __debug_fs_tree (file->as_dir.file_list[i], padding + 1);
  }
  else
  {
    puts (" (");
    puti (file->as_reg.size);
    puts (" bytes)\n");
  }
}

void
debug_fs_tree (struct memfs_file *file)
{
  __debug_fs_tree (file, 0);
}

int
init_fs_tree (void)
{
  struct memfs_file *f1, *f2, *f3, *pw;
  
  CHKPTR (root = memfs_file_dir_new ("<memfs-root>"));
  CHKPTR (f1   = memfs_file_dir_new_under (root, "etc"));
  CHKPTR (f2   = memfs_file_dir_new_under (root, "bin"));
  CHKPTR (f3   = memfs_file_dir_new_under (root, "usr"));
  CHKPTR (pw   = memfs_file_reg_new_under (f1, "passwd", "<no passwords>", 14));
  CHKPTR (pw   = memfs_file_reg_new_under (f1, "hostname", "<no passwords>", 14));
  CHKPTR (pw   = memfs_file_reg_new_under (f1, "atomik-release", "<no passwords>", 14));
  CHKPTR (pw   = memfs_file_reg_new_under (f2, "uname", "<no passwords>", 14));
  CHKPTR (f3   = memfs_file_dir_new_under (f1, "atomik"));
  CHKPTR (pw   = memfs_file_reg_new_under (f3, "settings.conf", "<no passwords>", 14));
  CHKPTR (pw   = memfs_file_reg_new_under (f2, "ls", "<no passwords>", 14));
  
  
  return 0;
  
fail:
  return -ENOMEM;
}

int
fs_file_open (busword_t owner, const char *path)
{
  struct memfs_file *file;
  mf_handle_t *handle;
  
  int error;
  int hid;
  
  if ((file = memfs_namei (root, path, &error)) == NULL)
    return -error;

  if ((handle = mf_handle_new (owner, file)) == NULL)
    return -ENOENT;

  if ((hid = PTR_LIST_APPEND_CHECK (openfile, handle)) == -1)
  {
    mf_handle_destroy (handle);

    return -ENOMEM;
  }

  handle->id = hid;

  return hid;
}

mf_handle_t *
mf_handle_from_hid (busword_t owner, busword_t hid)
{
  if (hid >= openfile_count ||
      openfile_list[hid] == NULL ||
      openfile_list[hid]->owner != owner)
    return NULL;

  return openfile_list[hid];
}

int
fs_close (busword_t owner, busword_t hid)
{
  mf_handle_t *handle;

  if ((handle = mf_handle_from_hid (owner, hid)) == NULL)
    return -EBADF;

  openfile_list[handle->id] = NULL;

  mf_handle_destroy (handle);

  return 0;
}

int
fs_write (busword_t owner, busword_t hid, const void *data, size_t size)
{
  mf_handle_t *handle;
  
  if ((handle = mf_handle_from_hid (owner, hid)) == NULL)
    return -EBADF;

  if (handle->file->type == MEMFILE_TYPE_DIR)
    return -EISDIR;

  if (size > 0)
    return -EROFS;
}

int
fs_read (busword_t owner, busword_t hid, void *data, size_t size)
{
  mf_handle_t *handle;
  
  if ((handle = mf_handle_from_hid (owner, hid)) == NULL)
    return -EBADF;

  if (handle->file->type == MEMFILE_TYPE_DIR)
    return -EISDIR;

  if (size > 0)
    return -EROFS;

  if (handle->ptr + size > handle->file->as_reg.size)
  {
    size = handle->file->as_reg.size - handle->ptr;
    
    if (size < 0)
      size = 0;
  }

  if (size > 0)
    memcpy (data, handle->file->as_reg.data, size);

  return size;
}

int
fs_lseek (busword_t owner, busword_t hid, int off, int whence)
{
  mf_handle_t *handle;
  int final_off;
  int max_size;
  
  if ((handle = mf_handle_from_hid (owner, hid)) == NULL)
    return -EBADF;

  if (handle->file->type == MEMFILE_TYPE_DIR)
    max_size = handle->file->as_dir.file_count;
  else
    max_size = handle->file->as_reg.size;

  switch (whence)
  {
  case SEEK_SET:
    final_off = off;
    break;

  case SEEK_CUR:
    final_off = handle->ptr + off;
    break;

  case SEEK_END:
    final_off = max_size + off;
    break;

  default:
    return -EINVAL;
  }

  if (final_off < 0)
    final_off = 0;
  else if (final_off > max_size)
    final_off = max_size;

  handle->ptr = final_off;
  
  return final_off;
}


void
_start (void)
{
  struct fs_msg msg;
  int result;

  if (init_fs_tree () == -1)
  {
    puts ("fs: no memory left\n");
    exit (1);
  }
  
  declare_service ("fs");

  puts ("fs: atomik filesystem service started - version 0.1\n");
  
  while ((result = msgread (&msg, sizeof (struct fs_msg), 0)) != -ENOSYS)
  {
    puts ("fs: received message (type: ");
    puti (msg.fm_header.mh_type);
    puts (")\n");

    if (msg.fm_header.mh_type == 2)
      debug_fs_tree (root);
  }

  puts ("fs exited, result: ");
  puti (result);
  puts ("\n");
  exit (0);
}
