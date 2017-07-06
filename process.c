#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

/** 
 *
 *  GET /index... HTTP1... \r\n 
 *  ... 
 *  ... 
 *  \r\n\r\n
 *
 */

/* strstr find substring 
 * move last 3 chars to beginning of new read 
 * buffersize = 1003
 * terminate buffer with \0  */

const char *LINE_BREAK = "\r\n";
const char *DOUBLE_LINE_BREAK ="\r\n\r\n";




void read_request(int read_fd){

    enum { BUFFSIZE=1000 };
    char buffer[BUFFSIZE];
    char *str_end;
    int did_read;

    while (1) {

        did_read = read(read_fd, buffer, BUFFSIZE);

        if (did_read == 0) {

            str_end = strstr(buffer, LINE_BREAK);
            if(str_end) {
                *str_end = '\0';
                printf("%s\n", buffer);
            }
            break;
        } else if (did_read < 0) {
            perror("readerror");
        }
    }
}


void main() {

    int read_fd = open("input.txt", O_RDONLY);
    read_request(read_fd);

}
