#include <stdio.h>
#include<stdlib.h>
#include <strings.h>
#include<string.h>
#include<unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include<signal.h> 

int gsockfd,gnewsockfd;
void error(char *msg)
{
    perror(msg);
    exit(1);
}
/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
void serverSetup(int *sockfd,int *newsockfd,int *portno,int *clilen,char *argv){
    struct sockaddr_in serv_addr, cli_addr;
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
    *portno = atoi(argv);
     serv_addr.sin_family = AF_INET;

     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(*portno);
     if (bind(*sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(*sockfd,5);
     *clilen = sizeof(cli_addr);
     *newsockfd = accept(*sockfd, (struct sockaddr *) &cli_addr, clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
    printf("listing to PORT: %d \n",*portno);
    gsockfd = *sockfd;
    gnewsockfd = *newsockfd;
}

void writeToSocket(char *input , int newsockfd){
    ssize_t  n = -1;
    bzero(input,256);
    fgets(input, 256, stdin);
    n = write(newsockfd,(char *)input,strlen(input));
    if(n == -1){
        error("erro while writting to socket");
    }
    bzero(input,256);
}

void readFromSocket(char *output,int newsockfd){
    ssize_t n = -1;
    bzero(output,256);
    while(n = read(newsockfd,(char*)output,256) > 0){
        printf("client-> %s",output);
        bzero(output,256);
    }
    if(n == -1){
        error("error while reading from socket");
    }
}

void handleParentSigInt(int sig){
    close(gsockfd);
    close(gnewsockfd);
    printf("closing the application sucessfully\n");
    exit(0);
}

void handleChildSigInt(int sig){
    close(gsockfd);
    close(gnewsockfd);
    exit(0);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
    serverSetup(&sockfd,&newsockfd,&portno,&clilen,argv[1]);
     signal(SIGINT,handleParentSigInt);
    pid_t pid = fork();
        if(pid < 0){
            printf("error couldn't fork \n");
            exit(1);
        }else if(pid == 0){
            char output[257];
            signal(SIGINT,handleChildSigInt);
            while(1){
                readFromSocket(output,newsockfd);
            }

        }else{
            char input[256];
            while(1){
                writeToSocket(input,newsockfd);
            }
        }
     return 0; 
}