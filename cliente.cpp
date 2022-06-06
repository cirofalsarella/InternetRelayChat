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

void sendMessage(void *sock){
    char buffer[4096] = { 0 };
    string message;
    cin >> message;
    while(message.compare("quit")){
        if(message.length()>4096){

        }
        else{
            send(sock, message, message.length(), 0);
        }
        
        cin >> message;
    }
    printf("Hello message sent\n");
}
void readMessage(void *sock){
    string message;
    int valread;
    char buffer[4096] = { 0 };

    valread = read(sock, buffer, 4096);
    printf("%s\n", buffer);
}
 
int main(int argc, char const* argv[])
{
    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    if ((client_fd
         = connect(sock, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    
    
    //USE THREADS, ONE TO READ FOR MESSAGES AND ONE TO WAIT FOR INPUTS AND SEND MESSAGES
    pthread_t writer, reader;
 
    // Create connection
    // depending on the input
 
    // Create thread
    pthread_create(&writer, NULL,sendMessage, (void *) sock);
    pthread_create(&reader, NULL, readMessage, (void *) sock);
 
    // Suspend execution of
    // calling thread
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);
    // closing the connected socket
    close(client_fd);
    return 0;
}