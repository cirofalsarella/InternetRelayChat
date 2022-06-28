#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <string>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr
using namespace std;

pthread_t writer, reader;

int connfd;
bool hasQuit=false;
void *sendMessage(void *sock){
    char buffer[4096] = { 0 };
    string message;
    int network_socket = connfd, j;
    while(!hasQuit){
        getline(cin, message);
        for(int i=0; i<=message.length()/4095; ++i){
            memset(buffer, 0, sizeof(buffer));
            for(j=0; j<message.length()-i*4095 && j<4096; ++j){
                buffer[j]=message[j+i*4095];
                
            }
            if(j!=4095) buffer[4095]='\0';
            else{
                buffer[4095]=message[4095+i*4095];
            }
            send(network_socket, buffer, 4096, 0);
        }
        if ((strncmp(buffer, "exit", 4)) == 0) {
            printf("Server Exit...\n");
            hasQuit=true;
            pthread_cancel(reader);
            pthread_exit(NULL);
            return NULL;
        }
    }

    pthread_exit(NULL);
    return NULL;
}

void *readMessage(void *sock){
    int network_socket = connfd;
    char buffer[4096] = { 0 };
    while(!hasQuit){
        memset(buffer, 0, sizeof(buffer));
        read(network_socket, buffer, 4096);
        if(buffer[0]!=0){
            printf(" Client message: ");
            printf("%s", buffer);
            while (buffer[4095]!='\0'){
                memset(buffer, 0, sizeof(buffer));
                read(network_socket, buffer, 4096);
                printf("%s", buffer);
            }
            printf("\n");
        }
        if ((strncmp(buffer, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            hasQuit=true;
            pthread_cancel(writer);
            pthread_exit(NULL);
            return NULL;
        }
    }
    
    pthread_exit(NULL);
    return NULL;
}

int main(){
    int sockfd, len;
    struct sockaddr_in servaddr, cli;
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
   
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    bind(sockfd, (SA*)&servaddr, sizeof(servaddr));
   
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);
   
    connfd = accept(sockfd, (SA*)&cli, (socklen_t*)&len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");
   
    pthread_create(&writer, NULL,sendMessage, &connfd);
    pthread_create(&reader, NULL, readMessage, &connfd);

    pthread_join(reader, NULL);
    pthread_join(writer, NULL);

    close(sockfd);
    return 0;
}
