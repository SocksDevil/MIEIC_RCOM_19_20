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

int open_control_connection(url_info_t url_info, pasv_info_t * pasv_info);
int open_data_connection(pasv_info_t pasv_info);
int close_connections();

int control_fd, data_fd;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: download address\n");
    exit(1);
  }

  /* Parse provided url */
  url_info_t url_info;
  parse_arguments(argv[1], &url_info);
  printf("Host: %s, url path: %s, user: %s, password: %s\n", url_info.host, url_info.url_path, url_info.user, url_info.password);

  /* Open control connection */
  if ((control_fd = ftp_open_connection(get_ip(url_info.host), SERVER_PORT)) == -1) {
    printf("Error opening control connection\n");
    return -1;
  }

  /* Read welcome msg */
  recvuntil(control_fd, TCP_READY);

  /* Client login on the server */
  ftp_login(control_fd, url_info);

  /* Activate passive mode */
  pasv_info_t pasv_info;
  ftp_passive_mode(control_fd, &pasv_info);

  /* Open data connection */
  if ((data_fd = ftp_open_connection(pasv_info.ip, pasv_info.port)) == -1) {
    printf("Error opening data connection\n");
    return -1;
  }

  /* Request recv */
  if (ftp_request_file_read(control_fd, url_info.url_path) == -1) {
    close_connections();
    return -1;
  }

  ftp_read_file(data_fd, url_info.url_path);
  
  close_connections();
  return 0;
}


int close_connections() {
  ftp_disconnect(control_fd);
  
  close(control_fd);
  close(data_fd);

  return 0;
}