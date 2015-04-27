AUTOMAKE_OPTIONS = subdir-objects
noinst_PROGRAMS=fs.asx

fs_asx_LDFLAGS=-nostdlib -static
fs_asx_LDADD=../../user/libuser.a
fs_asx_CFLAGS=-fstrength-reduce -finline-functions -nostdinc -nostdlib -fno-builtin -I. -I../../user/include -I../../arch/include -I../../arch/include-x86 -ggdb -Iinclude -march=i686 -D__386__ @AM_CFLAGS@ 

fs_asx_SOURCES=main.c include/memfs.h

