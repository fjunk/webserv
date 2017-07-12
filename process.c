#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>

#include "utils.h"


const char *LINE_BREAK = "\r\n";
const char *DOUBLE_LINE_BREAK ="\r\n\r\n";


char *read_request(int read_fd){

    char *db_break_ptr;
    char *break_ptr;
    char *new_str;

    int byte_read,i ;
    int offset;

    const int BUFFSIZE = 205;
    char buffer[BUFFSIZE];

    /* init buffer */
    for (i=0; i<BUFFSIZE; i++) {
        buffer[i] = '_';
    }

    /* INT_MAX to avoid int overflow at some point... */
    for (i=0; i<INT_MAX; i++) {

        byte_read = read(read_fd, buffer+offset, BUFFSIZE-offset-1);
        printf("Byte read: %d\n", byte_read);
        buffer[byte_read+offset] = '\0';
        printf("%s\n", buffer);

        if (byte_read > 0) {
            if (0 == i){
                break_ptr = strstr(buffer, LINE_BREAK);
                *break_ptr = '\0';
                new_str = strdup(buffer+offset);
                *break_ptr = 'x';
            }

            if (strstr(buffer, DOUBLE_LINE_BREAK) != NULL) {
                return new_str;
            } else {
                /* copy last 3 chars (last char is \0) */
                buffer[0] = buffer[byte_read+0];
                buffer[1] = buffer[byte_read+1];
                buffer[2] = buffer[byte_read+2];
            }
        } else if (byte_read < 0) {
            perror ("readerror");
        } else {
            printf("Invalid HTML Request! Terminate with \\r\\n\\r\\n!\n");
            exit(-1);
        }
    }
}


int main() {

    int read_fd = open("input.txt", O_RDONLY);
    char *str = read_request(read_fd);
    struct request *req = malloc(sizeof(struct request));

    split_request(str, req);


    printf("%s\n", str);
    printf("%s\n", req->req_str);
    printf("%s\n", req->method);
    printf("%s\n", req->ressource);
    printf("%s\n", req->protocol);
    /*
    free(str);
    free(req->req_str);
    free(req);
    */

    return 0;
}
