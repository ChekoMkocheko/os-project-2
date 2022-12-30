#ifndef __MYIO_H
#define __MYIO_H

#include <sys/types.h>

int myopen(const char *pathname, int flags);

int myclose(int fd);

ssize_t myread(int fd, void *buf, size_t count);

ssize_t mywrite(int fd, const void *buf, size_t count);

off_t myseek(int fd, off_t offset, int whence);

void myflush(int fd);

#endif
