// Dan Saada 208968560

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * Compares two files character by character and returns an integer value indicating their similarity.
 *
 * @param fd1 The file descriptor of the first file.
 * @param fd2 The file descriptor of the second file.
 *
 * @return 1 if the files are identical, 2 if the files are different, and 3 if the files are similar.
 */
int compareFiles(int fd1, int fd2) {
    char buffer1, buffer2;
    ssize_t bytesRead1, bytesRead2;
    int is_similar = 0, i=0, j=0;

    while (1) {
        i=0;
        j=0;
        // Read characters from fd1 while ignoring whitespaces and line breaks
        do {
            bytesRead1 = read(fd1, &buffer1, 1);
            i++;
        } while (bytesRead1 > 0 && isspace(buffer1));

        // Read characters from fd2 while ignoring whitespaces and line breaks
        do {
            bytesRead2 = read(fd2, &buffer2, 1);
            j++;
        } while (bytesRead2 > 0 && isspace(buffer2));

        if(bytesRead1 != bytesRead2){return 2;}
        // Compare characters case-insensitively
        if (buffer1 != buffer2 && tolower(buffer1) == tolower(buffer2)){is_similar = 1;}
        // whitespaces and line breaks identification
        if ((i > 1 || j > 1) && i != j) {is_similar = 1;}

        if ((bytesRead1 != bytesRead2 || buffer1 != buffer2)&& is_similar==0) {
            return 2; // different
        }

        // EOF
        if (bytesRead1 == 0 && bytesRead2 == 0) {
            return is_similar ? 3 : 1; // similar or identical
        }
    }
}

/**
 * Prints an error message to STDERR indicating that a system call failed and exits the program with -1.
 *
 * @param sysCall Name of the system call that failed.
 */
void sysCallError(const char* sysCall) {
    char errMsg[50];
    sprintf(errMsg, "Error in: %s\n", sysCall);
    write(STDERR_FILENO, errMsg, strlen(errMsg));
    exit(-1);
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 2;
    }

    // Open the fds
    int fd1 = open(argv[1], O_RDONLY);
    if (fd1 == -1) {
        sysCallError("open");
    }

    int fd2 = open(argv[2], O_RDONLY);
    if (fd2 == -1) {
        if(close(fd1) == -1) {
            sysCallError("close");
        }
        sysCallError("open");
    }

    // Get result of the files comparing
    int result = compareFiles(fd1, fd2);

    // Close the fds
    if(close(fd1) == -1) {
        if(close(fd2) == -1) {
            sysCallError("close");
        }
        sysCallError("close");
    }
    if(close(fd2) == -1) {
        sysCallError("close");
    }
    return result;
}

