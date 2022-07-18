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
  string str;
  char at = getchar();
  // FLAG -> mudar getchar pra outra função que corrija o bug
  while (at != EOF && at != '\r' && at != '\n') {
    str += at;
    at = getchar();
    // FLAG -> mudar getchar pra outra função que corrija o bug
    cout << str << endl;
  }

  if (at != EOF && str.length() == 0)
    str = readLine();

  return str;
}

void sigintHandler(int sig_num) {
  signal(SIGINT, sigintHandler);
}

void *sendMessage(void *sock) {
  char buffer[4096] = {0};
  string message;
  int network_socket = sockfd, j;
  while (!hasQuit) {
    sigintHandler(1);
    message = readLine();
    cout << message << endl;

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
    for (int i = 0; i <= message.length() / 4095; ++i) {
      memset(buffer, 0, sizeof(buffer));
      for (j = 0; j < message.length() - i * 4095 && j < 4096; ++j) {
        buffer[j] = message[j + i * 4095];
      }
      if (j != 4095)
        buffer[4095] = '\0';
      else {
        buffer[4095] = message[4095 + i * 4095];
      }

      send(network_socket, buffer, 4096, 0);
    }
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
  int network_socket = sockfd;
  char buffer[4096] = {0};
  while (!hasQuit) {
    memset(buffer, 0, sizeof(buffer));
    read(network_socket, buffer, 4096);
    if (buffer[0] != 0) {
      sendAck();
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

int main() {
  struct sockaddr_in servaddr, cli;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // trocar pelo ip do servidor se ele está em outra rede
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

  pthread_create(&writer, NULL, sendMessage, &sockfd);
  pthread_create(&reader, NULL, readMessage, &sockfd);

  pthread_join(reader, NULL);
  pthread_join(writer, NULL);

  close(sockfd);
  return 0;
}