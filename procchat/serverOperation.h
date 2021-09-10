//
// Created by herain on 5/7/21.
//

#ifndef PROCCHAT_SERVEROPERATION_H
#define PROCCHAT_SERVEROPERATION_H

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <zconf.h>
#include <string.h>
#include <signal.h>

#define CONNECT 0
#define SAY 1
#define SAYCONT 2
#define RECEIVE 3
#define RECVCONT 4
#define PING 5
#define PONG 6
#define DISCONNECT 7

#define INVALIDDOMAIN -1
#define FIFOFILECANNOTACCESS -2
#define DOMAINISNOTADIR -3
#define FIFOALREADYEXIST -4
#define CANNOTOPENFD -5
#define CANNOTCREATEPIPE -6
#define SUCCESS 0

#define MESSAGESIZE 2048
#define TYPESIZE 2
#define IDENTIFIERSIZE 256
#define PATHSIZE 515
#define DOMAINSIZE 257
#define COMMANDSIZE 32
#define SAYMESSAGESIZE 1790

#define READWRITETOALL 0777

int makeConnection(char *message, char *domain_param, char *identifier_param);

_Noreturn void runClientHandler(char *domain_name, char *identifier_name, int parent_pid);
void clientSaySomething(unsigned char *buffer, char *domainName, char *identifierName, short type);
void clientShutDown(int parentPid, char *domainName, char *identifierName);

#endif //PROCCHAT_SERVEROPERATION_H

