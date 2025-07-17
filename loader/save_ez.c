#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define SERIAL_PORT "/dev/ttyACM0"
#define BAUDRATE B4800
#define BUF_SIZE 1024

// Remove starting line number (digits + optional space) from a line
void remove_line_number(char *line)
{
    char *p = line;
    while (*p >= '0' && *p <= '9')
        p++;
    if (*p == ' ')
        p++;
    memmove(line, p, strlen(p) + 1);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <port> <output_file>\n", argv[0]);
        return 1;
    }

    int serial_fd = open(argv[1], O_RDONLY | O_NOCTTY);
    if (serial_fd < 0)
    {
        perror("open serial port");
        return 1;
    }

    FILE *out = fopen(argv[2], "w");
    if (!out)
    {
        perror("fopen output file");
        close(serial_fd);
        return 1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(serial_fd, &tty) != 0)
    {
        perror("tcgetattr");
        close(serial_fd);
        return 1;
    }

    cfsetospeed(&tty, BAUDRATE);
    cfsetispeed(&tty, BAUDRATE);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8 bits
    tty.c_cflag |= CLOCAL | CREAD;              // Enable receiver, set local mode
    tty.c_cflag &= ~PARENB;                     // No parity
    tty.c_cflag &= ~CSTOPB;                     // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                    // No flow control

    tty.c_lflag = 0; // Non-canonical mode
    tty.c_oflag = 0;
    tty.c_iflag = 0;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0)
    {
        perror("tcsetattr");
        close(serial_fd);
        return 1;
    }

    char buf[BUF_SIZE];
    int buf_pos = 0;
    int stop = 0;

    while (!stop)
    {
        char c;
        ssize_t n = read(serial_fd, &c, 1);
        if (n <= 0)
            break;

        if (c == '\0')
        {
            stop = 1;
            if (buf_pos > 0)
            {
                buf[buf_pos] = 0;
                remove_line_number(buf);
                fprintf(out, "%s\n", buf);
            }
            break;
        }

        if (c == '\n' || c == '\r')
        {
            if (buf_pos > 0)
            {
                buf[buf_pos] = 0;
                remove_line_number(buf);
                fprintf(out, "%s\n", buf);
                buf_pos = 0;
            }
        }
        else if (buf_pos < BUF_SIZE - 1)
        {
            buf[buf_pos++] = c;
        }
    }

    fclose(out);
    close(serial_fd);
    return 0;
}