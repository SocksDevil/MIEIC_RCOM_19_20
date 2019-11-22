#pragma once

#define MAX_URL_SIZE 2000

typedef enum{
    NO_USERNAME,
    USERNAME_ONLY,
    USERNAME_AND_PW,
    ERROR
} url_type;

char * get_ip(char *hostname);

char *build_url(char *hostname, char *user, char *password, char *url);

url_type parse_arguments(char *address, char *host, char *url_path, char *user, char *password);
