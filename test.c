#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#define DEVICE                                   "/dev/ttyS3"
#define S_TIMEOUT                                  1

int serial_fd = 0;
unsigned int total_send = 0;
unsigned int total_recv = 0;


int init_serial(void)
{
	serial_fd = open(DEVICE, O_RDWR | O_NOCTTY| O_NDELAY);
	if(serial_fd < 0)
	{
		perror("open failed!");
		exit(0);
	}
	
	struct termios options;
	tcgetattr(serial_fd, &options);
	options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;
	options.c_iflag |= IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	cfsetospeed(&options, B115200);
	tcflush(serial_fd, TCSANOW);
	tcsetattr(serial_fd, TCSANOW, &options);
}

int uart_send(int fd, char *data, int datalen)
{
	int len = 0;
	len = write(fd, data, datalen);
	if(len == datalen)
	{
		total_send = len;
		printf("total_send is %d\n", total_send);
		return len;
	}
	else
	{
		tcflush(fd, TCOFLUSH);
		return -1;
	}
}

int uart_recv(int fd, char *data, int datalen)
{
	int len = 0;
	int ret = 0;
	fd_set fs_read;
	struct timeval tv_timeout;
	FD_ZERO(&fs_read);
	FD_SET(fd, &fs_read);
	
#ifdef S_TIMEOUT
    tv_timeout.tv_sec = (10*20/115200+2);
	tv_timeout.tv_usec = 0;
	ret = select(fd+1, &fs_read, NULL, NULL, &tv_timeout);
#endif
    if(ret == -1)
	{
		printf("timeout ,break");
		return 0;
	}
    if(FD_ISSET(fd, &fs_read))
	{
		len = read(fd, data, datalen);
		total_recv = len;
		printf("total_recv is %d\n", total_recv);
		return len;
	}
    else
	{
		perror("select");
		return -1;
	}
}

int main()
{
	init_serial();
	char trans_data[] = {0xFE};
	char get_data[11];
	memset(get_data, 0, sizeof(char)*11);
	
	
	
	uart_send(serial_fd, trans_data, 11);
	printf("\n");
	usleep(1000);
	uart_recv(serial_fd, get_data, 11);
	printf("receive: %s\n", get_data);
	memset(get_data, 0, sizeof(char)*11);
	
	close(serial_fd);
	return 0;
}

