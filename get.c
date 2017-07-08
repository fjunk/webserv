#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdarg.h>

#include "utils.h"

/* function prototypes */
void do_get(const char *res_path, int fd);


/* STRUCTS */
typedef struct Dir_Ele {
    char str [200];
    int len;
    struct Dir_Ele *next;
} DIR_ELE;


/* HEADER TEMPLATES (T-prefix) */
const char *T_STATUS_400 = "HTTP/1.0 404 Not Found\n";
const char *T_STATUS_200 = "HTTP/1.0 200 OK\n";
const char *T_MIME_TYPE  = "Content-Type: %s\n";
const char *T_CONT_LEN   = "Content-Length: %d\n\n";

/* LIST TEMPLATES */
const char *T_LIST_ITEM = "<li>%s</li>";


/**
 * Writes a representation of 'dir' to given FileDescriptor fd as
 * a http response with appropriate header. 
 *
 * The Content consists of a html with an unordered list (ul) with
 * list entries for each file or folder in the directory.
 **/
void send_dir_content(DIR *dir, int fd) {

    char *mime_type = "text/html";
    struct dirent *entry;
    DIR_ELE *ele, *head; /* TODO rename */
    int n_dir_entries = 0;
    int content_length = 0;
    char *ul_start = "<html><ul>";
    char *ul_end = "</ul></html>";

    /* Status line */
    write(fd, T_STATUS_200, strlen(T_STATUS_200));
    /* Content type */
    dprintf(fd, T_MIME_TYPE, mime_type);

    /* read dir entries into mem */
    head = malloc(sizeof(DIR_ELE));
    ele = head;
    entry = readdir(dir);

    if(entry){

        n_dir_entries += 1;
        sprintf(ele->str, T_LIST_ITEM, entry->d_name);
        content_length += strlen(ele->str);
        ele->next = NULL;

        while((entry = readdir(dir))){
            ele->next = malloc(sizeof(DIR_ELE));
            ele = ele->next;
            sprintf(ele->str, T_LIST_ITEM, entry->d_name);
            content_length += strlen(ele->str);
            ele->next = NULL;
            n_dir_entries += 1;
        }

    }

    /* content length */
    content_length += strlen(ul_start) + strlen(ul_end);
    dprintf(fd, T_CONT_LEN, content_length);

    /* write content (unordered list) & free nodes */
    write(fd, ul_start, strlen(ul_start));
    while(head->next){
        write(fd, head->str, strlen(head->str));
        ele = head;
        head = head->next;
        free(ele);
    }

    write(fd, head->str, strlen(head->str));
    free(head);

    write(fd, ul_end, strlen(ul_end));
    printf("\n");
}

void send_error(const char *res_path, int fd){

    char *content = "<html>%s does not exist!</html>\n";
    char *mime_type = "text/html";
    int cont_len;

    /* Status line */
    write(fd, T_STATUS_400, strlen(T_STATUS_400));
    /* Content type */
    dprintf(fd, T_MIME_TYPE, mime_type);
    /* Content length */
    cont_len = snprintf(NULL, 0, content, res_path);
    dprintf(fd, T_CONT_LEN, cont_len);
    /* Content */
    dprintf(fd, content, res_path);
}



void send_file(const char *res_path, int fd){

    const char *mime_type;
    struct stat s;
    int cont_len;

    enum{ BUFFSIZE=1000 };
    char buffer[BUFFSIZE];
    int bytes_read, bytes_written;
    int read_fd;

    if( !(mime_type = get_mime_type(res_path)) ) return;

    /* Status line */
    write(fd, T_STATUS_200, strlen(T_STATUS_200));
    /* Content type */
    dprintf(fd, T_MIME_TYPE, mime_type);
    /* Content length */
    read_fd = open(res_path, O_RDONLY);
    fstat(read_fd, &s);
    cont_len = s.st_size;
    dprintf(fd, T_CONT_LEN, cont_len);

    /* Content (copy buff by buff) */
    while(1) {
        bytes_read = read(read_fd, buffer, BUFFSIZE);
        if (bytes_read == 0) {
            break;
        } else if (bytes_read < 0) {
            perror("Read-Error");
            break;
        }

        bytes_written = write(fd, buffer, bytes_read);
        if(bytes_written <= 0){
            perror("Write-Error");
            exit(4);
        }
    }

    close(read_fd);
}

void do_get(const char *res_path, int fd){
    char *abs_path;
    char cwd[1024];
    DIR* dir;

    getcwd(cwd, sizeof(cwd));
    abs_path = malloc(strlen(cwd) + strlen(res_path) + 2);
    sprintf(abs_path, "%s/%s", cwd, res_path);

    if(access(abs_path, F_OK) == -1) {
        /* file does NOT exists */
        send_error(abs_path, fd);
    } else {
        if((dir = opendir(abs_path))){
            /* res_path describes a dir */
            send_dir_content(dir, fd);
            closedir(dir);
        } else {
            /* res_path describes a file */
            send_file(abs_path, fd);
        }
    }

    free(abs_path);
}


