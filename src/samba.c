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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "defs.h"
#include "cmdline.h"
#include "comm.h"
#include "sam.h"
#include "batch.h"

int can_exit = 0;
int batch_mode = 0;
int verbose = 0;

struct option long_opt [] = {
	{"interface", required_argument, 0, 'i'},
	{"baud", required_argument, 0, 'b'},
	{"protocol", required_argument, 0, 'p'},
	{"source", required_argument, 0, 's'},
	{"verbose", no_argument, 0, 'v'},
	{"quiet", no_argument, 0, 'q'},
	{"version", no_argument, 0, 'V'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0},
};

void usage();

int main(int argc, char *argv[])
{
	int retval;
	char *cmd;
	int c;
	int opt_index;
	char *port = NULL;
	int baud = 0;
	char *ok;
	char *source = NULL;

	while (1) {
		c = getopt_long(argc, argv, "i:b:p:s:vqVh", long_opt, &opt_index);
		if (c == -1)
			break;
		switch (c) {
			case 0:
				printf("\ngetopt_long == 0\noption == '%s'\n", long_opt[opt_index].name);
				if (optarg)
					printf(" with arg '%s'", optarg);
				printf("\n");
				break;
			case 'i':
				port = optarg;
				break;
			case 'b':
				baud = strtol(optarg, &ok, 0);
				if (!(*ok == '\0')) {
					fprintf(stderr, "Invalid baud rate: '%s'\n", optarg);
					return 254;
				}
				break;
			case 'p':
				if (strcmp(optarg, "raw") == 0) {
					ucprot(PROT_RAW);
				} else {
					if (strcmp(optarg, "xm") == 0) {
						ucprot(PROT_XM);
					} else {
						fprintf(stderr, "Invalid protocol: '%s'\n", optarg);
						return 254;
					}
				}
				break;
			case 's':
				batch_mode = 1;
				source = optarg;
				break;
			case 'v':
				++verbose;
				break;
			case 'q':
				verbose = -1;
				break;
			case 'V':
				printf(PROJECT " version " PROJECT_VERSION "\n");
				return 0;
				break;
			case 'h':
				usage();
				return 0;
				break;
			case '?':
				return 254;
				break;
		}
	}

	if (batch_mode) {
		VERBOSE(1, "Initialising batch mode\n");
		retval = batch_init(source);
		if (retval) {
			return retval;
		}
	}

	comm_init(port, baud);

	printf("Type 'help' to view list of supported commands\n"
			"or 'help <command>' to view command's help\n");
	while (!can_exit) {
		if (batch_mode)
			cmd = read_batch_line();
		else
			cmd = readline(PROJECT "> ");
		retval = 0;
		if (cmd) {
			if (strlen(cmd) > 0) {
				retval = process_cmdline(cmd);
				if (batch_mode)
					if (retval)
						can_exit = 1;
				add_history(cmd);
			}
			free(cmd);
		} else {
			if (batch_mode)
				can_exit = 1;
		}
	}

	comm_clean();
	batch_clean();

	return retval;
}

void usage()
{
	printf("Usage: samba [OPTIONS]\n");
	printf("OPTIONS:\n");
	printf("\t-i DEVICE\n\t--interface=DEVICE\n");
	printf("\t\tUse DEVICE to communicate with microcontroller (" DEFAULT_PORT ").\n");
	printf("\t-b BAUDRATE\n\t--baud=BAUDRATE\n");
	printf("\t\tCommunicate with microcontroller at the given BAUDRATE (115200).\n");
	printf("\t-p PROTOCOL\n\t--protocol=PROTOCOL\n");
	printf("\t\tSupported protocols is 'raw' and 'xm'.\n"
			"\t\tThe 'raw' protocol used over USB ACM interface\n"
			"\t\tand the 'xm' (XModem) used over serial TTY interface.\n"
			"\t\tDefault is xm.\n");
	printf("\t-s FILE\n\t--source=FILE\n");
	printf("\t\tUse commands from FILE instead of stdin (batch mode).\n");
	printf("\t-v\n\t--verbose\n");
	printf("\t\tBe verbose. More options increase verbose level.\n");
	printf("\t-q\n\t--quiet\n");
	printf("\t\tBe quiet. No output to stdout.\n");
	printf("\t-v\n\t--version\n");
	printf("\t\tPrint version and exit.\n");
	printf("\t-h\n\t--help\n");
	printf("\t\tPrint this screan and exit.\n");
}

/* End of file */
