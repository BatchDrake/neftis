AUTOMAKE_OPTIONS = subdir-objects
noinst_PROGRAMS=auth.asx

auth_asx_LDFLAGS=-nostdlib -static
auth_asx_LDADD=../../user/libuser.a
auth_asx_CFLAGS=-fstrength-reduce -finline-functions -nostdinc -nostdlib -fno-builtin -I. -I../../user/include -I../../arch/include -I../../arch/include-x86 -ggdb -Iinclude -march=i686 -D__386__ @AM_CFLAGS@ 

auth_asx_SOURCES=main.c

