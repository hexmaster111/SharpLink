#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#define SERIAL_PORT "/dev/ttyACM0"
void configure_serial(int fd)
{
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0)
    {
        perror("tcgetattr");
        exit(1);
    }

    cfsetospeed(&tty, B4800);
    cfsetispeed(&tty, B4800);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8 bits
    tty.c_cflag &= ~PARENB;                     // No parity
    tty.c_cflag &= ~CSTOPB;                     // 1 stop bit
    tty.c_cflag |= CREAD | CLOCAL;              // Enable receiver, ignore modem ctrl lines

    tty.c_lflag = 0; // No canonical mode, no echo
    tty.c_oflag = 0; // No remapping, no delays
    tty.c_iflag = 0; // No special input processing

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        perror("tcsetattr");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    int serial_fd = open(SERIAL_PORT, O_WRONLY | O_NOCTTY);
    if (serial_fd < 0)
    {
        perror("open serial port");
        return 1;
    }
    configure_serial(serial_fd);

    FILE *file = fopen(argv[1], "rb");
    if (!file)
    {
        perror("fopen");
        close(serial_fd);
        return 1;
    }

    char buf[1024];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), file)) > 0)
    {
        if (write(serial_fd, buf, n) != n)
        {
            perror("write");
            fclose(file);
            close(serial_fd);
            return 1;
        }
    }

    fclose(file);
    close(serial_fd);
    return 0;
}