
struct request {
    char *req_str;     /* the whole request str */
    char *method;      /* http method i.e. GET */
    char *ressource;   /* requested file, i.e. index.html */
    char *protocol;    /* http protocol version i.e. HTPP/1.0 */
};

char* percent_decode(const char* str);
int next_token_idx(const char *str, int start_idx);
void split_request(const char*request_str, struct request *request);

