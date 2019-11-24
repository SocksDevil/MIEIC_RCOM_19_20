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

#include "hostname.h"
#include "connection.h"

#define SERVER_PORT 21
#define MAX_SIZE 2500

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: download address\n");
    exit(1);
  }

  /* Parse provided url */
  url_info_t url_info;
  parse_arguments(argv[1], &url_info);
  printf("Host: %s, url path: %s, user: %s, password: %s\n", url_info.host, url_info.url_path, url_info.user, url_info.password);

  /* server address handling*/
  struct sockaddr_in server_addr;
  bzero((char *) &server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(get_ip(url_info.host)); /*32 bit Internet address network byte ordered*/
  server_addr.sin_port = htons(SERVER_PORT);

  int socketfd; 
  /* open an TCP socket*/
  if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket()");
    exit(1);
  }

  /* connect to the server*/
  if (connect(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("connect()");
    exit(1);
  }

  /* Read welcome msg */
  recvuntil(socketfd, TCP_READY);

  /* Client login on the server */
  ftp_login(socketfd, url_info);

  /* Activate passive mode */
  pasv_info_t pasv_info;
  ftp_passive_mode(socketfd, &pasv_info);  
  
  /* Client disconnect */
  ftp_disconnect(socketfd);

  
  close(socketfd);
  return 0;
}