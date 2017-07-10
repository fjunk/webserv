#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include "utils.h"


const char *MIME_DICT [][2]= {
    {".html", "text/html"},
    {".txt", "text/plain"}, 
    {".c", "text/plain"}, 
    {".png", "image/png"}, 
    {".jpg", "image/jpeg"}, 
    {".jpeg", "image/jpeg"}, 
    {NULL, NULL}
};

void log_request(struct request* request) {
    /* int fd = open("server.log", O_WRONLY | O_CREAT | O_APPEND, 0644); */

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    /*write(fd, str, strlen(str)); */
}


/* Takes a string containing a request and a request-struct and fills
 * the struct with the information contained in the string */
void split_request(const char *request_str, struct request *request) {

    int ress_idx, prot_idx, i;
    int str_len = strlen(request_str);

    /* calculate start index of next token */
    ress_idx = next_token_idx(request_str, 0);
    prot_idx = next_token_idx(request_str, ress_idx);
    /* set values to request struct */
    request->req_str = strdup(request_str);

    /* replace all spaces in the base-str with \0 to 
     * be able to read the substrings as strings since they
     * are terminated */
    for ( i = 0; i < str_len; i++ ) {
        if(request->req_str[i] == ' ')
            request->req_str[i] = '\0';
    }

    request->method   = request->req_str;
    request->ressource = percent_decode((request->req_str) + ress_idx);
    request->protocol = (request->req_str) + prot_idx;

}

/* takes a string and an start index .Finds the next occurence
 * of a space and returns its index in the given string */
int next_token_idx(const char *str, int start_idx) {
    int i;
    int str_len = strlen(str);

    for ( i = start_idx; i < str_len; i++ ) {
        if(str[i] == ' ') return i + 1;
    }
    
    return -1;
}

/* decodes a percent-encoded string and returns the decoded
string. The caller has to free the memory. */
char* percent_decode(const char* str){

    int i, j = 0;
    int str_len = strlen(str);
    int decoded_len = str_len;
    char *decoded;
    char hex_str[3];
    char ascii_char;

    /* calculate length of return-string */
    for(i=0; i<str_len; i++){
        if(str[i] == '%') {
            decoded_len -= 2;
        }
    }

    /* allocate mem for new string */
    decoded = malloc((decoded_len + 1) * sizeof(char));

    /* actual decoding */
    for(i=0; i<=str_len; i++){
        /* decode if %-encoded string is found */
        if(str[i] == '%'){
            hex_str[0] = str[++i];
            hex_str[1] = str[++i];
            hex_str[2] = '\0';
            ascii_char = (int)strtol(hex_str, NULL, 16);
        } else {
            ascii_char = str[i];
        }
        /* write either decoded char or normal char to decoded */
        decoded[j++] = ascii_char;
    }

    return decoded;
}


/* finds the matching mime_type to a file given by its extension */
const char *get_mime_type(const char *path) {

    const char **tuple = MIME_DICT[0];
    const char *ext = strrchr(path, '.');
    const char *mime_type = NULL;
    enum{EXT, MIME};

    /* if no extension was found in path */
    if(!ext) return NULL;

    /* find matching mime tipe to found extension */
    while(tuple[EXT]){
        if(strcmp(ext, tuple[EXT]) == 0)
            mime_type = tuple[MIME];
        /* move to next mime-type (+= sizeof(MIME_DICT)) */
        tuple++;
    }
    return mime_type;
}

/*
void main() {

    const char *test_request = "GET%20/index.html%20HTTP/1.0";
    struct request* req = malloc(sizeof(struct request));

    split_request(test_request, req);

    printf("%s\n", req->req_str);
    printf("%s\n", req->method);
    printf("%s\n", req->ressource);
    printf("%s\n", req->protocol);
    
    printf("%s\n", test_request);

    free(req->req_str);
    free(req);

}
*/
