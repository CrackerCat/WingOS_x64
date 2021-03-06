#pragma once

#define NULL 0
typedef unsigned long size_t;

void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *p);
void operator delete[](void *p);

#ifdef __STRICT_ANSI__
#define wfinline __attribute__((unused))
#else
#define wfinline __attribute__((always_inline))
#endif
