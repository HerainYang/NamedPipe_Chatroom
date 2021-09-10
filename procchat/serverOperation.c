//
// Created by herain on 5/7/21.
//

#include "serverOperation.h"

/**
 * This function will make a new connection if connection doesn't exist
 * @param message
 * @param domain_param
 * @param identifier_param
 * @return
 */
int makeConnection(char *message, char *domain_param, char *identifier_param) {
    char readFIFO_path[PATHSIZE];
    char writeFIFO_path[PATHSIZE];
    readFIFO_path[0] = '\0';
    writeFIFO_path[0] = '\0';
    char *identifier = (char *) (message + TYPESIZE);
    identifier[strlen(identifier)] = '\0';

    char *domain_origin = (char *) (message + TYPESIZE + IDENTIFIERSIZE);
    char domain[DOMAINSIZE];

    strncpy(domain, domain_origin, strlen(domain_origin));
    if(domain[strlen(domain_origin) - 1] != '/')
    {
        domain[strlen(domain_origin)] = '/';
        domain[strlen(domain_origin) + 1] = '\0';
    }else {
        domain[strlen(domain)] = '\0';
    }

    strcat(readFIFO_path, domain);
    strcat(readFIFO_path, identifier);
    strncat(readFIFO_path, "_RD", 4);
    readFIFO_path[strlen(readFIFO_path)] = '\0';

    strcat(writeFIFO_path, domain);
    strcat(writeFIFO_path, identifier);
    strncat(writeFIFO_path, "_WR", 4);
    writeFIFO_path[strlen(writeFIFO_path)] = '\0';

    domain_param[0] = '\0';
    identifier_param[0] = '\0';
    strncpy(identifier_param, identifier, strlen(identifier));
    identifier_param[strlen(identifier)] = '\0';
    strncpy(domain_param, domain, strlen(domain));
    domain_param[strlen(domain)] = '\0';

    if(access(domain, F_OK) == -1){
        //dir doesn't exist
        //make Dir
        //printf("[connection] dir doesn't exist, create dir and make node for it\n");
        char command[522];
        command[0] = '\0';
        strcat(command, "mkdir -p ");
        strcat(command, domain);
        FILE* fp = popen(command, "w");
        pclose(fp);
    } else {
        //domain is exist
        //test if domain is dir or not
        struct stat state;
        stat(domain, &state);
        if(!(S_ISDIR(state.st_mode))){
            return DOMAINISNOTADIR;
        }
    }

    if(access(readFIFO_path, F_OK) == -1){ // If file is not exist
        int res = mkfifo(readFIFO_path, READWRITETOALL);
        if(res == -1){
            printf("%d\n", errno);
            return FIFOFILECANNOTACCESS;
        }
    }

    if(access(writeFIFO_path, F_OK) == -1){ // If file is not exist
        int res = mkfifo(writeFIFO_path, READWRITETOALL);
        if(res == -1){
            unlink(readFIFO_path);
            return FIFOFILECANNOTACCESS;
        }
    }

    //client will read though _RD, write though _WR
    //so client handler and global process should write though _RD, read though _RD

    return SUCCESS;
}

int terminal_flag; //this flag will be set to 1 after receiving a PONG, if not, will be set to 0.
int parent_pid_global;
int ping_pid_global;
char domain_global[DOMAINSIZE];
char identifier_global[IDENTIFIERSIZE];

void receivedAPong(){
    terminal_flag = 1;
}

void missing_pong(){
    kill(ping_pid_global, SIGKILL);
    clientShutDown(parent_pid_global, domain_global, identifier_global);
    //kill(parent_pid_global, SIGUSR1);
}

_Noreturn void runClientHandler(char *domain_name, char *identifier_name, int parent_pid) {
    fflush(stdout);
    char WR_filepath[PATHSIZE];
    char RD_filepath[PATHSIZE];
    WR_filepath[0] = '\0';
    RD_filepath[0] = '\0';
    domain_global[0] = '\0';
    identifier_global[0] = '\0';

    parent_pid_global = parent_pid;
    strncpy(domain_global, domain_name, strlen(domain_name));
    strncpy(identifier_global, identifier_name, strlen(identifier_name));

    strncat(WR_filepath, domain_name, strlen(domain_name));
    strncat(RD_filepath, domain_name, strlen(domain_name));
    strncat(WR_filepath, identifier_name, strlen(identifier_name));
    strncat(RD_filepath, identifier_name, strlen(identifier_name));
    strcat(WR_filepath, "_WR");
    strcat(RD_filepath, "_RD");
    WR_filepath[strlen(WR_filepath)] = '\0';
    RD_filepath[strlen(RD_filepath)] = '\0';

    int pid_current = getpid();


    int pid_ping = fork();

    ping_pid_global = pid_ping;
    fflush(stdout);

    if(pid_ping == 0){
        fflush(stdout);
        struct sigaction act;
        act.sa_handler = receivedAPong;
        act.sa_flags = 0;
        sigemptyset(&act.sa_mask);
        sigaction(SIGUSR1, &act, NULL);
        char msg_ping[MESSAGESIZE];
        memset(msg_ping, 0, MESSAGESIZE);
        short *type = (short *) msg_ping;
        (*type) = PING;
        while (1) {
            terminal_flag = 0;
            sleep(15);
            if (terminal_flag == 0) {
                fflush(stdout);
                kill(pid_current, SIGUSR1);
            } else {
                fflush(stdout);
                int write_fd = open(RD_filepath, O_WRONLY | O_NONBLOCK);
                write(write_fd, msg_ping, MESSAGESIZE);
            }
        }
    }



    struct sigaction act;
    act.sa_handler = missing_pong;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGUSR1, &act, NULL);

    int read_fd;
    read_fd = open(WR_filepath, O_RDONLY); //read path is end with WR, ch need to read WD
    fd_set read_fdset;
    terminal_flag = 1;

    fflush(stdout);

    unsigned char buffer[MESSAGESIZE];
    while (1){
        //printf("enter loop\n");
        fflush(stdout);
        FD_ZERO(&read_fdset);

        FD_SET(read_fd, &read_fdset);

        if(select(read_fd + 1, &read_fdset, NULL, NULL, NULL) <= 0){
            //fprintf(stderr, "[client handler] select error\n");
        } else {
            if(FD_ISSET(read_fd, &read_fdset)){
                if(read(read_fd, buffer, MESSAGESIZE) > 0){
                    //fprintf(stderr, "%s read_fd change\n", WR_filepath);
                    short *type = (short *) buffer;
                    if((*type) == SAY){
                        clientSaySomething(buffer, domain_name, identifier_name, (*type));
                    } else if ((*type) == SAYCONT){
                        clientSaySomething(buffer, domain_name, identifier_name, (*type));
                    } else if((*type) == PONG){
                        //All good here, do nothing
                        kill(pid_ping, SIGUSR1);
                    } else if((*type) == DISCONNECT){
                        kill(pid_ping, SIGKILL);
                        clientShutDown(parent_pid_global, domain_name, identifier_name);
                        _exit(0);
                    } else {
                        printf("impossible to received this type, type is (%d)\n", (*type));
                    }
                    fflush(stdout);
                }
            }
        }
    }
}

void clientSaySomething(unsigned char *buffer, char *domainName, char *identifierName, short type) {
    char command[COMMANDSIZE+DOMAINSIZE];
    char file_name[DOMAINSIZE+IDENTIFIERSIZE];
    char msg_receive[MESSAGESIZE];

    short *type_rec = (short *) msg_receive;
    memset(msg_receive, 0, MESSAGESIZE);

    if(type == SAY)
        (*type_rec) = RECEIVE;
    else
        (*type_rec) = RECVCONT;

    memcpy(msg_receive + TYPESIZE, identifierName, strlen(identifierName) + sizeof(char));
    memcpy(msg_receive + TYPESIZE + IDENTIFIERSIZE, buffer + TYPESIZE, SAYMESSAGESIZE);

    if(type == SAYCONT)
        msg_receive[MESSAGESIZE-1] = buffer[MESSAGESIZE-1];

    command[0] = '\0';
    strcpy(command, "ls ");
    strcat(command, domainName);
    strcat(command, "*_RD");
    strcat(command, " -p");
    command[strlen(command)] = '\0';
    //fprintf(stderr, "command is: %s\n", command);
    fflush(stdout);


    FILE *file = popen(command, "r");
    while (fgets(file_name, DOMAINSIZE+IDENTIFIERSIZE, file) != NULL){
        if(strlen(file_name) > 3 && file_name[strlen(file_name) - 1] == '\n')
            file_name[strlen(file_name) - 1] = '\0';
        //fprintf(stderr, "%s\n", file_name);
        if(strncmp(file_name + strlen(domainName), identifierName, strlen(identifierName)) == 0){
            //fprintf(stderr, "same file is %s\n", file_name);
        } else {
            //fprintf(stderr, "different file is %s\n", file_name);
            int write_fd = open(file_name, O_WRONLY);
            if(write_fd > 0){
                //printf("writer open!\n");
                write(write_fd, msg_receive, MESSAGESIZE);
            }
            close(write_fd);
        }
        fflush(stdout);
    }
    fclose(file);
}

void clientShutDown(int parentPid, char *domainName, char *identifierName) {
    //printf("client shut down incorrectly\n");
    char command[DOMAINSIZE+IDENTIFIERSIZE];
    command[0] = '\0';
    strcpy(command, "rm ");
    strcat(command, domainName);
    strcat(command, identifierName);
    strcat(command, "_WR");
    //printf("rm command: %s\n", command);
    FILE *file = popen(command, "w");
    fclose(file);

    command[0] = '\0';
    strcpy(command, "rm ");
    strcat(command, domainName);
    strcat(command, identifierName);
    strcat(command, "_RD");
    //printf("rm command: %s\n", command);
    file = popen(command, "w");
    fclose(file);

    kill(parentPid, SIGUSR1);
    fflush(stdout);
}