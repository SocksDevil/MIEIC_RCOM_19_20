#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include "app_layer.h"
#include "utils.h"

static int seq_number = 0;

app_ctrl_packet prepare_ctrl_packet(app_ctrl_field packet_type, char * filename, unsigned long filesize) {

    int filesize_size = byte_size(filesize);
    int filename_size = strlen(filename)+1;

    // header
    app_ctrl_packet packet;
    packet.c_field = packet_type;
    packet.tlv_count = TLV_PACKETS_COUNT;
    packet.tlv_packets = (tlv_packet*) malloc(sizeof(tlv_packet) * TLV_PACKETS_COUNT);

    // filesize
    packet.tlv_packets[CTRL_SIZE_I].type = TLV_SIZE_T;
    packet.tlv_packets[CTRL_SIZE_I].length = filesize_size;
    packet.tlv_packets[CTRL_SIZE_I].value = (unsigned char *) malloc(sizeof(unsigned char) * filesize_size);
    for (int i = 0; i < filesize_size; i++)
        packet.tlv_packets[CTRL_SIZE_I].value[i] = (unsigned char) (filesize >> 8 * (filesize_size - i - 1));

    // filename
    packet.tlv_packets[CTRL_NAME_I].type = TLV_NAME_T;
    packet.tlv_packets[CTRL_NAME_I].length = filename_size;
    packet.tlv_packets[CTRL_NAME_I].value = (unsigned char *) malloc(sizeof(unsigned char) * filename_size);
    for (int i = 0; i < filename_size; i++)
        packet.tlv_packets[CTRL_NAME_I].value[i] = filename[i];

    return packet;
}

app_data_packet prepare_data_packet(app_ctrl_field packet_type, unsigned char * buff, short buffsize) {

    app_data_packet packet;
    packet.c_field = packet_type;
    packet.seq_n = seq_number++ % UCHAR_MAX;
    packet.byte_n_l = (unsigned char) buffsize;
    packet.byte_n_h= (unsigned char) (buffsize >> 8);

    packet.data = (unsigned char *) malloc(sizeof(unsigned char) * buffsize);
    for (int i = 0; i < buffsize; i++)
        packet.data[i] = buff[i];

    return packet;
}

int send_file(char * filename, int fd) {

    int file_fd;
    if ((file_fd = open(filename, O_RDONLY)) == -1) {
        perror("open");
        return -1;
    }

    //unsigned long filesize = file_size(filename);

    //app_ctrl_packet start_packet = prepare_ctrl_packet(CTRL_START, filename, filesize);
    // send packet

    int n;
    unsigned char buff[DATA_BYTES];
    while((n = read(file_fd, buff, DATA_BYTES))) {
        //app_data_packet data_packet = prepare_data_packet(CTRL_DATA, buff, n);
        // send packet
    }

    //app_ctrl_packet end_packet = prepare_ctrl_packet(CTRL_END, filename, filesize);
    // send packet

    printf("%d", fd);

    return 0;
}