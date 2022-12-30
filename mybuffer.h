#ifndef __MYBUFFER_H
#define __MYBUFFER_H

#include <stddef.h>

#define MYBUFFERSIZE 1000

struct mybuffer
{
    char buffer[MYBUFFERSIZE];
    int local_pos; 
    int cached_len; 
    int mode; 
    int fd; 
};

// gets buffer from the array at the index fd
// if the fd is invalid returns NULL
struct mybuffer *get_buffer(int fd);

// creates buffer and adds it to the array
// if the array is full, it grows it
// returns the created buffer
struct mybuffer *make_buffer(int fd, int mode);

// removes buffer from arr
// returns 1 on sucess, fails otherwise
int free_buffer(int fd);

// flushes the buffer
// if the buffer is of mode write, then
// the bytes in the buffer are written
// in all cases the buffer is set to have
// no bytes in it.
// On failure -1 is returned,
// otherwise returns number of bytes written
int flush_buffer(int fd);

// gets amount of bytes stored in buf
int get_buffer_len(int fd);

// gets amount of free space in buffer
int get_buffer_free(int fd);

// returns whether the buffer is full
int is_buffer_full(int fd);

#endif
