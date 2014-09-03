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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "defs.h"
#include "batch.h"

extern int verbose;

static int batch = -1;
static char *source = NULL;
static char *position = NULL;
static int filesize = 0;

int batch_init(char *filename)
{
	int retval;
	struct stat st;

	batch_clean();

	retval = stat(filename, &st);
	if (retval < 0) {
		perror("Can not stat source file");
		return retval;
	}

	source = malloc(st.st_size);
	if (!source) {
		perror("Can not allocate memory for source file");
		return ERR_MEM;
	}

	filesize = st.st_size;
	VERBOSE(3, "Source file %s size %d\n", filename, filesize);

	batch = open(filename, O_RDONLY);
	if (batch < 0) {
		perror("Can not open source file for reading");
		batch_clean();
		return batch;
	}

	retval = read(batch, source, filesize);
	if (retval < 0) {
		perror("Can not read source file");
		batch_clean();
		return retval;
	}

	position = source;
	VERBOSE(3, "Initial position 0x%08X\n", source);

	return 0;
}

void batch_clean()
{
	if (batch > 0)
		close(batch);
	FREE(source);
	position = NULL;
	filesize = 0;
	batch = -1;
}

char *read_batch_line()
{
	char *s;
	int size;

	VERBOSE(3, "Reading line from source file...\n");
	if ((source == NULL) || (position == NULL))
		return NULL;

	if ((position - source) < filesize) {
		VERBOSE(3, "Finding line... ");
		s = strstr(position, "\n");
		if (s != NULL) {
			size = s - position;
			VERBOSE(3, "found %d characters\n", size);
			s = malloc(size + 1);
			if (s == NULL) {
				perror("Can not allocate memory for command line");
			} else {
				VERBOSE(3, "Duplicating command line\n");
				memset(s, 0, size + 1);
				memcpy(s, position, size);
				VERBOSE(3, "Advance position by %d = ", size);
				position += size;
				position++;
				VERBOSE(3, "0x%08X\n", position);
			}
		} else {
			VERBOSE(3, "not found\n");
		}

		return s;
	}

	return NULL;
}

/* End of file */
