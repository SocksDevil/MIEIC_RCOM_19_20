#define FLAG 0x7E
#define A 0x03
#define C_SET 0x03

int main() {
    unsigned char SET[5];
    SET[0] = FLAG;
    SET[1] = A;

    SET[3] = SET[1] ^ SET[2];
}