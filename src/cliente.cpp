// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>
// For threading, link with lpthread
#include <pthread.h>
#include <semaphore.h>

#define PORT 8080

using namespace std;

bool hasQuit=false;
void *sendMessage(void *sock){
    char buffer[4096] = { 0 };
    string message;
    int network_socket = socket(AF_INET, SOCK_STREAM, 0), j;
    do{
        cout << "write message";
        cin >> message;
        for(int i=0; i<=message.length()/4095; ++i){
            for(j=0; j<message.length()-i*4095 && j<4096; ++j){
                buffer[j]=message[j+i*4095];
                
            }
            if(j!=4095) buffer[4095]='\0';
            send(network_socket, buffer, 4096, 0);
        }
    }while(message.compare("quit") && !hasQuit);
    hasQuit=true;

    pthread_exit(NULL);
    return NULL;
}

void *readMessage(void *sock){
    string message;
    int network_socket = socket(AF_INET, SOCK_STREAM, 0);
    char buffer[4096] = { 0 };
    do{
        read(network_socket, buffer, 4096);
        if(buffer[0]!='\0'){
            printf("%s\n", buffer);
            while (buffer[4095]!='\0'){
                read(network_socket, buffer, 4096);
                printf("%s", buffer);
            }
        }
        
    }while(!hasQuit);

    hasQuit=true;
    
    pthread_exit(NULL);
    return NULL;
}
 
int main(int argc, char const* argv[])
{
    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    //inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
 
    client_fd = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    
    //USE THREADS, ONE TO READ FOR MESSAGES AND ONE TO WAIT FOR INPUTS AND SEND MESSAGES
    pthread_t writer, reader;
 
    // Create connection
    // depending on the input
 
    // Create thread
    pthread_create(&writer, NULL,sendMessage, &sock);
    pthread_create(&reader, NULL, readMessage, &sock);
 
    // Suspend execution of
    // calling thread
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);
    // closing the connected socket
    close(client_fd);
    return 0;
}