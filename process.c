#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#include "utils.h"

/** 
 *  GET /index... HTTP1... \r\n 
 *  ... 
 *  ... 
 *  \r\n\r\n
 */

/*
 * move last 3 chars to beginning of new read 
 * buffersize = 1003
 * terminate buffer with \0  */

const char *LINE_BREAK = "\r\n";
const char *DOUBLE_LINE_BREAK ="\r\n\r\n";




char *read_request(int read_fd){

    enum { BUFFSIZE=1000 };
    char buffer[BUFFSIZE] = {1};

    char *new_str = "";
    char *break_ptr;
    char *db_break_ptr;
    int byte_read; 

    buffer[BUFFSIZE-1] = '\0';
    /* read once to get the GET string */
    byte_read = read(read_fd, buffer, BUFFSIZE-1);

    if (byte_read > 0) {
            break_ptr = strstr(buffer, LINE_BREAK);
            db_break_ptr = strstr(buffer, DOUBLE_LINE_BREAK);

            if(break_ptr) {
                *break_ptr = '\0';
                new_str = strdup(buffer);
            }

            /* if the whole request has been read, return get-string */
            if(db_break_ptr) return new_str;

    } else if (byte_read < 0) {
        perror("readerror");
    }
    return new_str;

    /* TODO: read from buffer till doublelinebreak is present */
}

/*
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
    free(str);
    free(req->req_str);
    free(req);

    return 0;
}
*/
