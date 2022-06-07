#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
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

int sockfd;
bool hasQuit=false;
void *sendMessage(void *sock){
    char buffer[4096] = { 0 };
    string message;
    int network_socket = sockfd, j;
    while(!hasQuit){
        getline(cin, message);
        for(int i=0; i<=message.length()/4095; ++i){
            memset(buffer, 0, sizeof(buffer));
            for(j=0; j<message.length()-i*4095 && j<4096; ++j){
                buffer[j]=message[j+i*4095];
                
            }
            if(j!=4095) buffer[4095]='\0';
            send(network_socket, buffer, 4096, 0);
        }
        if ((strncmp(buffer, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            hasQuit=true;
            pthread_exit(NULL);
            return NULL;
        }
    }

    pthread_exit(NULL);
    return NULL;
}

void *readMessage(void *sock){
    int network_socket = sockfd;
    char buffer[4096] = { 0 };
    while(!hasQuit){
        memset(buffer, 0, sizeof(buffer));
        read(network_socket, buffer, 4096);
        if(buffer[0]!=0){
            printf("%s\n", buffer);
            while (buffer[4095]!='\0'){
                memset(buffer, 0, sizeof(buffer));
                read(network_socket, buffer, 4096);
                printf("%s", buffer);
            }
        }
        if ((strncmp(buffer, "exit", 4)) == 0) {
            printf("Server Exit...\n");
            hasQuit=true;
            pthread_exit(NULL);
            return NULL;
        }
    }
    
    pthread_exit(NULL);
    return NULL;
}
   
int main(){
    struct sockaddr_in servaddr, cli;
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
   
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
   
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
   
    pthread_t writer, reader;
    pthread_create(&writer, NULL,sendMessage, &sockfd);
    pthread_create(&reader, NULL, readMessage, &sockfd);

    pthread_join(writer, NULL);
    pthread_join(reader, NULL);
   
    close(sockfd);
    return 0;
}
