AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS=-I ../../include/ -I.

noinst_LIBRARIES=libdefsched.a

libdefsched_a_CFLAGS=-fstrength-reduce \
		-finline-functions -nostdinc -nostdlib -fno-builtin -I. -ggdb -march=i686 \
		-D__386__ -I../../include @AM_CFLAGS@

libdefsched_a_CCASFLAGS=-DASM -D__386__ @AM_CCASFLAGS@

libdefsched_a_SOURCES=defsched.c defsched.h
