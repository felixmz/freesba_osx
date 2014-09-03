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

#include "defs.h"
#include "fsio.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static char *filebuf = NULL;

char *loadfile(char *name, size_t *size)
{
  struct stat st;
  int fd;

  if (stat(name, &st) < 0) {
    perror("Can not stat file");
    return NULL;
  }

  if (!S_ISREG(st.st_mode)) {
    fprintf(stderr, "%s is not a regular file!\n", name);
    return NULL;
  }

  *size = st.st_size;

  FREE(filebuf);
  filebuf = malloc(st.st_size);
  if (!filebuf) {
    perror("Can not allocate memory");
    return NULL;
  }

  fd = open(name, O_RDONLY);
  if (fd < 0) {
    perror("Can not open file");
    return NULL;
  }

  if (read(fd, filebuf, st.st_size) < 0) {
    perror("Can not read file");
    FREE(filebuf);
    return NULL;
  }

  close(fd);

  return filebuf;
}

int savefile(char *name, char *buf, size_t size)
{
	int retval = 0;
	int fd;

	fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		perror("Can not open file for writing");
		return ERR_IO;
	}

	retval = write(fd, buf, size);
	if (retval < 0) {
		perror("Can not write file");
		retval = ERR_IO;
	}
	if (retval != size) {
		char s[256];
		sprintf(s, "Writen %d bytes while %d requested", retval, size);
		perror(s);
		retval = ERR_IO;
	} else {
		retval = 0;
	}

	close(fd);

	return retval;
}

void freefile()
{
  FREE(filebuf);
}
