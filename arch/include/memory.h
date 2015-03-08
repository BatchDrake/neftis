#ifndef _MEMORY_H
#define _MEMORY_H

#include <cpu.h>

#define PAGE_SIZE (1 << __PAGE_BITS)
#define PAGE_MASK (PAGE_SIZE - 1)
#define PAGE_OFFSET(page) ((busword_t) (page) & PAGE_MASK)
#define PAGE_START(page)  ((busword_t) (page) & ~PAGE_MASK)
#define PAGE_NO(page) (PAGE_START (page) >> __PAGE_BITS)
#define PAGE_ADDR(num) ((num) << __PAGE_BITS)

#endif /* _MEMORY_H */
