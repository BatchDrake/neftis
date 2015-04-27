AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES=libuser.a

libuser_a_CFLAGS=-fstrength-reduce -finline-functions -nostdinc -nostdlib -fno-builtin -Iinclude -ggdb -I../arch/include -I../arch/include-x86 -I../microkernel/loaders/elf -march=i686 -D__386__ @AM_CFLAGS@ 
    
libuser_a_SOURCES=\
ipc.c \
malloc.c \
stdlib.c \
system.c \
u_util.c \
include/atomik.h \
include/ctype.h \
include/errno.h \
include/malloc.h \
include/stdlib.h \
include/stddef.h \
include/u_util.h
