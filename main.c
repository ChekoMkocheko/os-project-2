#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "mytests.h"

int main(int argc, char* argv[])
{
    printf("Starting mytests.\n");

    if (run_basic_tests() == 1){
        printf("All tests passed.\n");
    } else {
        printf("Some tests failed.\n");
    }
}
