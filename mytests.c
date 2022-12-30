#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "mytests.h"
#include "myio.h"
#include "testtext.c"

#define BUFFERSIZE 50

const char *f_out_name = "outfile";

int test_write() {
    int f_out;
    int l = strlen(testtext);
    int w = 0;

    if ((f_out = myopen(f_out_name, O_WRONLY | O_CREAT)) < 0) {
        printf("Failed to open files.\n");
        return 0;
    }

    w += mywrite(f_out, testtext, l);
    if (w < 0) {
        printf("Error writing\n");
        return 0;
    }
    myflush(f_out);

    if (l != w) {
        printf("Wrote %d bytes, expected %d.\n", w, l);
        return 0;
    }

    if (myclose(f_out) < 0) {
        printf("Failed to close files.\n");
        return 0;
    }
    
    return 1;
}

int stress_test() {
    int fds[1000];
    memset(fds, 0, 1000 * sizeof(int));
    for (int i = 0; i < 500; i++) {
        char buf[10];
        sprintf(buf, "file_%d", i);
        if (!(fds[i] = myopen(buf, O_WRONLY | O_CREAT))) {
            return -1;
        }
    }
    for (int i = 0; i < 500; i++) {
        if (fds[i] != 0) {
            if (mywrite(fds[i], "TEST", 4) < 4) {
                return -1;
            }
            myflush(fds[i]);
        }
    }
    for (int i = 0; i < 500; i++) {
        if (fds[i] != 0) {
            if (!(myclose(fds[i]))) {
                return -1;
            }
        }
    }
    return 1;
}

int test_read() {
    char buf[BUFFERSIZE];
    int r = 0;
    int t = 0;
    int f_out;
    if ((f_out = myopen(f_out_name, O_RDONLY)) < 0) {
        printf("Failed to open files.\n");
        return 0;
    }
    while ((r = myread(f_out, buf, BUFFERSIZE)) > 0) {
        if (strncmp(buf, testtext + t, r) != 0) {
            printf("Found difference between files.\n");
            return 0;
        }
        t += r;
    }
    if (myclose(f_out) < 0) {
        printf("Failed to close files.\n");
        return 0;
    }
    return 1;
}

int test_seek_set() {
    int f_out;
    if ((f_out = myopen(f_out_name, O_RDONLY)) < 0) {
        printf("Failed to open files.\n");
        return 0;
    }
    int indexes[10] = { 0, 1110, 40, 2270, 100, 500, 1300, 330, 3, 2000};
    char buf[BUFFERSIZE];

    for (int i = 0; i < 10; i++) {
        myseek(f_out, indexes[i], SEEK_SET);
        int r = myread(f_out, buf, BUFFERSIZE);
        if (strncmp(buf, testtext + indexes[i], r) != 0) {
            printf("Found difference between files\n");
            return 0;
        }
    }
    if (myclose(f_out) < 0) {
        printf("Failed to close files.\n");
        return 0;
    }
    return 1;
}

int test_seek_cur() {
    int f_out;
    if ((f_out = myopen(f_out_name, O_RDONLY)) < 0) {
        printf("Failed to open files.\n");
        return 0;
    }

    char buf[1];
    int r = 0;
    int inc = 30;
    for (int p = 0; p < 2000; p += inc + r) {
        r = myread(f_out, buf, 1);
        myseek(f_out, inc, SEEK_CUR);
        if (strncmp(buf, testtext + p, r) != 0) {
            printf("Found difference between files\n");
            return -1;
        }
    }

    if (myclose(f_out) < 0) {
        printf("Failed to close files.\n");
        return 0;
    }
    return 1;
}

// finds occurence of the str in the outfile
// returns -1 when str is not present
int find_occurence(char *str) {
    int f_out;
    if ((f_out = myopen(f_out_name, O_RDONLY)) < 0) {
        printf("Failed to open files.\n");
        return 0;
    }

    int l = strlen(str);
    char *buf = malloc(l);
    myread(f_out, buf, l-1);
    int p = 0;
    while (myread(f_out, buf+l-1, 1) > 0) {
        if (strncmp(buf, str, l) == 0) {
            printf("Found occurence of string\n");
            return p;
        }
        for (int i = l-1; i > 0; i--) {
            buf[i] = buf[i-1];
        }
        p++;
    }
    if (myclose(f_out) < 0) {
        printf("Failed to close files.\n");
        return 0;
    }
    return -1;
}

// changes each occurence of str to new_str
// strs must be same length
int replace(char *str, char *new_str) {
    if (strlen(str) != strlen(new_str)) {
        return 1;
    }

    int f_out;
    if ((f_out = myopen(f_out_name, O_RDWR)) < 0) {
        printf("Failed to open files.\n");
        return 0;
    }

    int l = strlen(str);
    char *buf = malloc(l);
    myread(f_out, buf, l-1);
    while (myread(f_out, buf+l-1, 1) > 0) {
        if (strncmp(buf, str, l) == 0) {
            myseek(f_out, -l, SEEK_CUR);
            mywrite(f_out, new_str, l);
            myflush(f_out);
        }
        for (int i = 0; i < l-1; i++) {
            buf[i] = buf[i+1];
        }
    }
    if (myclose(f_out) < 0) {
        printf("Failed to close files.\n");
        return 0;
    }
    return 1;
}

// tries to replace one word with another in the outfile
// it then checks to see if that word is still in the file
int test_rw() {
    int words = 1;
    char *strs[] = { "alias" };
    char *reps[] = { "ALIAS" };
    for (int i = 0; i < words; i++) {
        replace(strs[i], reps[i]);
        if (find_occurence(strs[i]) != -1) {
            printf("Failed to replace %s\n", strs[i]);
            return -1;
        }
    }
    return 1;
}

// tests the usual features
int run_basic_tests() {
    if (!test_write() || !test_read() || !test_seek_set() || !test_seek_cur() || !test_rw()) {
        return 0;
    }
    return 1;
}

// tests opening many files
int run_stress_test() {
    if (!stress_test()) {
        return 0;
    }
    return 1;
}
