#include "llinterpretation.h"
#include "constants.h"
#include "stdlib.h"



int interpreter(unsigned char ** buffer){

    unsigned char * new_buffer = (unsigned char*) malloc(MAX_BUFFER);
    int size = 1;
    for(;;size++){
        if((*buffer)[size] == FLAG) 
            break;
            
    }

    char bcc2 = (*buffer)[size-2];

    int new_size = 0;
    for(int i = 4;i < size-2;i++){
        new_buffer[new_size]=0;
        new_size++;

    }
    new_size++;

    char xor = new_buffer[0];
    for(int i =1; i < new_size;i++){
        xor^=new_buffer[i];
    }

    if(xor!=bcc2)
        return -1;

    free(*buffer);
    (*buffer) = new_buffer;

    return new_size;

}