#pragma once
#include <termios.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define FLAG 0x7E
#define EMITTER_A 0x03
#define RECEPTOR_A 0x01
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C_RR_0 0x05
#define C_RR_1 0x85
#define C_REJ_0 0x01
#define C_REJ_1 0x81
#define C_RI_0 0x00
#define C_RI_1 0x40

#define SEND_ATTEMPTS 3
#define TIMEOUT_T 3
#define ESCAPE_CHAR 0x7d
#define FLAG_SUBST 0x5e
#define ESCAPE_SUBST 0x5d

typedef enum {
  STATE_FLAG_I = 0,
  STATE_A = 1,
  STATE_C = 2,
  STATE_BCC = 3,
  STATE_FLAG_E = 4,
  STATE_END = 5,
  STATE_DATA,
  STATE_BCC2,
  STATE_ERROR,
  STATE_WRONG_SEQ_NUM,
  STATE_WRONG_SEQ_END,
  STATE_DISC,
  STATE_DISC_END
} state_machine;

#define MAX_SIZE 255 //TO-DO - Check actual value

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define MAX_TRIES 5
#define DISC_ON_READ -10
typedef enum{
    TRANSMITTER,
    RECEIVER    
} connection_role;

typedef struct{
  char port[20];                 /*Dispositivo /dev/ttySx, x = 0, 1*/
  int baud_rate;                  /*Velocidade de transmissão*/
  unsigned int timeout;          /*Valor do temporizador: 1 s*/
  unsigned int num_transmissions; /*Número de tentativas em caso defalha*/
} link_layer;


