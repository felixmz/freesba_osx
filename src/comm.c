/***************************************************************************
 *   Copyright (C) 2006 by Yuri Ovcharenko                                 *
 *   amwsoft@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "defs.h"
#include "comm.h"

extern int verbose;

static int port = -1;
static int baud = DEFAULT_BAUD;
static char *port_name = NULL;

static struct termios ios, old_ios;

static char *set_port_name(char *name)
{
	if (port_name)
		free(port_name);
	if (name) {
		VERBOSE(3, "Set new port name\n");
		port_name = strdup(name);
	}
	else
		port_name = strdup(DEFAULT_PORT);
	return port_name;
}

int comm_open(char *name, int b)
{
	if (port > 0)
		comm_close();
	if (name)
		set_port_name(name);
	if (b) ios_init(b);

	VERBOSE(2, "Opening port...\n");
	port = open(port_name, O_RDWR | O_NONBLOCK | O_NOCTTY);
	if (port < 0) {
		perror("Can not open port");
		return -1;
	}
	VERBOSE(3, "Retrive old port attributes...\n");
	if (tcgetattr(port, &old_ios) < 0) {
		perror("Can not get port parameters");
		comm_close();
		return -1;
	}
	VERBOSE(3, "Apply new port attributes...\n");
	if (tcsetattr(port, TCSANOW, &ios) < 0) {
		perror("Can not set port parameters");
		comm_close();
		return -1;
	}

	VERBOSE(2, "Opened.\n");
	return 0;
}

void comm_close()
{
	VERBOSE(2, "Closing port...\n");
	if (port < 0)
		return;
	VERBOSE(3, "Apply old port attributes...\n");
	tcsetattr(port, TCSANOW, &old_ios);
	close(port);
	port = -1;
}

int comm_opened()
{
	return port >= 0;
}

static void ios_init(int b)
{
	VERBOSE(2, "Initializing termios structure.\n");
	cfmakeraw(&ios);

	/* Input flags */
	ios.c_iflag |= IGNBRK | IGNPAR;
	/* Control flags */
	ios.c_cflag |= CREAD | CLOCAL;
	ios.c_cflag &= ~CRTSCTS;
	/* Control char's */
	ios.c_cc[VINTR] = 0;
	ios.c_cc[VQUIT] = 0;
	ios.c_cc[VERASE] = 0;
	ios.c_cc[VKILL] = 0;
	ios.c_cc[VEOF] = 0;
	ios.c_cc[VMIN] = 0;
	ios.c_cc[VEOL] = 0;
	ios.c_cc[VTIME] = 0;
	ios.c_cc[VEOL2] = 0;
	ios.c_cc[VSTART] = 0;
	ios.c_cc[VSTOP] = 0;
	ios.c_cc[VSUSP] = 0;
	ios.c_cc[VLNEXT] = 0;
	ios.c_cc[VWERASE] = 0;
	ios.c_cc[VREPRINT] = 0;
	ios.c_cc[VDISCARD] = 0;

	baud = DEFAULT_BAUD;
	switch (b) {
		case 57600:
			baud = B57600;
			break;
		case 115200:
			baud = B115200;
			break;
		case 230400:
			baud = B230400;
			break;
		case 460800:
			baud = B460800;
			break;
	}
	cfsetospeed(&ios, baud);
	cfsetispeed(&ios, baud);
}

void comm_init(char *name, int baud)
{
	set_port_name(name);
	ios_init(baud);
}

void comm_clean()
{
	comm_close();
	if (port_name)
		free(port_name);
	port_name = NULL;
}

void comm_info()
{
	printf("Port:\t\t%s %s\n", port_name, comm_opened() ? "opened." : "closed.");
	printf("Baud:\t\t");
	switch (baud) {
		case B57600:
			printf("57600");
			break;
		case B230400:
			printf("230400");
			break;
		case B460800:
			printf("460800");
			break;
		case B115200:
		default:
			printf("115200");
			break;
	}
	printf(".\n");
}

int comm_read(char *data, size_t size)
{
	int retval = 0;

	if (port < 0)
		retval = comm_open(NULL, 0);
	if (retval < 0)
		return retval;
	VERBOSE(4, "Reading from device...\n");
	return read(port, data, size);
}

int comm_write(char *data, size_t len)
{
	int retval = 0;
	int count;

	if (port < 0)
		retval = comm_open(NULL, 0);
	if (retval < 0)
		return retval;

	count = len;
	while (count > 0) {
		VERBOSE(4, "Writing to device... ");
		retval = write(port, data, count);
		if (retval < 0) {
			if (errno == EAGAIN)
				continue;
			else
				return retval;
		}
		VERBOSE(4, "%d bytes.\n", retval);
		count -= retval;
		data += retval;
	}

	return len;
}

/* End of file */
