#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#define SERIAL_PORT "/dev/ttyACM0" 


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s output_file\n", argv[0]);
        return 1;
    }

    int serial_fd = open(SERIAL_PORT, O_RDONLY | O_NOCTTY);
    if (serial_fd < 0) {
        perror("open serial port");
        return 1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(serial_fd, &tty) != 0) {
        perror("tcgetattr");
        close(serial_fd);
        return 1;
    }

    cfsetospeed(&tty, B4800);
    cfsetispeed(&tty, B4800);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8 bits
    tty.c_cflag |= CLOCAL | CREAD;              // Enable receiver, set local mode
    tty.c_cflag &= ~PARENB;                     // No parity
    tty.c_cflag &= ~CSTOPB;                     // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                    // No flow control

    tty.c_iflag = IGNPAR;                       // Ignore framing/parity errors
    tty.c_oflag = 0;
    tty.c_lflag = 0;                            // Raw input

    tcflush(serial_fd, TCIFLUSH);
    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(serial_fd);
        return 1;
    }

    FILE *out = fopen(argv[1], "wb");
    if (!out) {
        perror("fopen output file");
        close(serial_fd);
        return 1;
    }

    unsigned char buf[256];
    ssize_t n;
    int stop = 0;
    while (!stop && (n = read(serial_fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; ++i) {
            if (buf[i] == 0) {
                fwrite(buf, 1, i, out);
                stop = 1;
                break;
            }
        }
        if (!stop)
            fwrite(buf, 1, n, out);
    }

    fclose(out);
    close(serial_fd);
    return 0;
}