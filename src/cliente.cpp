#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>

#define MAX 80
#define PORT 3002
#define IP "127.0.0.1"
// Use server IP if in another network

#define SA struct sockaddr

using namespace std;
pthread_t writer, reader;

int sockfd;
bool hasQuit = false;

void sendAck() {
  char buf[4096];
  memset(buf, 0, sizeof(buf));
  buf[0] = 'a';
  buf[1] = 'c';
  buf[2] = 'k';
  send(sockfd, buf, 4096, 0);
}

string readLine() {
  // Faltou aplicar nessa função alguma leitura que lesse diretamente o que é digitado
  // Depois que apertamos o 'ctrl+c' o buffer da linha é limpo, perdendo as informações anteriores

  string line = "";
  char at = getchar();

  while (at != EOF && at != '\r' && at != '\n') {
    line += at;
    at = getchar();
  }

  if (at != EOF && line.length() == 0)
    line = readLine();

  return line;
}

void *sendMessage(void *sock) {
  // Set buffer and other variables
  char buffer[4096] = {0};
  string message;
  int network_socket = sockfd, j;

  while (!hasQuit) {
    // Read the message of the client
    message = readLine();

    // End of messages
    if (cin.eof()) {
      memset(buffer, 0, sizeof(buffer));
      buffer[0] = '/';
      buffer[1] = 'q';
      buffer[2] = 'u';
      buffer[3] = 'i';
      buffer[4] = 't';
      buffer[5] = '\0';
      send(network_socket, buffer, 4096, 0);
      printf("Client Exit...\n");
      hasQuit = true;
      pthread_cancel(reader);
      pthread_exit(NULL);
      return NULL;
    }

    // Send message in portions of 4095 bytes till we send the full message
    for (int i = 0; i <= message.length() / 4095; ++i) {
      memset(buffer, 0, sizeof(buffer));
      for (j = 0; j < message.length() - i * 4095 && j < 4096; ++j) {
        buffer[j] = message[j + i * 4095];
      }

      if (j != 4095)
        buffer[4095] = '\0';
      else
        buffer[4095] = message[4095 + i * 4095];

      send(network_socket, buffer, 4096, 0);
    }

    // Forced Quit
    if ((strncmp(buffer, "/quit", 4)) == 0) {
      printf("Client Exit...\n");
      hasQuit = true;
      pthread_cancel(reader);
      pthread_exit(NULL);
      return NULL;
    }
  }

  pthread_exit(NULL);
  return NULL;
}

void *readMessage(void *sock) {
  // Set variables
  int network_socket = sockfd;
  char buffer[4096] = {0};

  while (!hasQuit) {
    // Get buffer info
    memset(buffer, 0, sizeof(buffer));
    read(network_socket, buffer, 4096);

    // If there is a message
    if (buffer[0] != 0) {
      sendAck();

      // Print the different parts of the message
      printf("%s", buffer);
      while (buffer[4095] != '\0') {
        memset(buffer, 0, sizeof(buffer));
        read(network_socket, buffer, 4096);
        sendAck();
        printf("%s", buffer);
      }
      printf("\n");
    }
  }

  pthread_exit(NULL);
  return NULL;
}

// Just ignores ctrl+c
void sigintHandler(int sig_num) {
  printf("\nSorry, you can't do it. Try again.\n");
  return;
}

int main() {
  // Set to ignore ctrl C calls
  signal(SIGINT, sigintHandler);
  struct sockaddr_in servaddr, cli;

  // Set socket and its address
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(IP);
  servaddr.sin_port = htons(PORT);

  string s;
  cin >> s;

  while (s.compare("/connect")) {
    cout << "write /connect to connect" << endl;
    cin >> s;
  }
  if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
    printf("connection with the server failed...\n");
    exit(0);
  } else
    printf("connected to the server...\n");

  // Begin connection
  pthread_create(&writer, NULL, sendMessage, &sockfd);
  pthread_create(&reader, NULL, readMessage, &sockfd);

  pthread_join(reader, NULL);
  pthread_join(writer, NULL);

  close(sockfd);
  return 0;
}