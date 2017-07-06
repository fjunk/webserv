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
 * move last 3 chars to beginning of new read */

char *LINE_BREAK = "\r\n";




void process_request(int fd_in, int fd_out) {


}


