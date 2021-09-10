#include <stdio.h>
#include <zconf.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <signal.h>

#include "server.h"

#include "serverOperation.h"


#define GLOBALPIPE ("gevent")

void clienthandler_shutdown(int sig, siginfo_t *info, void *ctx){
    kill(info->si_pid, SIGKILL);
    fflush(stdout);
}

int main() {
    int pid = getpid();

    if(access(GLOBALPIPE, F_OK) == -1){ // If file is not exist
        int res = mkfifo(GLOBALPIPE, READWRITETOALL);
        if(res == -1){
            printf("Can not open global event file");
            return 1;
        }
    }

    int gevent_fd = open(GLOBALPIPE, O_RDONLY);
    if(gevent_fd < 0){
        printf("%d\n", errno);
        perror("open error: ");
        printf("Global event file descriptor open failed\n");
        return 1;
    }

    struct sigaction act;
    act.sa_sigaction = clienthandler_shutdown;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);

    signal(SIGCHLD,SIG_IGN);

    fd_set readfd_set;
    char buf[MESSAGESIZE];
    while (1){
        FD_ZERO(&readfd_set);
        FD_SET(gevent_fd, &readfd_set);

        if(select(gevent_fd + 1, &readfd_set, NULL, NULL, NULL) <= 0){
            //fprintf(stderr, "select error, errno: %d\n", errno);
        } else {
            if(FD_ISSET(gevent_fd, &readfd_set)){
                if(read(gevent_fd, buf, MESSAGESIZE) > 0){
                    short *type = (short *) buf;
                    switch (*type) {
                        case CONNECT:{
                            int res;
                            char identifier[IDENTIFIERSIZE];
                            char domain[DOMAINSIZE];
                            //fprintf(stderr, "connection required from %s\n", (buf+2));
                            res = makeConnection(buf, domain, identifier);
                            if(res == SUCCESS){
                                int forkRes = fork();
                                fflush(stdout);
                                if(forkRes == 0){
                                    close(gevent_fd);
                                    runClientHandler(domain, identifier, pid);
                                    return 0;
                                }
                            }
                            break;
                        }
                        default:{
                            printf("error input\n");
                        }
                    }
                }
            }
        }
    }
}

