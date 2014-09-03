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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "defs.h"
#include "cmdline.h"
#include "comm.h"
#include "fsio.h"
#include "sam.h"

extern int can_exit;
extern int verbose;

const struct cmd_t cmd_list[] = {
	{
		.code = CMD_CLOSE,
		.cmd  = "close",
		.help = "Usage:\n"
				"\tclose\n"
				"Close communication port"
	},
	{
		.code = CMD_CONNECT,
		.cmd  = "connect",
		.help = "Usage:\n\tconnect\n"
				"Connect to microcontroller over serial port.\n"
				"USB ACM interface does not need connection.\n"
				"Connection performed by sending 0x80 0x80 0x23 sequence and\n"
				"waiting for reply ends with '>' character.\n"
				"\tWARNING!\n"
				"Before connection is performed protocol switched to XModem."
	},
	{
		.code = CMD_EXIT,
		.cmd  = "exit",
		.help = "Usage:\n"
				"\texit\n"
				"Exit from " PROJECT
	},
	{
		.code = CMD_FLASH,
		.cmd  = "flash",
		.help = "Usage:\n"
				"\tflash <addr> <filename>\n"
				"Program the given file (binary) to Flash Memory.\n"
				"\t<addr>\t- Address in Flash to program to. The address must be word-aligned.\n"
				"\t<filename> - file to program."
	},
	{
		.code = CMD_GO,
		.cmd  = "go",
		.help = "Usage:\n"
				"\tgo <addr>\n"
				"Begin execution at the given address. The address must be word-aligned."
	},
	{
		.code = CMD_HELP,
		.cmd  = "help",
		.help = "Usage:\n"
				"\thelp [command]\n"
				"Show command list or command's help"
	},
	{
		.code = CMD_INFO,
		.cmd  = "info",
		.help = "Usage:\n"
				"\tinfo\n"
				"Show info about " PROJECT " state"
	},
	{
		.code = CMD_LOCK,
		.cmd  = "lock",
		.help = "Usage:\n"
				"\tlock <value>\n"
				"Set or clear corresponding LOCKS bits.\n"
				"\t<value>\t- LOCKS bits value.\n"
				"\t\tFor example, to set LOCKS0 and LOCKS5 bits\n"
				"\t\tand clear all other You can use command\n"
				"\t\tlock 0x21"
	},
	{
		.code = CMD_NVM,
		.cmd  = "nvm",
		.help = "Usage:\n"
				"\tnvm <value>\n"
				"Set GPNVM bits to <value>."
	},
	{
		.code = CMD_OPEN,
		.cmd  = "open",
		.help = "Usage:\n"
				"\topen [port [baud]]\n"
				"Connect to a microcontroller running SAM-BA over TTY device (USB ACM or serial).\n"
				"\tdevice\t- tty device (/dev/ttyS0, /dev/ttyACM0)\n"
				"\tbaud\t- baudrate\n"
				"Baudrate supported:\n"
				"\t57600\t230400\n"
				"\t115200\t460800\n"
				"Default is: " DEFAULT_PORT " 115200"
	},
	{
		.code = CMD_PROT,
		.cmd  = "prot",
		.help = "Usage:\n"
				"\tprot [<raw> | <xm>]\n"
				"Set communication protocol.\n"
				"Supported protocols are:\n"
				"\traw\t - used with USB ACM interface\n"
				"\txm\t - used with serial TTY interface\n"
				"xm protocol uses XModem for block transmission and 0x80 0x80 0x23 autobaud sequence.\n"
				"Without parameter prints current protocol."
	},
	{
		.code = CMD_READ,
		.cmd  = "read",
		.help = "Usage:\n"
				"\tread <addr>\n"
				"Read a word from the given address. The address must be word-aligned."
	},
	{
		.code = CMD_RECV,
		.cmd  = "recv",
		.help = "Usage:\n"
				"\trecv <addr> <size> <filename>\n"
				"Receive data from RAM or FLASH from the given address with the given size (bytes)\n"
				"and write to file with the given file name.\n"
				"The address must be word-aligned."
	},
	{
		.code = CMD_SEND,
		.cmd  = "send",
		.help = "Usage:\n"
				"\tsend <addr> <filename>\n"
				"Upload the given file (binary) to RAM. This command cannot be used\n"
				"to program FLASH (use the 'flash' command to program FLASH).\n"
				"\t<addr>\t- Address in RAM to upload to. The address must be word-aligned.\n"
				"\t</path/to/file/name> - file to upload.",
	},
	{
		.code = CMD_SHOW,
		.cmd  = "show",
		.help = "Usage:\n"
				"\tshow [id | efc]\n"
				"Show various parameters.\n"
				"\tid\t- Show CID register.\n"
				"\tefc\t- Show Embedded Flash Controller Status.\n"
	},
	{
		.code = CMD_VERSION,
		.cmd  = "version",
		.help = "Usage:\n"
				"\tversion\n"
				"Show version information",
	},
	{
		.code = CMD_WRITE,
		.cmd  = "write",
		.help = "Usage:\n"
				"\twrite <addr> <data>\n"
				"Write a word to the given address in RAM. The address must be word-aligned."
	},

	{.code = 0,} /* Last must be zero */
};

int decode_cmd(char *cmd, int *c)
{
	int i = 0;
	while (cmd_list[i].code) {
		if (strcmp(cmd, cmd_list[i].cmd) == 0) {
			if (c) {*c = i;}
			return cmd_list[i].code;
		}
		i++;
	}
	return -1;
}

#define MAX_PARAM	4

int process_cmdline(const char *command)
{
	int retval = 0;
	char *cmdline;
	char *cmdline_p;
	char *param[MAX_PARAM];
	int cmd;
	int i;
	unsigned long addr;
	char *ok;
	char *c;
	unsigned long data;
	int opened;
	size_t size;
	int show;

	cmdline = strdup(command);
	cmdline_p = cmdline;
	for (i = 0; i < MAX_PARAM; i++)
		param[i] = NULL;
	i = 0;
	while (i < MAX_PARAM) {
		if (!(param[i] = strsep(&cmdline, " ")))
			break;
		i++;
	}
#if 0
	i = 0;
	while (i < MAX_PARAM) {
		if (param[i])
			PDEBUG("PARAM[%d]:\t'%s'\n", i, param[i]);
		else
			break;
		i++;
	}
#endif
	cmd = decode_cmd(param[0], NULL);
	switch (cmd) {
		/*************************************************************************/
		case CMD_EXIT:
			can_exit = 1;
			break;
		/*************************************************************************/
		case CMD_VERSION:
			printf("\n\t" PROJECT " version " PROJECT_VERSION "\n\n");
			printf("This program is free software; you can redistribute it and/or modify\n");
			printf("it under the terms of the GNU General Public License as published by\n");
			printf("the Free Software Foundation; either version 2 of the License, or\n");
			printf("(at your option) any later version.\n");
			printf("\n%s (SAM-BA) is a SAM Boot Agent for \n", PROJECT);
			printf("Atmel AT91SAM microcontrollers.\n");
			printf("It is interfaces with SAM-BA firmware in microcontrollers\n");
			printf("and allow SRAM and Flash programming, view registers etc.\n");
			printf("\nAuthor: Yuri Ovcharenko <amwsoft@gmail.com>\n\n");
			break;
		/*************************************************************************/
		case CMD_HELP:
			if (param[1]) {
				int c;
				if (decode_cmd(param[1], &c) > 0)
					printf("%s\n", cmd_list[c].help);
				else
					printf("Unknown command: '%s'\n", param[1]);
			} else {
				printf("\n");
				i = 0;
				while (cmd_list[i].code) {
					printf("%s\t\t", cmd_list[i].cmd);
					if (i & 1)
						printf("\n");
					i++;
				}
				printf("\n");
			}
			break;
		/*************************************************************************/
		case CMD_INFO:
			comm_info();
			uc_info();
			break;
		/*************************************************************************/
		case CMD_OPEN:
		{
			int baud = 0;
			if (param[2]) {
				char *ok;
				baud = strtoul(param[2], &ok, 0);
				if (*ok != '\0') {
					fprintf(stderr, "Invalid parameter: '%s'\n", param[2]);
					retval = ERR_INVPARAM;
				}
			}
			if (!retval) {
				VERBOSE(1, "Opening port %s at %d baud\n", param[1] == NULL ? DEFAULT_PORT : param[1], baud ? baud : 115200);
				retval = comm_open(param[1], baud);
			}
		}
			break;
		/*************************************************************************/
		case CMD_CLOSE:
			VERBOSE(1, "Closing port %s\n", param[1] == NULL ? DEFAULT_PORT : param[1]);
			comm_close();
			break;
		/*************************************************************************/
		case CMD_READ:
			if (!param[1]) {
				fprintf(stderr, "Too few parameters for command\n");
				retval = ERR_NOPARAM;
			} else {
				addr = strtoul(param[1], &ok, 0);
				if (*ok != '\0') {
					fprintf(stderr, "Invalid parameter: '%s'\n", param[1]);
					retval = ERR_INVPARAM;
				} else {
					opened = comm_opened();
					retval = ucread(addr, &data);
					if (!opened)
						comm_close();
					if (!retval) {
						printf("0x%08lX\n", data);
					}
				}
			}
			break;
		/*************************************************************************/
		case CMD_WRITE:
			if ((!param[1]) || (!param[2])) {
				fprintf(stderr, "Too few parameters for command\n");
				retval = ERR_NOPARAM;
			} else {
				addr = strtoul(param[1], &ok, 0);
				if (*ok != '\0') {
					fprintf(stderr, "Invalid parameter: '%s'\n", param[1]);
					retval = ERR_INVPARAM;
				} else {
					data = strtoul(param[2], &ok, 0);
					if (*ok != '\0') {
						fprintf(stderr, "Invalid parameter: '%s'\n", param[2]);
						retval = ERR_INVPARAM;
					} else {
						opened = comm_opened();
						retval = ucwrite(addr, data);
						if (!opened)
							comm_close();
					}
				}
			}
			break;
		/*************************************************************************/
		case CMD_CONNECT:
			opened = comm_opened();
			VERBOSE(1, "Connecting to microcontroller...\n");
			retval = ucconnect(addr, data);
			if (!opened)
				comm_close();
			printf("%sCONNECTED\n", retval ? "" : "NOT ");
			break;
		/*************************************************************************/
		case CMD_SEND:
			opened = comm_opened();
			VERBOSE(1, "Sending data to RAM...\n");
			if (!param[1] || !param[2]) {
				fprintf(stderr, "Too few parameters for command\n");
				retval = ERR_NOPARAM;
			} else {
				addr = strtoul(param[1], &ok, 0);
				if (*ok != '\0') {
					fprintf(stderr, "Invalid parameter '%s'\n", param[1]);
					retval = ERR_INVPARAM;
				} else {
					c = loadfile(param[2], &size);
					if (!c) {
						return ERR_IO;
					} else {
						retval = ucsend(addr, c, size);
						if (retval < 0) {
							perror("Error sending file");
						}
						freefile();
					}
				}
			}
			if (!opened)
				comm_close();
			break;
		/*************************************************************************/
		case CMD_RECV:
			opened = comm_opened();
			VERBOSE(1, "Receiving data from memory...\n");
			if (!param[1] || !param[2] || !param[3] ) {
				fprintf(stderr, "Too few parameters for command\n");
				retval = ERR_NOPARAM;
			} else {
				addr = strtoul(param[1], &ok, 0);
				if (*ok != '\0') {
					fprintf(stderr, "Invalid parameter '%s'\n", param[1]);
					retval = ERR_INVPARAM;
				} else {
					size = strtoul(param[2], &ok, 0);
					if (*ok != '\0') {
						fprintf(stderr, "Invalid parameter '%s'\n", param[2]);
						retval = ERR_INVPARAM;
					} else {
						c = malloc(size);
						if (!c) {
							perror("No memory for data");
							retval = -ENOMEM;
						} else {
							retval = ucrecv(addr, c, size);
							if (!retval) {
								retval = savefile(param[3], c, size);
								if (!retval) {
									VERBOSE(1, "Done\n");
								}
							}
						}
						FREE(c);
					}
				}
			}
			if (!opened)
				comm_close();
			break;
		/*************************************************************************/
		case CMD_GO:
			opened = comm_opened();
			if (!param[1]) {
				fprintf(stderr, "Too few parameters for command\n");
				retval = ERR_NOPARAM;
			} else {
				addr = strtoul(param[1], &ok, 0);
				if (*ok != '\0') {
					fprintf(stderr, "Invalid parameter: '%s'\n", param[1]);
					retval = ERR_INVPARAM;
				} else {
					retval = ucgo(addr);
					if (!retval) {
						printf("0x%08lX\n", data);
					}
				}
			}
			if (!opened)
				comm_close();
			break;
		/*************************************************************************/
		case CMD_FLASH:
			opened = comm_opened();
			VERBOSE(1, "Programming data to flash...\n");
			if (!param[1] || !param[2]) {
				fprintf(stderr, "Too few parameters for command\n");
				retval = ERR_NOPARAM;
			} else {
				addr = strtoul(param[1], &ok, 0);
				if (*ok != '\0') {
					fprintf(stderr, "Invalid parameter '%s'\n", param[1]);
					retval = ERR_INVPARAM;
				} else {
					c = loadfile(param[2], &size);
					if (!c) {
						return ERR_IO;
					} else {
						retval = ucflash(addr, c, size);
						if (retval < 0) {
							perror("Error programming flash");
						}
						freefile();
					}
				}
			}
			if (!opened)
				comm_close();
			break;
		/*************************************************************************/
		case CMD_PROT:
			if (!param[1]) {
				fprintf(stderr, "Too few parameters for command\n");
				retval = ERR_NOPARAM;
			} else {
				if (strcmp(param[1], "raw") == 0) {
					ucprot(PROT_RAW);
				} else {
					if (strcmp(param[1], "xm") == 0) {
						ucprot(PROT_XM);
					} else {
						fprintf(stderr, "Invalid parameter: '%s'\n", param[1]);
						retval = ERR_INVPARAM;
					}
				}
			}
			break;
		/*************************************************************************/
		case CMD_SHOW:
			opened = comm_opened();
			if (!param[1]) {
				show = 0xFFFFFFFF;
			} else {
				if (strcmp(param[1], "id") == 0)
					show = SHOW_ID;
				else
				if (strcmp(param[1], "efc") == 0)
					show = SHOW_EFC;
				else
				{
					fprintf(stderr, "Invalid parameter: '%s'\n", param[1]);
					retval = ERR_INVPARAM;
				}
			}
			if (!retval) {
				if (show & SHOW_ID) {
					printf("Identification\n");
					retval = ucread(CIDR, &data);
					if (!retval) {
						printf("\tCIDR = 0x%08X\n", data);
						ucshowproc(data, "\t");
						if (!ucarch(data)) {
							printf("\tUnknown architecture!\n");
						} else {
							printf("\t%s%d detected.\n", ucarch(data), ucnvpsiz(data));
							printf("\tFlash page size used: %d\n", ucnvpsiz(data) >= 128 ? 256 : 128);
						}
					}
				}
			}
			if (!retval) {
				if (show & SHOW_EFC) {
					printf("Embedded Flash Controller (EFC)\n");
					retval = ucread(FMR, &data);
					if (!retval) {
						printf("\tFMR = 0x%08X\n", data);
						printf("\t  Flash Status: %s\n", data & MC_FMR_FRDY ? "READY" : "BUSY");
						printf("\t  Errors: PROGE: %d LOCKE: %d\n", data & MC_FMR_PROGE, data & MC_FMR_PROGE);
						printf("\t  Erase Before Programming: %s\n", data & MC_FMR_NEBP ? "NO" : "YES");
						printf("\t  Flash Wait State:\n\t\tRead: %d cycles\n\t\tWrite: %d cycles\n",
						((data & MC_FMR_FWS_MASK) >> MC_FMR_FWS_SHIFT) + 1,
						((data & MC_FMR_FWS_MASK) >> MC_FMR_FWS_SHIFT) == 3 ? ((data & MC_FMR_FWS_MASK) >> MC_FMR_FWS_SHIFT) + 1 : ((data & MC_FMR_FWS_MASK) >> MC_FMR_FWS_SHIFT) + 2);
						printf("\t  Flash Microsecond Cycle Number: %d\n", (data & MC_FMR_FMCN_MASK) >> MC_FMR_FMCN_SHIFT);

						retval = ucread(FSR, &data);
						if (!retval) {
							printf("\tFSR = 0x%08X\n", data);
							printf("\t  Flash Status: %s\n", data & MC_FSR_FRDY ? "READY" : "BUSY");
							printf("\t  Errors: PROGE: %d LOCKE: %d\n", data & MC_FSR_PROGE, data & MC_FSR_PROGE);
							printf("\t  Security bit: %s\n", data & MC_FSR_SECURITY ? "Active" : "Inactive");
							printf("\t  General-purpose NVM\n\t\tGPNVM0: %d BOD:\t     %s\n\t\tGPNVM1: %d BOD Reset: %s\n\t\tGPNVM2: %d Memory:    %s\n",
							data & MC_FSR_GPNVM0 ? 1 : 0, data & MC_FSR_GPNVM0 ? "Enabled" : "Disabled",
							data & MC_FSR_GPNVM1 ? 1 : 0, data & MC_FSR_GPNVM1 ? "Enabled" : "Disabled",
							data & MC_FSR_GPNVM2 ? 1 : 0, data & MC_FSR_GPNVM2 ? "ROM/SRAM" : "Flash/SRAM"
							      );
							printf("\t  LOCKS ");
							for (i = 0; i < 16; i++) {
								printf("%3d", i);
							}
							printf("\n\t\t");
							for (i = 0; i < 16; i++) {
								printf("%3d", data & (1 << (MC_FSR_LOCK_SHIFT + i)) ? 1 : 0);
							}
							printf("\n");
						}
					}
				}
			}
			if (!opened)
				comm_close();
			break;
		/*************************************************************************/
		case CMD_LOCK:
			opened = comm_opened();
			if (!param[1]) {
				fprintf(stderr, "Too few parameters for command\n");
				retval = ERR_NOPARAM;
			} else {
				data = strtoul(param[1], &ok, 0);
				if (*ok != '\0') {
					fprintf(stderr, "Invalid parameter '%s'\n", param[1]);
					retval = ERR_INVPARAM;
				} else {
					if (data & (~0xFFFF)) {
						fprintf(stderr, "Invalid LOCKS value '0x%08X'\n", data);
						retval = ERR_INVPARAM;
					} else {
						retval = ucsetlocks(data);
						if (retval) {
							perror("Read/Write error");
						}
					}
				}
			}
			if (!opened)
				comm_close();
			break;
		/*************************************************************************/
		case CMD_NVM:
			opened = comm_opened();

			if (!param[1]) {
				fprintf(stderr, "Too few parameters for command\n");
				retval = ERR_NOPARAM;
			} else {
				data = strtoul(param[1], &ok, 0);
				if (*ok != '\0') {
					fprintf(stderr, "Invalid parameter '%s'\n", param[1]);
					retval = ERR_INVPARAM;
				} else {
					if (data > 7) {
						fprintf(stderr, "Invalid GPNVM value '0x%08X'\n", data);
						retval = ERR_INVPARAM;
					} else {
						retval = ucsetnvm(data);
						if (retval < 0) {
							perror("Read/Write error");
						}
					}
				}
			}

			if (!opened)
				comm_close();
			break;
		/*************************************************************************/
		case 0:
			opened = comm_opened();
			if (!opened)
				comm_close();
			break;
		/*************************************************************************/
		default:
			fprintf(stderr, "Unknown command: '%s'\n", param[0]);
			retval = ERR_UNKNOWN;
			break;
	}
	free(cmdline_p);
	return retval;
}

/* End of file */
