
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "constants.h"

int stuff_buffer(unsigned char ** buffer, int length) {
    // determine new length
    int new_length = 0;
    for (int i = 0; i < length; i++) {
        new_length++;
        if ((*buffer)[i] == FLAG || (*buffer)[i] == ESCAPE_CHAR) new_length++;
    }

    // create new buffer
    unsigned char * new_buffer = (unsigned char*) malloc(new_length);
    memset(new_buffer, 0, sizeof(unsigned char) * new_length);

    // fill new buffer
    for (int i = 0, ins_pos = 0; i < length; i++) {
        if((*buffer)[i] == FLAG) {
            new_buffer[ins_pos] = ESCAPE_CHAR;
            new_buffer[ins_pos+1] = FLAG_SUBST;
            ins_pos++;
        }
        else if((*buffer)[i] == ESCAPE_CHAR) {
            new_buffer[ins_pos] = ESCAPE_CHAR;
            new_buffer[ins_pos+1] = ESCAPE_SUBST;
            ins_pos++;
        }
        else {
            new_buffer[ins_pos] = (*buffer)[i];
        }
        ins_pos++;
    }

    // assign new variables
    *buffer = new_buffer;
    return new_length;
}

int destuff_buffer(unsigned char ** buffer, int length) {
    // determine new_length
    int new_length = length;
    for (int i = 0 ; i < length; i++) {
        if ((*buffer)[i] == ESCAPE_CHAR) new_length--;
    }

    // create new buffer
    unsigned char * new_buffer = (unsigned char *) malloc(new_length);
    memset(new_buffer, 0, sizeof(unsigned char) * new_length);

    // fill new buffer
    for (int i = 0, ins_pos = 0; i < length; i++, ins_pos++) {        
        if ((*buffer)[i] == ESCAPE_CHAR) {
            if ((*buffer)[i+1] == FLAG_SUBST) {
                new_buffer[ins_pos] = FLAG;
            }
            else if ((*buffer)[i+1] == ESCAPE_SUBST) {
                new_buffer[ins_pos] = ESCAPE_CHAR;
            }
            i++;
        }
        else {
            new_buffer[ins_pos] = (*buffer)[i];
        }
    }

    // assign new variables
    *buffer = new_buffer;
    return new_length;
}