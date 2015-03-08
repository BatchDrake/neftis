AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS=-Iinclude

noinst_LIBRARIES=libcommon.a

libcommon_a_CFLAGS=-fstrength-reduce \
                   -finline-functions -nostdinc -nostdlib -fno-builtin -I. -I../arch/include-@AM_ARCH@ -I../arch/include -ggdb -march=i686 \
                   -D__386__ @AM_CFLAGS@	

libcommon_a_CCASFLAGS=-DASM -D__386__ @AM_CCASFLAGS@

libcommon_a_SOURCES=rbtree.c \
	kmalloc.c \
	radix_tree.c \
	list.c \
	stdlib.c \
	hook.c \
	vkprintf.c \
	include/ctype.h  \
	include/alloc.h \
	include/ansi.h \
	include/kmalloc.h \
	include/string.h \
	include/config.h \
	include/defines.h \
	include/stdarg.h \
	include/util.h \
	include/misc \
	include/misc/errno.h \
	include/misc/tar.h \
	include/misc/vkprintf.h \
	include/misc/rbtree.h \
	include/misc/list.h \
	include/misc/radix_tree.h \
	include/misc/hook.h \
	include/debug.h \
	include/values.h

