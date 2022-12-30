#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> 
#include "myio.h" 
#include "mybuffer.h"

/* The function takes a pointer to a file path and flags (access modes 
   {r0, wo,rw}). It creates a file if it does not exist, and returns 
   an open file given the correct access mode. Returns an error if user
   has no permissions to access/modify the file */
int myopen(const char *pathname, int flags){
    int fd;
    if ((fd = open(pathname, flags, (mode_t)0666)) < 0){
        return -1;
    }
    if (make_buffer(fd, flags) == NULL){
        myclose(fd);
        return -1;
    }
    return fd;
}

/* The function takes a file descriptor as input. 
   Closes a file that is open and frees the memory 
   occupied by the file in the open files buffer
   It returns an error if the file cannot be closed */
int myclose(int fd) {
    myflush(fd);
    if (close(fd) < 0){
        return -1;
    }
    free_buffer(fd);
    return 0;
}

/* The function takes a file descriptor, a pointer to a free buffer and 
   the size of file read so far as inputs. It then reads bytes 
   specified by count from the source file (fd) and puts them into 
   the buffer. It reurns the number of bytes read that are ready 
   to be written */
ssize_t myread(int fd, void *buf, size_t count){
    struct mybuffer *b = get_buffer(fd); /* get a free chunk of memory to use */
    if (b == NULL){
        return -1;
    }
    ssize_t w;
    int x = 0;
    for (w = 0; w < count; w += x) {
        x = b->cached_len < count - w ? b->cached_len : count - w;
        memcpy((char*)buf + w, b->buffer + b->local_pos, x);
        b->local_pos += x;
        b->cached_len -= x;
        if (b->cached_len == 0) {
            flush_buffer(fd);
            int r;
            if ((r = read(fd, b->buffer, MYBUFFERSIZE)) <= 0) {
                w += x;
                break;
            }
            b->cached_len += r;
        }
    }
    return w;
}

/*The function takes a file descriptor, a pointer to a buffer and 
  the size of bytes to be written. It returns the number of bytes written
  into the file specified by the file descriptor*/
ssize_t mywrite(int fd, const void *buf, size_t count){
    struct mybuffer *b = get_buffer(fd); /*retrieve the buffer where the file is opened*/
    if (b == NULL){
        return -1;
    }
    if (b->mode == O_RDWR && b->cached_len == 0) {
        char a;
        if (myread(fd, &a, 1) < 0) {
            return -1;
        }
        b->local_pos = 0;
        b->cached_len = MYBUFFERSIZE;
    }
    ssize_t r;
    int x = 0;
    for (r = 0; r < count; r += x) {
        x = MYBUFFERSIZE - b->local_pos < count - r ? MYBUFFERSIZE - b->local_pos : count - r;
        memcpy(b->buffer + b->local_pos, (char*)buf + r, x);
        b->local_pos += x;
        if (b->mode == O_RDWR) {
            b->cached_len -= x;
        }
        if (b->local_pos == MYBUFFERSIZE) {
            if (flush_buffer(fd) < 0) {
                return -1;
            }
        }
    }
    return r;
}

/*The function takes a file descriptor, the size of bytes to be written
  and the position in the heap as inputs. It returns the correct position
  of the client pointer in the heap*/
off_t myseek(int fd, off_t offset, int whence){
    struct mybuffer *b = get_buffer(fd); /*retrieve the correct buffer*/
    if (b == NULL){
        return 0;
    }
    off_t o; /* offset */
    myflush(fd);
    if ((o = lseek(fd, offset, whence)) == -1) {
        return -1;
    }
    return o - MYBUFFERSIZE;
}

/* Takes a file descriptor as input. It writes bytes read from the 
   buffer into the file specified by the fd. It also updates the 
   client pointer to a the last byte written*/
void myflush(int fd){
    struct mybuffer *b = get_buffer(fd);
    if (b == NULL){
        return;
    }
    if (flush_buffer(fd) < 0) {
        return;
    }
}
