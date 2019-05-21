#pragma once
#pragma warning(disable : 4996)

#include <windows.h>

static inline void usleep(unsigned usec)
{
    Sleep(usec / 1000);
}

int open(const char *name, int flags, ...);
void close(int fd);
int read(int fd, void *buf, unsigned len);
int write(int fd, const void *buf, unsigned len);


