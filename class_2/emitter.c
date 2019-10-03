#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define FLAG 0x7E
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07
#define SEND_ATTEMPTS 3
#define TIMEOUT_T 3
#define STATE_FLAG_1 0
#define STATE_A 1
#define STATE_C 2
#define STATE_BCC 3
#define STATE_FLAG_2 4 

int send_cnt = 0;
int fd;

void sendTrama() {
    send_cnt++;
    printf("Sending message. Attempt %d\n", send_cnt);
    unsigned char SET[5];
    SET[0] = FLAG;
    SET[1] = A;
    SET[2] = C_SET;
    SET[3] = SET[1] ^ SET[2];
    SET[4] = FLAG;

    write(fd, SET, 5);

    // only try to send msg 3 times
    if (send_cnt == SEND_ATTEMPTS+1) {
        printf("Could not send message. Exiting program.\n");
        exit(1);
    }
    
    alarm(TIMEOUT_T);
}

int main(int argc, char * argv[]) {
    struct termios oldtio,newtio;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */
  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */
    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    (void) signal(SIGALRM, sendTrama);
    sendTrama();
    alarm(TIMEOUT_T);

    unsigned char last = 0;
    int state = 0;
    int end_loop = 0;
    char parity;
    while(!end_loop) {
        read(fd, &last, 1);

        /*if(alarm_triggered) {
            alarm_triggered = false;
            alarm(3);
            continue;
        }
        else {
            // clear alarm
        }*/
        alarm(0);

        switch(state) {
            case STATE_FLAG_1:
                if (last == FLAG) state++;
                else {
                    printf("Received invalid frame %d: %x\n", state, last);
                    end_loop = true;
                }
                break;
            case STATE_A:
                if (last == A) {
                    parity = A;
                    state++;
                }
                else {
                    printf("Received invalid frame %d: %x\n", state, last);
                    end_loop = true;
                }
                break;
            case STATE_C:
                if (last == C_UA) {
                    parity ^= C_UA;
                    state++;
                }
                else {
                    printf("Received invalid frame %d: %x\n", state, last);
                    end_loop = true;
                }
                break;
            case STATE_BCC:
                if (last == parity) state++;
                else {
                    printf("Received invalid frame %d: %x\n", state, last);
                    end_loop = true;
                }
                break;
            case STATE_FLAG_2:
                if (last == FLAG) {
                    printf("Received full message\n");
                }
                else {
                    printf("Received invalid frame %d: %x\n", state, last);
                }
                end_loop = true;
                break;
            default:
                printf("Received unknown frame\n");
                end_loop = true;
        }
    }

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}
