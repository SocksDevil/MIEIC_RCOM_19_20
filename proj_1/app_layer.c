#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include "app_layer.h"
#include "utils.h"
#include "ll.h"

app_ctrl_packet prepare_ctrl_packet(app_ctrl_field packet_type, char * filename, long filesize) {

    uint8_t filesize_size = byte_size(filesize);
    int filename_size = strlen(filename)+1;

    // header
    app_ctrl_packet packet;
    packet.c_field = packet_type;
    packet.tlv_count = TLV_PACKETS_COUNT;
    packet.tlv_packets = (tlv_packet*) malloc(sizeof(tlv_packet) * TLV_PACKETS_COUNT);

    // filesize
    packet.tlv_packets[CTRL_SIZE_I].type = TLV_SIZE_T;
    packet.tlv_packets[CTRL_SIZE_I].length = filesize_size;
    packet.tlv_packets[CTRL_SIZE_I].value = (uint8_t *) malloc(sizeof(uint8_t) * filesize_size);
    for (int i = 0; i < filesize_size; i++)
        packet.tlv_packets[CTRL_SIZE_I].value[i] = (uint8_t) (filesize >> 8 * (filesize_size - i - 1));

    // filename
    packet.tlv_packets[CTRL_NAME_I].type = TLV_NAME_T;
    packet.tlv_packets[CTRL_NAME_I].length = filename_size;
    packet.tlv_packets[CTRL_NAME_I].value = (uint8_t *) malloc(sizeof(uint8_t) * filename_size);
    for (int i = 0; i < filename_size; i++)
        packet.tlv_packets[CTRL_NAME_I].value[i] = filename[i];

    return packet;
}

app_data_packet prepare_data_packet(app_ctrl_field packet_type, uint8_t * buff, short buffsize, int seq_number) {

    app_data_packet packet;
    packet.c_field = packet_type;
    packet.seq_n = seq_number % UCHAR_MAX;
    packet.byte_n_low = (uint8_t) buffsize;
    packet.byte_n_high= (uint8_t) (buffsize >> 8);

    packet.data = (uint8_t *) malloc(sizeof(uint8_t) * buffsize);
    for (int i = 0; i < buffsize; i++)
        packet.data[i] = buff[i];

    return packet;
}

void free_ctrl_packet(app_ctrl_packet * packet) {
    for (int i = 0; i < packet->tlv_count; i++) {
        free(packet->tlv_packets[i].value);
    }
    free(packet->tlv_packets);
}

void free_data_packet(app_data_packet * packet) {
    free(packet->data);
}

int send_ctrl_packet(int fd, app_ctrl_packet * packet) {

    // alocate buffer
    int buffer_size = 1;
    for (int i = 0; i < packet->tlv_count; i++) {
        buffer_size += 2;
        buffer_size += packet->tlv_packets[i].length;
    }
    char * buffer = (char *) malloc(sizeof(char) * buffer_size);
    if (buffer == NULL) {
        perror("malloc");
        return -1;
    }

    // fill buffer
    int ins_i = 0;
    buffer[ins_i++] = packet->c_field;
    for (int i = 0; i < packet->tlv_count; i++) {
        tlv_packet * tlv_p = &(packet->tlv_packets[i]);

        buffer[ins_i++] = tlv_p->type;
        buffer[ins_i++] = tlv_p->length;
        for (int j = 0; j < tlv_p->length; j++)
            buffer[ins_i++] = tlv_p->value[j];
    }

    // send buffer
    if (llwrite(fd, buffer, buffer_size) == -1) {
        perror("llwrite");
        return -1;
    }

    free(buffer);

    return 0;
}

int send_data_packet(int fd, app_data_packet * packet) {

    int data_bytes_count = (packet->byte_n_high << 8) + packet->byte_n_low;

    // alocate buffer
    int buffer_size = 4 + data_bytes_count;
    char * buffer = (char *) malloc(sizeof(char) * buffer_size);
    if (buffer == NULL) {
        perror("malloc");
        return -1;
    }

    // fill buffer
    int ins_i = 0;
    buffer[ins_i++] = packet->c_field;
    buffer[ins_i++] = packet->seq_n;
    buffer[ins_i++] = packet->byte_n_high;
    buffer[ins_i++] = packet->byte_n_low;
    for (int i = 0; i < data_bytes_count; i++) {
        buffer[ins_i++] = packet->data[i];
    }

    // send buffer
    if (llwrite(fd, buffer, buffer_size) == -1) {
        perror("llwrite");
        return -1;
    }

    free(buffer);

    return 0;
}

int send_file(char * filename, int fd) {

    int file_fd;
    if ((file_fd = open(filename, O_RDONLY)) == -1) {
        perror("open");
        return -1;
    }

    long filesize = file_size(filename);

    // start ctrl packet
    app_ctrl_packet start_packet = prepare_ctrl_packet(CTRL_START, filename, filesize);
    if (send_ctrl_packet(fd, &start_packet) == -1) {
        printf("Error sending start packet\n");
        return -1;
    }

    // data packets
    int n, seq_number = 0;
    uint8_t buff[DATA_BYTES];
    while((n = read(file_fd, buff, DATA_BYTES))) {
        app_data_packet data_packet = prepare_data_packet(CTRL_DATA, buff, n, seq_number++);

        if (send_data_packet(fd, &data_packet) == -1) {
            printf("Error sending data packet nº%d\n", seq_number-1);
            return -1;
        }

        free_data_packet(&data_packet);
    }

    // end ctrl packet
    app_ctrl_packet end_packet = prepare_ctrl_packet(CTRL_END, filename, filesize);
    if (send_ctrl_packet(fd, &end_packet) == -1) {
        printf("Error sending end packet\n");
        return -1;
    }

    // cleanup
    free_ctrl_packet(&start_packet);
    free_ctrl_packet(&end_packet);
    if (close(file_fd) == -1) {
        perror("close file_fd");
        return -1;
    }

    return 0;
}

ctrl_info parse_ctrl_packet(char * buffer, unsigned short size) {

    unsigned short i = 1;
    ctrl_info info;
    info.filesize = 0;
    info.filename = NULL;

    if(buffer[0] != CTRL_START && buffer[0] != CTRL_END) {
        printf("Wrong control field value, expected 2, got %d\n", buffer[0]);
        return info;
    }

    // handle i segfaults if corrupted packet
    while(i < size) {
        uint8_t type = (uint8_t) buffer[i++];
        uint8_t length = (uint8_t) buffer[i++];
        
        switch(type) {
            case TLV_SIZE_T:
                for (int j = 0; j < length; j++)
                    info.filesize = (((unsigned long) info.filesize) << 8) + (uint8_t) buffer[i++];
                break;

            case TLV_NAME_T:
                info.filename = (char*) malloc(sizeof(char) * length);
                for (int j = 0; j < length; j++)
                    info.filename[j] = buffer[i++];

                break;

            default:
                printf("Unknown TLV type\n");
        }

    }

    return info;
}

int parse_data_packet(char * buffer, unsigned short size, int fd, uint8_t seq_number) {

    if (size < 4) {
        printf("Invalid data packet: small size\n");
        return -1;
    }

    if (buffer[0] != CTRL_DATA) {
        printf("Wrong control field value. Got %u expected 1\n", buffer[0]);
        return -1;
    }

    if (buffer[1] != seq_number) {
        printf("Wrong sequence number. Got %u, expected %u\n", buffer[1], seq_number);
        return -1;
    }

    unsigned short length = ((uint8_t) buffer[2] << 8) + (uint8_t) buffer[3];
    if (size != length + 4) {
        printf("Invalid data packet: unexpected file size. Expected %d, got %d\n", size, length+4);
        return -1;
    }
    
    for (unsigned short i = 4; i < length+4; i++) {
        if (write(fd, &buffer[i], 1) == -1) {
            perror("write data");
            return -1;
        }
    }

    return 0;
}

bool matching_ctrl_packets(ctrl_info start, ctrl_info end) {

    // validate filenames
    if(strcmp(start.filename, end.filename)) {
        printf("Conflicting filenames in control packets\n");
        return false;
    }

    long filesize = file_size(start.filename);
    if(filesize != start.filesize || filesize != end.filesize) {
        printf("Wrong filesize in control packets\n");
        return false;
    }

    return true;    
}

int read_file(int fd) {
    
    // TODO check the buffer size
    char buffer[MAX_FRAME_SIZE];
    unsigned short count;
    ctrl_info start_info, end_info;

    // read start control packet
    if ((count = (unsigned short) llread(fd, buffer)) <= 0) {
        printf("Application layer: Couldn't read start packet\n");
        return -1;
    }

    printf("Read start control packet\n");

    // interpret start packet
    start_info = parse_ctrl_packet(buffer, count);
    if (start_info.filename == NULL) {
        printf("Invalid start control packet\n");
        return -1;
    }

    printf("Interpreted start control packet\n");

    // open new file
    int new_fd;
    if((new_fd = open(start_info.filename, O_WRONLY | O_CREAT, 0660)) == -1) {
        perror("open filename");
        return -1;
    }

    // read packets until end
    uint8_t seq_number = 0;
    bool reached_end = false;
    while(!reached_end) {

        if ((count = (unsigned short) llread(fd,buffer)) <= 0) {
            printf("Reached end of input and did not find control end\n");
            return -1;
        }

        // reached final packet
        if (buffer[0] == CTRL_END) {
            reached_end = true;
            break;
        }

        // interpret data
        if (parse_data_packet(buffer, count, new_fd, seq_number++ % UCHAR_MAX) == -1) {
            printf("Error parsing data packet\n");
            return -1;
        }
    }
    
    printf("Read end control packet\n");

    // interpret final packet
    end_info = parse_ctrl_packet(buffer, count);
    if (end_info.filename == NULL) {
        printf("Invalid end control packet\n");
        return -1;
    }
    printf("Interpreted end control packet\n");

    if(close(new_fd) == -1) {
        perror("close");
        return -1;
    }

    if(!matching_ctrl_packets(start_info, end_info)) {
        printf("Error validating control packets\n");
        return -1;
    }

    free(start_info.filename);
    free(end_info.filename);

    return 0;
}