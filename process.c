#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#include "utils.h"


const char *LINE_BREAK = "\r\n";
const char *DOUBLE_LINE_BREAK ="\r\n\r\n";


char *read_request(int read_fd){

    enum { BUFFSIZE=1000 };
    char buffer[BUFFSIZE] = {1};

    char *db_break_ptr;
    char *break_ptr;
    char *new_str;

    int byte_read; 

    while(1) {
        byte_read = read(read_fd, buffer+3, BUFFSIZE-4);
        buffer[byte_read-1] = '\0';
        printf("%s\n", buffer);

        if (byte_read > 0) {
            break_ptr = strstr(buffer, LINE_BREAK);

            if (break_ptr != NULL) {
                new_str = strdup(break_ptr);
            }
            
            if (strstr(buffer, DOUBLE_LINE_BREAK)) {
                return new_str;
            }

        } else if (byte_read < 0) {
            perror ("readerror");
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
    free(str);
    free(req->req_str);
    free(req);

    return 0;
}
