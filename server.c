#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils.h"
#include "process.h"
#include "get.h"


#define MAX_CHILDS 17
#define FNAME_PID "server.pid"
#define LOGFILE "server.log"


/* PROTOTYPES */
void serve(int port);

/* GLOBALS */
int child_pids[MAX_CHILDS];
char cwd [1024];



/* removes the pid-file from current dir and kills all child processes */
void int_handler(int sig) {
    int i;

    /* kill children first */
    for(i=0; i<MAX_CHILDS; i++) {
        if(child_pids[i]){
            printf("killing child %d\n", child_pids[i]);
            if (kill(child_pids[i], SIGINT)) {
                perror("Kill failed\n");
            }
        }
    }
    /* remove PID-file */
    printf("removing %s!\n", FNAME_PID);
    /* change back to base-dir and remove the PID-file */
    chdir(cwd); remove(FNAME_PID);
    _exit(-1);
}

/* Child handler that catches SIGCHLD and removes the childs
 * PID from the list when it is finished */ 
void child_handler(int sig) {
    int i, pid, status; 

    pid = wait(&status);
    printf("child handler: removing %d\n", pid);

    /* remove pid from list and reset to 0 */
    for(i=0; i<MAX_CHILDS; i++) {
        if(child_pids[i] == pid){
            child_pids[i] = 0; break;
        }
    }
}


/* saves the PID of the running program in the current directory */
void write_pid() {
    int fd = open(FNAME_PID, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int pid = getpid();
    char s_pid [10];

    sprintf(s_pid, "%d", pid);
    write(fd, s_pid, strlen(s_pid));
    close(fd);
}


/* reads the PID from the pid-file in the current dir and returns it 
 * if an error occurred while opening / reading the file, -1 is returned.
 */
int read_pid() {
    int fd = 0;
    int len = 0;
    char s_pid [10];

    fd = open(FNAME_PID, O_RDONLY);
    if (fd == -1) { return -1; }

    len = read(fd, s_pid, 10);
    if (len <= 0) { perror("error reading FNAME_PID!"); exit(-1); }

    s_pid[len] = '\n';

    return atoi(s_pid);
}


void start(const char* base_dir, const int port) {
    write_pid();
    /* in case we want to change the diretory, 
     * save where the pid-file got written to */
    if (base_dir != NULL) {
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
        }
        if (chdir(base_dir) == -1 ) 
            { perror("Could not change directory"); exit(-1); }
    }
    /* save server pid to filesystem */
    serve(port);
}


void print_status() {
    int pid = read_pid();
    /* order of the checks is important since kill(-1, x) is NO GUCCI */
    if (pid != -1 && kill(pid, 0) == 0) {
        printf("Server is running with pid %d!\n", pid);
    } else {
        printf("Server is NOT running!\n");
    }
}

void stop() {
    int pid = read_pid();
    /* could not find file with pid -> server not running */
    if ( pid == -1 || kill(pid, 0) != 0) {
        printf("Server not running / pid %d does not exist", pid);
        exit(0);
    }
    /* server is running: stop it */
    kill(pid, SIGINT);
}


void serve(int port) {

    int i, child_pid;
    int server_sockfd, client_sockfd;
    socklen_t client_len;
    struct sockaddr_in server_addr, client_addr;
    struct request *request;
    char *request_str;

    /* set type, port and listen on all interfaces */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 

    /* INIT SOCKET */
    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error"); exit(-1);
    }

    /* BIND SOCKET */
    if (bind(server_sockfd, (struct sockaddr*) &server_addr, 
                sizeof(struct sockaddr_in)) < 0) {
        perror("bind error"); exit(-1);
    }

    /* TEST SOCKET */
    if (listen(server_sockfd, 5) < 0) { 
        perror("listen error"); 
        exit(-1);
    }

    /* install SIGCHLD handler to respond to finishing childs */
    signal(SIGCHLD, child_handler);
    signal(SIGINT, int_handler);

    while(1) {
        client_len = sizeof(struct sockaddr);
        client_sockfd = accept(server_sockfd,
                (struct sockaddr *) &client_addr, &client_len);

        if (client_sockfd < 0) { perror("accept error"); exit(-1); }

        child_pid = fork();

        if (child_pid != 0) {
            for(i=0; i<MAX_CHILDS; i++) {
                if( ! child_pids[i]) {
                    child_pids[i] = child_pid;
                    break;
                }
            }
        }

        if (child_pid == 0) {

            /* close parent-socket */
            close(server_sockfd);

            /* reset signal handlers to Default for childs */
            signal(SIGINT, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);

            printf("processing request...\n");

            /* alloc space for request-struct */
            request = malloc(sizeof(struct request));

            /* read from socket and split GET-line into struct */
            request_str = read_request(client_sockfd);
            split_request(request_str, request);
            printf("Request; %s\n", request->ressource);

            /* get ressource */
            do_get(request->ressource, client_sockfd);

            /* valgrind doesn't care about these 2... why? */
            free(request->ressource);
            free(request->req_str);
            free(request);
            
            exit(1);
        }

        close(client_sockfd);

    }
}

/*  ############## MAIN ############### */
int main(int argc, char **argv){

    const int CMD = 1;
    const int DEFAULT_PORT = 8080;
    int port;

    /************  START  *************/
    if (argc == 1) { 
        /* no arguments: start server with default-config 
         * which means cwd as root-dir and port 8080 */
        start(NULL, DEFAULT_PORT); 
    
    } else if (argc == 2) {

        /* optional commands to see if server is running / to stop it */
        if (strcmp(argv[CMD], "status") == 0) { print_status(); }
        else if (strcmp(argv[CMD], "stop") == 0) { stop(); }
        else {
            start(argv[1], DEFAULT_PORT);
        }

    }  else if (argc == 3) {
        /* 2 additional args, change basedir and port */
        /* check that the port is a valid port number */
        port = atoi(argv[2]);
        if (port < 1 || port > 65535) {
            printf("Invalid Port number!\n"); exit(-1);
        }
        start(argv[1], port);

    
    } else {
        printf("Invalid Arguments!\n");
        return -1;
    }

    return 0;
}

