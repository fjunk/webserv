#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdarg.h>

#include "utils.h"

/* function prototypes */
void do_get(const char *res_path, int fd);


/* STRUCTS */
typedef struct Dir_Ele {
    char str [200];
    char as_link [300];
    int len;
    struct Dir_Ele *next;
} DIR_ELE;


/* HEADER TEMPLATES (T-prefix) */
const char *T_STATUS_400 = "HTTP/1.0 404 Not Found\r\n";
const char *T_STATUS_200 = "HTTP/1.0 200 OK\r\n";
const char *T_MIME_TYPE  = "Content-Type: %s\r\n";
const char *T_CONT_LEN   = "Content-Length: %d\r\n\r\n";

/* HTML TEMPLATES */
const char *T_LIST_ITEM = "<li>%s</li>";
const char *T_HTML_LINK = "<a href=\"%s\">%s</a>";
const char *T_HTML_DIR_LINK = "<a href=\"%s/\">%s</a>";

enum { ERR=-1, DIR_=0, EXEC=1, NORM=2 };

/**
 * Writes a representation of 'dir' to given FileDescriptor fd as
 * a http response with appropriate header. 
 *
 * The Content consists of a html with an unordered list (ul) with
 * list entries for each file or folder in the directory.
 **/
void send_dir(const char *abs_res_path, const char *res_path, int fd) {

    char *mime_type = "text/html";
    struct dirent *entry;
    DIR_ELE *ele, *head; /* TODO rename */
    int n_dir_entries = 0;
    int content_length = 0;
    char *ul_start = "<html><head></head><body><ul>";
    char *ul_end = "</ul></body></html>";
    DIR *dir = opendir(abs_res_path);

    /* Status line */
    write(fd, T_STATUS_200, strlen(T_STATUS_200));
    /* Content type */
    dprintf(fd, T_MIME_TYPE, mime_type);

    /* read dir entries into mem */
    head = malloc(sizeof(DIR_ELE));
    ele = head;
    entry = readdir(dir);

    if(entry) {

        /* hide dotfiles */
        if (strcmp(entry->d_name, ".") != 0) {
            n_dir_entries += 1;
            if(entry->d_type == DT_DIR){
                sprintf(ele->as_link, T_HTML_DIR_LINK, entry->d_name, entry->d_name);
            } else {
                sprintf(ele->as_link, T_HTML_LINK, entry->d_name, entry->d_name);
            }
            /* copy name of dir-entry */
            strcpy(ele->str, entry->d_name);
            content_length += snprintf(NULL, 0, T_LIST_ITEM, ele->as_link);
            ele->next = NULL;
        }

        /* append to custom list of dir entries */
        while((entry = readdir(dir))){
            /* hide dotfiles */
            if (strcmp(entry->d_name, ".") != 0) {
                ele->next = malloc(sizeof(DIR_ELE));
                ele = ele->next;
                if(entry->d_type == DT_DIR){
                    sprintf(ele->as_link, T_HTML_DIR_LINK, entry->d_name, entry->d_name);
                } else {
                    sprintf(ele->as_link, T_HTML_LINK, entry->d_name, entry->d_name);
                }
                content_length += snprintf(NULL, 0, T_LIST_ITEM, ele->as_link);
                ele->next = NULL;
                n_dir_entries += 1;
            }
        }
    }
    closedir(dir);

    /* content length */
    content_length += strlen(ul_start) + strlen(ul_end);
    dprintf(fd, T_CONT_LEN, content_length);

    /* write content (unordered list) & free nodes */
    write(fd, ul_start, strlen(ul_start));
    while(head->next){
        dprintf(fd, T_LIST_ITEM, head->as_link);
        ele = head;
        head = head->next;
        free(ele);
    }

    dprintf(fd, T_LIST_ITEM, head->as_link);
    free(head);

    write(fd, ul_end, strlen(ul_end));
    printf("\n");
}

void send_error(const char *res_path, int fd){

    char *content = "<html>%s does not exist / is not supported!</html>\n";
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

    if((mime_type = get_mime_type(res_path)) == NULL ) {
        send_error(res_path, fd);
    }

    /* Status line */
    write(fd, T_STATUS_200, strlen(T_STATUS_200));
    /* Content type */
    dprintf(fd, T_MIME_TYPE, mime_type);
    /* Content length */
    printf("%s\n", res_path);
    read_fd = open(res_path, O_RDONLY);
    if (read_fd < 0) {
        perror("Open-Error"); exit(-1);
    }
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

/* executes given file and pipes stdin and stdout to client_fd */
void exec_file(const char *res_path, int fd) {
    /* prepare args for execv */
    const char *argv[64] = {res_path, NULL};

    /* bind stdin (0) and stdout (1) to client_fd  */
    if (dup2(fd, 0) == -1 || dup2(fd, 1) == -1 ) 
        perror("Error while redirecting input / output");

    /* execute file */
    if (-1 == execv(argv[0], (char **)argv)) {
        perror("Error while trying to execute file");
    }
}

int filetype(const char *abs_path) {
    struct stat fs;
    DIR* dir;
    int file_type = -1;

    if (access(abs_path, F_OK) == -1 ) {
        file_type = ERR;
    } /* file does not exist */
    else if ((dir = opendir(abs_path)) != NULL) {
        closedir(dir);
        file_type = DIR_;
    } /* file is direcotry */
    else if (stat(abs_path, &fs) == 0 && fs.st_mode & S_IXUSR) {
        file_type = EXEC;
    } /* file is exectuable */
    else {
        file_type = NORM;
    } /* file is normal file */
    return file_type;
}


void do_get(const char *res_path, int fd){
    char *abs_path;
    char *cwd;
    int path_len;

    cwd = getcwd(NULL, 0);
    /* could delete the '/' to avoid double slashes but since they
     * are no problem i'll leave it there. */
    path_len = snprintf(NULL, 0, "%s/%s", cwd, res_path) + 1;
    abs_path = malloc(path_len);
    sprintf(abs_path, "%s%s", cwd, res_path);

    switch(filetype(abs_path)) {
        case DIR_:  send_dir(abs_path, res_path, fd); break;
        case EXEC:  exec_file(abs_path, fd); break;
        case NORM:  send_file(abs_path, fd); break;
        case ERR:   send_error(abs_path, fd); break;
        default: printf("Error detecting filetype\n");
    }

    free(abs_path);
    free(cwd);
}

