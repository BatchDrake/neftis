AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS=-I ../../include/ -I.

noinst_LIBRARIES=libelf.a

libelf_a_CFLAGS=-fstrength-reduce \
		-finline-functions -nostdinc -nostdlib -fno-builtin -I. -ggdb -march=i686 \
		-D__386__ -I../../include @AM_CFLAGS@

libelf_a_CCASFLAGS=-DASM -D__386__ @AM_CCASFLAGS@

libelf_a_SOURCES=abi.c dyn.c elf.c elf.h elf_state.h
