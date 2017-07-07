#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"


/* Takes a string containing a request and a request-struct and fills
 * the struct with the information contained in the string */
void split_request(const char *request_str, struct request *request) {

    char *decoded_request = percent_decode(request_str);

    int ress_idx, prot_idx, i;
    int str_len = strlen(decoded_request);

    /* calculate start index of next token */
    ress_idx = next_token_idx(decoded_request, 0);
    prot_idx = next_token_idx(decoded_request, ress_idx);

    /* set values to request struct */
    request->req_str  = strdup(decoded_request);
    request->method   = request->req_str;
    request->ressource = (request->req_str) + ress_idx;
    request->protocol = (request->req_str) + prot_idx;

    free(decoded_request);
    
    /* replace all spaces in the base-str with \0 to 
     * be able to read the substrings as strings since they
     * are terminated */
    for ( i = 0; i < str_len; i++ ) {
        if(request->req_str[i] == ' ')
            request->req_str[i] = '\0';
    }
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
