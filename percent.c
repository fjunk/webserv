#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "percent.h"

/* decodes a percent-encoded string and returns the decoded
string. The caller has to free the memory. */
char* percent_decode(const char* input){

    int i, j = 0;
    int input_len = strlen(input);
    int output_len = input_len;
    char *output;
    char hex_str[3];
    char ascii_char;

    /* calculate length of return-string */
    for(i=0; i<input_len; i++){
        if(input[i] == '%') {
            output_len -= 2;
        }
    }

    /* allocate mem for new string */
    output = malloc((output_len + 1) * sizeof(char));

    /* actual deoding */
    for(i=0; i<=input_len; i++){
        /* decode if %-encoded string is found */
        if(input[i] == '%'){
            hex_str[0] = input[++i];
            hex_str[1] = input[++i];
            hex_str[2] = '\0';
            ascii_char = (int)strtol(hex_str, NULL, 16);
        } else {
            ascii_char = input[i];
        }
        /* write either decoded char or normal char to output */
        output[j++] = ascii_char;
    }

    return output;
}

char* percent_encode(const char* input, const char* to_encode){
    return "encode";
}
