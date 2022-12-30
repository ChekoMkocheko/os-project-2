#include "mybuffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// the starting size of the buffer array
#define MYBUFFARRSIZE 100

int mybuffers = 0; /* the number of buffers in the array */
int mybuffer_len = MYBUFFERSIZE; /* the length of the array */
struct mybuffer *mybuffer_arr;  /* the array */

/* This function takes file descriptor as input, returns 
   the buffer where the fd and its contents are stored  */
struct mybuffer *get_buffer(int fd){
    if (fd < 0 || fd >= mybuffer_len) {
        return NULL;
    }
    return &mybuffer_arr[fd];
}

/* This function takesa file descriptor as input and allocates
   memory in the heap for the associtated file. The buffer 
   which keeps track of the information for the file is created 
   and added to the Array of buffers  */


struct mybuffer *make_buffer(int fd, int mode){
    static int first_run = 1;

    if (first_run) {
        first_run = 0;
        if (!(mybuffer_arr = malloc(sizeof(struct mybuffer) * MYBUFFERSIZE))) {
            return NULL;
        }

    }

    if (mybuffers >= mybuffer_len) {
        struct mybuffer *new_mybuffer_arr; 
        if (!(new_mybuffer_arr = malloc(sizeof(struct mybuffer) * 2 * mybuffer_len))) {
            return NULL;
        }
        memcpy(new_mybuffer_arr, mybuffer_arr, sizeof(struct mybuffer) * mybuffer_len);
        mybuffer_len *= 2;
        free(mybuffer_arr);
        mybuffer_arr = new_mybuffer_arr;
    }

    struct mybuffer b;

    b.local_pos = 0; 
    b.cached_len = 0;
    b.mode = mode & O_ACCMODE;
    b.fd = fd;

    mybuffer_arr[fd] = b;
    mybuffers++;

    return &mybuffer_arr[fd];
}

/* This function takes a file descriptor as input and frees the 
   memory allocated for the associated file in the heap */

int free_buffer(int fd){
    struct mybuffer *b = get_buffer(fd);
    if (b == NULL){
        return -1;
    }
    mybuffers--;
    return 1;
}

int flush_buffer(int fd) {
    struct mybuffer *b = get_buffer(fd);
    if (b == NULL) {
        return -1;
    }
    int w = 0;
    int old_cached = b->cached_len;
    if (b->mode == O_RDWR) {
        if (lseek(fd, -get_buffer_len(fd), SEEK_CUR) < 0) {
            return -1;
        }
        if ((w = write(fd, b->buffer, get_buffer_len(fd))) < 0) {
            return -1;
        }
        if (lseek(fd, -old_cached, SEEK_CUR) < 0) {
            return -1;
        }
    }
    else if (b->mode == O_RDONLY) {
        if (lseek(fd, -b->cached_len, SEEK_CUR) < 0) {
            return -1;
        }
    }
    else if (b->mode == O_WRONLY) {
        if ((w = write(fd, b->buffer, get_buffer_len(fd)) < 0) < 0) {
            return -1;
        }
    }
    b->local_pos = 0;
    b->cached_len = 0;
    return w;
}

int get_buffer_len(int fd) {
    struct mybuffer *b = get_buffer(fd);
    if (b == NULL) {
        return -1;
    }
    return b->local_pos + b->cached_len;
}

int get_buffer_free(int fd) {
    return MYBUFFERSIZE - get_buffer_len(fd);
}

int is_buffer_full(int fd) {
    return get_buffer_len(fd) >= MYBUFFERSIZE;
}
