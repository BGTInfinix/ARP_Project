#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc <= 1) {
         return EXIT_FAILURE;
    }

    pid_t processes[20];
    int numberOfProcess = argc -1;

    for(int i=1; i<argc; i++) {
        processes[i] = (pid_t) atoi(argv[i]);
    }
    //wait();
    return EXIT_SUCCESS;
}