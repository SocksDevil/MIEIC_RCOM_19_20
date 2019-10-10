#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define FLAG 0x7E
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07
#define SEND_ATTEMPTS 3
#define TIMEOUT_T 3
#define STATE_FLAG_I 0
#define STATE_A 1
#define STATE_C 2
#define STATE_BCC 3
#define STATE_FLAG_E 4
#define STATE_END 5

#define MAX_SIZE 255 //TO-DO - Check actual value

#define _POSIX_SOURCE 1 /* POSIX compliant source */

typedef enum{
    TRANSMITTER,
    RECEIVER    
} connection_role;

typedef struct{
  char port[20];                 /*Dispositivo /dev/ttySx, x = 0, 1*/
  int baud_rate;                  /*Velocidade de transmissão*/
  unsigned int sequence_number;   /*Número de sequência da trama: 0, 1*/
  unsigned int timeout;          /*Valor do temporizador: 1 s*/
  unsigned int num_transmissions; /*Número de tentativas em caso defalha*/
  char frame[MAX_SIZE];          /*Trama*/
} link_layer;