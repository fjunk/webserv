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


#define MAX_CHILDS 17
#define FNAME_PID "server.pid"
#define LOGFILE "server.log"


/* PROTOTYPES */
void serve(int port);

/* GLOBALS */
int child_pids[MAX_CHILDS];


/* log string of request */
void log_request(char *str) {
    /* char* log_file = LOGFILE;
    int fd = open(log_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    write(fd, str, strlen(str)); */
    printf("%s\n", str);
}

/* removes the pid-file from current dir and kills all child processes */
void int_handler(int sig) {
    int i;

    /* kill children first */
    for(i=0; i<MAX_CHILDS; i++) {
        if(child_pids[i]){
            printf("killing child %d\n", child_pids[i]);
            if (kill(child_pids[i], sig)) {
                perror("Kill failed\n");
            }
        }
    }
    /* remove PID-file */
    printf("removing %s!\n", FNAME_PID);
    unlink(FNAME_PID);
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


/* reads the PID from the pid-file in the current dir and returns it */
int read_pid() {
    int fd = open(FNAME_PID, O_RDONLY);
    char s_pid [10];
    int len = read(fd, s_pid, 10);

    s_pid[len] = '\n';
    return atoi(s_pid);
}


/* Returns 1 if a process with pid exists, else 0 */
int process_exists(int pid) {
    return kill(pid, 0) == 0 ? 1 : 0;
}

void start() {
    /* save server pid to filesystem */
    write_pid();
    serve(8000);
}

void status() {
    int pid = read_pid();
    if(process_exists(pid)){
        printf("Server running with pid: %d", pid);
    } else {
        printf("Server not running!");
    }
}

void stop() {
    int pid = read_pid();
    if(process_exists(pid))
        kill(pid, SIGINT);
    else
        printf("pid %d does not exist", pid);
}


void serve(int port) {

    int i, child_pid;
    int server_sockfd, client_sockfd;
    socklen_t client_len;
    struct sockaddr_in server_addr, client_addr;

    /* install SIGCHLD handler to respond to finishing childs */
    signal(SIGCHLD, child_handler);

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
                    printf("could save child_pid!\n");
                    break;
                }
            }
        }

        if (child_pid == 0) {
            close(server_sockfd);


            /* TODO STUFF */
            /* decode / split / do_get */
            printf("meow meow meoooow\n");
            sleep(3);

            exit(1);
        }

        close(client_sockfd);

    }
}

/*  ############## MAIN ############### */
int main(int argc, char **argv){

    enum{ CMD=1 };

    /* need at least one argument */
    if ( argc < 2 ) return -1;

    /************  START  *************/
    if(strcmp(argv[CMD], "start") == 0) {
        start();
    
    /************  STATUS  ************/
    } else if (strcmp(argv[CMD], "status") == 0) {
        status();

    /************  STOP  ************/
    } else if (strcmp(argv[CMD], "stop") == 0) {
        stop();

    } else {
        printf("Invalid Arguments!\n");
        printf("Try: ./server start | status | stop\n");
        return -1;
    }

    return 0;
}

