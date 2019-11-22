#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#define MAX_SIZE 2500
#include "hostname.h"
#include "statemachine.h"
#define SERVER_PORT 21

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: download address\n");
    exit(1);
  }
  char host[MAX_URL_SIZE], url_path[MAX_URL_SIZE], user[MAX_URL_SIZE], password[MAX_URL_SIZE];
  parse_arguments(argv[1], host, url_path, user, password);
  printf("Host: %s, url path: %s, user %s, password: %s\n", host, url_path, user, password);
  /*server address handling*/
  struct sockaddr_in server_addr;
  bzero((char *) &server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(get_ip(host)); /*32 bit Internet address network byte ordered*/
  server_addr.sin_port = htons(SERVER_PORT);

  // printf("Server ip %s\n", get_ip(argv[1]));



  int socketfd; 
  /*open an TCP socket*/
  if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket()");
    exit(1);
  }
  /*connect to the server*/
  if (connect(socketfd,
              (struct sockaddr *) &server_addr,
              sizeof(server_addr)) < 0) {
    perror("connect()");
    exit(1);
  }
  char buf[MAX_SIZE];
  sprintf(buf, "user %s\r\n", user);

  recvuntil(socketfd, "Name");

  printf("Meias\n");
  int bytes = write(socketfd, buf, strlen(buf));

  printf("Written %s\n", buf);
  printf("Written %d bytes\n", bytes);

  recvuntil(socketfd, "Name");

  
  close(socketfd);
  return 0;
}