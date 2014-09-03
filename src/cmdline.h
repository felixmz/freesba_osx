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

#ifndef _CMDLINE_H
#define _CMDLINE_H

#define ERR_UNKNOWN	(ERROR_CMD + 0x01)
#define ERR_NOPARAM	(ERROR_CMD + 0x02)
#define ERR_INVPARAM	(ERROR_CMD + 0x03)

#define CMD_EXIT	1
#define CMD_VERSION	2
#define CMD_HELP	3
#define CMD_READ	4
#define CMD_WRITE	5
#define CMD_INFO	6
#define CMD_OPEN	7
#define CMD_CLOSE	8
#define CMD_CONNECT	9
#define CMD_SEND	10
#define CMD_RECV	11
#define CMD_GO		12
#define CMD_FLASH	13
#define CMD_PROT	14
#define CMD_SHOW	15
#define CMD_LOCK	16
#define CMD_NVM		17

#define SHOW_ID		1
#define SHOW_EFC	2

struct cmd_t {
	int code;
	char *cmd;
	char *help;
};

int process_cmdline(const char *command);

#endif /* _CMDLINE_H */
