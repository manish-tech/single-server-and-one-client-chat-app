#include <stdio.h>
#include<stdlib.h>
#include <strings.h>
#include<string.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#include <sys/stat.h>
#include<signal.h> 
int gsockfd,gcpid;
void error(char *msg)
{
    perror(msg);
    exit(0);
}

void handleParentSigInt(int sig){
    close(gsockfd);
    printf("closing the application sucessfully\n");
    exit(0);
}

void handleChildSigInt(int sig){
    close(gsockfd);
    exit(0);
}

void clientSetup(int *sockfd,char *host,char *port){
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = atoi(port);
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(*sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("connection established\n");
    gsockfd = *sockfd;
}


void writeToSocket(char *input , int sockfd){
    ssize_t  n = -1;
    bzero(input,256);
    fgets(input, 256, stdin);
    n = write(sockfd,(char *)input,strlen(input));
    if(n == -1){
        error("erro while writting to socket");
    }
    bzero(input,256);
}

void readFromSocket(char *output,int sockfd){
    ssize_t n = -1;
    bzero(output,256);
    while(n = read(sockfd,(char*)output,256) > 0){
        printf("server-> %s",output);
        bzero(output,256);
    }
    if(n == -1){
        error("error while reading from socket");
    }
}
int main(int argc, char *argv[])
{
    int sockfd, portno;
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    clientSetup(&sockfd,argv[1],argv[2]);
    signal(SIGINT,handleParentSigInt);
    
    pid_t pid = fork();
        if(pid < 0){
            printf("error couldn't fork \n");
            exit(1);
        }else if(pid == 0){
            signal(SIGINT,handleChildSigInt);
            char output[256];
            while(1){
                readFromSocket(output,sockfd);
            }
        }else{
            char input[256];
            while(1){
                writeToSocket(input,sockfd);
            }
        }
    return 0;
}