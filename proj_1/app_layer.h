#pragma once

#include <stdint.h>

#define CTRL_SIZE_I 0
#define CTRL_NAME_I 1

#define TLV_PACKETS_COUNT 2
#define TLV_SIZE_T 0
#define TLV_NAME_T 1

#define DATA_BYTES 500

typedef enum{
    CTRL_DATA = 1,
    CTRL_START = 2,
    CTRL_END = 3
} app_ctrl_field;

typedef struct{
    uint8_t type;
    uint8_t length;
    uint8_t * value;
} tlv_packet;

typedef struct{
    char c_field; /* Control field. 2 - start, 3 - end */
    uint8_t tlv_count; /* Number of tlv packets included */
    tlv_packet * tlv_packets;
} app_ctrl_packet;

typedef struct{
    char c_field;
    uint8_t seq_n;
    uint8_t byte_n_low;
    uint8_t byte_n_high;
    uint8_t * data;
} app_data_packet;


int send_file(char * filename, int fd);