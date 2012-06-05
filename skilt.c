/*
  Simple test program demonstrating how to interface with a Mobitec flip dot display
  using RS-485.
 */

#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

/*
  Calculates the checksum and adds it to data. Size is adjusted accordingly.
*/
static void add_checksum(void *data, int *size)
{
	int csum = 0;
	int i;
	unsigned char *p = (unsigned char *)data;

	for(i=1; i<*size; i++)
	{
		csum += p[i];
	}

	p[i] = csum & 0xff;

	(*size)++;
	if(p[i] == 0xfe)
	{
		p[i+1] = 0x00;
		(*size)++;
	} else if(p[i] == 0xff)
	{
		p[i] = 0xfe;
		p[i+1] = 0x01;
		(*size)++;
	}
}

static int fd;

/* Simple header with start byte 0xff, sign address 0x06, and 0xa2 for text mark */
static const unsigned char sign_hdr[] = {0xff, 0x06, 0xa2};

/*
  Writes the supplied ascii string to the display
*/
static void write_text(const char *txt)
{
	unsigned char msg[1024];
	int len;
	
	memcpy(msg, sign_hdr, sizeof(sign_hdr));
	len = sizeof(sign_hdr);
	strcpy((char *)msg + len, txt);
	len += strlen(txt);
	add_checksum(msg, &len);
	msg[len++] = 0xff;
	
	if(write(fd, msg, len) < len)
	{
		perror("writing failed");
	}
}

/*
  Sets all pixels except the first column
*/
static void write_all_white(void)
{
	char buffer[1024];
	int len;

	len = 0;
	buffer[len++] = 0xd2; // Set x coordinate to 1
	buffer[len++] = 1;
	buffer[len++] = 0xd3; // Set y coordinate to 4
	buffer[len++] = 4;
	buffer[len++] = 0xd4; // Select bitmap font
	buffer[len++] = 0x77;
	// 0x20 - 0x3f is used with the bitmap font
	// 0x20 is no pixels set and 0x3f is all pixels set
	memset(buffer+len, 0x3f, 112);
	len += 112;

	buffer[len++] = 0xd2;
	buffer[len++] = 1;
	buffer[len++] = 0xd3;
	buffer[len++] = 9;
	buffer[len++] = 0xd4;
	buffer[len++] = 0x77;
	memset(buffer+len, 0x3f, 112);
	len += 112;

	buffer[len++] = 0xd2;
	buffer[len++] = 1;
	buffer[len++] = 0xd3;
	buffer[len++] = 14;
	buffer[len++] = 0xd4;
	buffer[len++] = 0x77;
	memset(buffer+len, 0x3f, 112);
	len += 112;

	buffer[len++] = 0xd2;
	buffer[len++] = 1;
	buffer[len++] = 0xd3;
	buffer[len++] = 19;
	buffer[len++] = 0xd4;
	buffer[len++] = 0x77;
	memset(buffer+len, 0x3f, 112);
	len += 112;

	buffer[len] = '\0';
	write_text(buffer);
}

int main(int argc, char *argv[])
{
	struct termios term_opt;
	fd = open("/dev/ttyUSB0", O_RDWR);

	if(fd <= 0)
	{
		perror("failed to open serial port");
		return -1;
	}
	
	tcgetattr(fd, &term_opt);
	
	cfmakeraw(&term_opt);
	cfsetispeed( &term_opt, B4800);
	cfsetospeed( &term_opt, B4800);
	tcsetattr(fd, TCSANOW, &term_opt);

	while(1) {
		write_text("Davs");
		usleep(4*1000*1000);
		write_text("Hejsa");
		usleep(4*1000*1000);
		write_all_white();
		usleep(4*1000*1000);
	}

	close(fd);
	
	return 0;
}
