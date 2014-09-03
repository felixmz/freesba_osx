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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "defs.h"
#include "sam.h"
#include "comm.h"
#include "crc-1021.h"

extern const int sambaflash_size;
extern const char sambaflash[];

static int prot = PROT_RAW;
static int timeout = DEFAULT_TIMEOUT;

extern int verbose;

int ucprot(int p)
{
	if ((p == PROT_RAW) || (p == PROT_XM)) {
		VERBOSE(2, "Setting protocol to %s.\n", p == PROT_XM ? "XModem" : "RAW");
		prot = p;
	}
	return prot;
}

int ucread(unsigned long addr, unsigned long *data)
{
	char *c;
	char C;
	char **pos;
	int retval;
	char obuf[32];
	char ibuf[32];
	char *buf = ibuf;
	unsigned long d = 0;
	struct timeval tv, tvc;
	int i;
	int tm = 0;

	VERBOSE(2, "Reading from microntroller...\n");

	*data = 0;
	if (addr & 0x3)
		return ERR_ALIGN;

	if ((ucprot(-1) == PROT_XM) && (!ucconnect())) {
		fprintf(stderr, "Can not connect!\n");
		return ERR_CONNECT;
	}

	sprintf(obuf, "w%08lX,#", addr);

	VERBOSE(3, "Sending '%s'...\n", obuf);

	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0)
		return retval;

	memset(ibuf, 0, sizeof(ibuf));
	retval = gettimeofday(&tv, NULL);
	if (retval < 0) {
		perror("Can not get current time");
		usleep(100000);
		VERBOSE(3, "Read response...\n");
		retval = comm_read(ibuf, sizeof(ibuf));
		if (retval < 0)
			return retval;
	} else {
		VERBOSE(3, "Read response...\n");
		i = 0;
		tvc.tv_sec = tv.tv_sec;
		tvc.tv_usec = tv.tv_usec;
		/* 0.5 sec timeout */
		while ( (tm = ((tvc.tv_sec * 1000000 + tvc.tv_usec) - (tv.tv_sec * 1000000 + tv.tv_usec))) < timeout) {
			retval = comm_read(&C, 1);
			if (retval < 0)
				return retval;
			if (retval > 0) {
				ibuf[i] = C;
				if (i < sizeof(ibuf) - 2)
					i++;
				if (C == '>') {
					break;
				}
			}
			retval = gettimeofday(&tvc, NULL);
			if (retval < 0) {
				perror("Can not get current time");
				return retval;
			}
		}
	}

	if (tm >= timeout) {
		fprintf(stderr, "Timeout\n");
		return ERR_TIMEOUT;
	}

	VERBOSE(3, "Response is '%s'\n", ibuf);

	if (!strstr(ibuf, ">")) {
		printf("Incomplete response from microcontroller.\n");
		return ERR_INCOMPLETE;
	}

	pos = &buf;
	c = strsep(pos, "\r");
	if (!c) {
		printf("Unrecognized response from microcontroller.\n");
		return ERR_UNRECOGNIZED;
	}
	c = strsep(pos, "\n");
	if (!c) {
		printf("Unrecognized response from microcontroller.\n");
		return ERR_UNRECOGNIZED;
	}

	d = strtoul(c, &buf, 0);
	if (*buf != '\0')
		return -1;

	*data = d;

	VERBOSE(2, "Reading done.\n");

	return 0;
}

int ucwrite(unsigned long addr, unsigned long data)
{
	int retval;
	char obuf[32];
	char ibuf[32];

	VERBOSE(2, "Writing to microntroller...\n");

	if (addr & 0x3)
		return ERR_ALIGN;

	if ((ucprot(-1) == PROT_XM) && (!ucconnect())) {
		fprintf(stderr, "Can not connect!\n");
		return ERR_CONNECT;
	}

	sprintf(obuf, "W%08lX,%08lX#", addr, data);

	VERBOSE(3, "Sending '%s'...\n", obuf);

	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0)
		return retval;

	retval = ucwaitresp('>');

	VERBOSE(2, "Writing done.\n");

	return retval;
}

int ucwaitresp(char resp)
{
	char C;
	int retval;
	struct timeval tv, tvc;
	int tm = 0;

	VERBOSE(2, "Wait response...\n");
	VERBOSE(3, "Wait for '0x%02X'\n", resp);

	retval = gettimeofday(&tv, NULL);
	if (retval < 0) {
	  perror("Can not get current time");
	  return retval;
	} else {
		tvc.tv_sec = tv.tv_sec;
		tvc.tv_usec = tv.tv_usec;
		/* 0.5 sec timeout */
		while ( (tm = ((tvc.tv_sec * 1000000 + tvc.tv_usec) - (tv.tv_sec * 1000000 + tv.tv_usec))) < timeout) {
			retval = comm_read(&C, 1);
			if (retval < 0)
				return retval;
			if (retval > 0) {
				VERBOSE(3, " 0x%02X", C);
				if (C == resp)
					break;
			}
			retval = gettimeofday(&tvc, NULL);
			if (retval < 0) {
				perror("Can not get current time");
				return retval;
			}
		}
	}

	retval = tm >= timeout ? ERR_TIMEOUT : 0;

	VERBOSE(3, "\nWait status %d\n", retval);

	return retval;
}

int ucconnect()
{
	int count = 8;
	int retval;
	char obuf[32];
	char ibuf[32];

	VERBOSE(2, "Connecting...\n");
	ucprot(PROT_XM);

	memset(obuf, 0, sizeof(obuf));
	obuf[0] = 0x80;
	obuf[1] = 0x80;
	obuf[2] = '#';

	while (count) {
		VERBOSE(3, "Sending '%s'...\n", obuf);
		retval = comm_write(obuf, 3);
		if (retval < 0)
			return retval;
		retval = ucwaitresp('>');
		if (retval < 0)
			return retval;
		if (!retval)
			break;
		count--;
	}

	retval = count > 0;

	VERBOSE(3, "Connecting status (%d): %sCONNECTED\n", count, retval ? "" : "NOT ");
	VERBOSE(2, "Connecting %s.\n", retval ? "done" : "failed");

	return retval;
}

void uc_info()
{
	printf("Protocol:\t%s\n", ucprot(-1) == PROT_XM ? "XModem" : "RAW");
}

int ucsend(unsigned long addr, char *buf, size_t len)
{
	int retval;
	int count;
	int blockcount;
	int block;
	unsigned char obuf[256];
	unsigned char ibuf[32];
	unsigned short crc;
	char resp;

	if (addr & 0x3)
		return 0;

	if ((ucprot(-1) == PROT_XM) && (!ucconnect())) {
		fprintf(stderr, "Can not connect!\n");
		return ERR_CONNECT;
	}

	sprintf(obuf, "S%08lX,%08lX#", addr, len);

	VERBOSE(3, "Sending '%s'\n", obuf);
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0)
		return retval;

	if (ucprot(-1) == PROT_XM) {
		resp = 'C';
	} else {
		resp = '\r';
	}
	retval = ucwaitresp(resp);
	if (retval != 0) {
		if (retval > 0) {
			fprintf(stderr, "Can not start sending. Microcontroller not responding.\n");
			return ERR_TIMEOUT;
		}
		else
			return retval;
	}

	blockcount = len / XM_SIZE;
	if (len & XM_MASK)
		blockcount++;
	block = 1;
	VERBOSE(0, "Sending data to RAM using ");
	VERBOSE(0, "%s", ucprot(-1) ? "XModem" : "RAW");
	VERBOSE(0, " protocol with block size %d bytes.\n", XM_SIZE);
	VERBOSE(0, "Bytes: %d, Blocks: %d\n", len,  blockcount);
	count = len;
	while (count > 0) {
		VERBOSE(0, "\rSending Block %d / %d ", block, blockcount);
		memset(obuf, 0, sizeof(obuf));
		if (ucprot(-1) == PROT_XM) {
			obuf[0] = SOH;
			obuf[1] = block & 0xFF;            /* BLK # */
			obuf[2] = 255 - (block & 0xFF);    /* ! BLK # */
			memcpy(&obuf[3], buf, XM_SIZE < count ? XM_SIZE : count);
			crc = crc_1021(0, &obuf[3], XM_SIZE);
			obuf[XM_SIZE + 3] = (crc >> 8) & 0xFF;
			obuf[XM_SIZE + 3 + 1] = crc & 0xFF;
		} else {
			memcpy(obuf, buf, XM_SIZE < count ? XM_SIZE : count);
		}
		if (ucprot(-1) == PROT_XM)
			retval = comm_write(obuf, XM_SIZE + 5);
		else
			retval = comm_write(obuf, XM_SIZE);
		if (retval < 0)
			return retval;

		block++;
		count -= XM_SIZE;
		buf += XM_SIZE;

		if (ucprot(-1) == PROT_XM) {
			VERBOSE(3, "Wait ACK\n");
			retval = ucwaitresp(ACK);
			if (retval != 0) {
				if (retval > 0) {
					fprintf(stderr, "Packet is not acknowledged. Microcontroller not responding.\n");
					return ERR_TIMEOUT;
				}
				else
					return retval;
			}
		}
	}

	if (ucprot(-1) == PROT_XM) {
		obuf[0] = EOT;
		VERBOSE(3, "Sending EOT\n");
		retval = comm_write(obuf, 1);
		if (retval < 0)
			return retval;
	}

	retval = ucwaitresp('>');
	if (retval != 0) {
		if (retval > 0) {
			fprintf(stderr, "Can not finish sending. Microcontroller not responding.\n");
			return ERR_TIMEOUT;
		}
		else
			return retval;
	}

	VERBOSE(0, "Finished\n");

	return retval;
}

int ucrecv(unsigned long addr, char *buf, size_t len)
{
	int retval;
	int count;
	int blockcount;
	int block;
	int pos;
	unsigned char obuf[256];
	unsigned char ibuf[256];
	unsigned short crc;
	char resp;

	if (addr & 0x3)
		return 0;

	if ((ucprot(-1) == PROT_XM) && (!ucconnect())) {
		fprintf(stderr, "Can not connect!\n");
		return ERR_CONNECT;
	}

	sprintf(obuf, "R%08lX,%08lX#", addr, len);

	VERBOSE(3, "Sending '%s'\n", obuf);
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0)
		return retval;

	if (ucprot(-1) == PROT_XM) {
		usleep(10000);
		VERBOSE(3, "Sending 'C'\n");
		obuf[0] = 'C';
		retval = comm_write(obuf, 1);
		if (retval < 0)
			return retval;
	}

	resp = '\r';
	retval = ucwaitresp(resp);
	if (retval != 0) {
		if (retval > 0) {
			fprintf(stderr, "Can not start sending. Microcontroller not responding.\n");
			return ERR_TIMEOUT;
		}
		else
			return retval;
	}

	blockcount = len / XM_SIZE;
	if (len & XM_MASK)
		blockcount++;
	block = 1;
	VERBOSE(0, "Receiving data from memory using ");
	VERBOSE(0, "%s", ucprot(-1) ? "XModem" : "RAW");
	VERBOSE(0, " protocol with block size %d bytes.\n", XM_SIZE);
	VERBOSE(0, "Bytes: %d, Blocks: %d\n", len,  blockcount);
	count = len;
	while (count > 0) {
		int remainder = count > XM_SIZE ? XM_SIZE : count;
		VERBOSE(0, "\rReceiving Block %d / %d ", block, blockcount);
		pos = 0;
		VERBOSE(3, "\nremainder = %d\n", remainder);
		while (pos < (ucprot(-1) == PROT_XM ? XM_SIZE + 5 : remainder)) {
			retval = comm_read(&ibuf[pos], (ucprot(-1) == PROT_XM ? XM_SIZE + 5 : remainder) - pos);
			if (retval < 0)
				return retval;
			if (retval > 0) {
				pos += retval;
				VERBOSE(3, "pos = %d\n", pos);
			}
		}
		if (4 <= verbose) {
			int ii;
			for (ii = 0; ii < (ucprot(-1) == PROT_XM ? XM_SIZE + 5 : remainder); ii++)
				VERBOSE(4, " 0x%02X", (unsigned char)ibuf[ii]);
			VERBOSE(4, "\n");
		}
		VERBOSE(3, "Copy to user buffer from %d count %d\n", ucprot(-1) == PROT_XM ? 3 : 0, remainder);
		memcpy(buf, &ibuf[ucprot(-1) == PROT_XM ? 3 : 0], remainder);

		block++;
		count -= XM_SIZE;
		buf += XM_SIZE;

		if (ucprot(-1) == PROT_XM) {
			VERBOSE(3, "Sending ACK\n");
			obuf[0] = ACK;
			retval = comm_write(obuf, 1);
			if (retval < 0) {
				fprintf(stderr, "Can not acknowledge\n");
				return retval;
			}
		}
	}

	retval = ucwaitresp('>');
	if (retval != 0) {
		if (retval > 0) {
			fprintf(stderr, "Can not finish receiving. Microcontroller not responding.\n");
			return ERR_TIMEOUT;
		}
		else
			return retval;
	}

	VERBOSE(0, "Finished\n");

	return retval;
}

int ucflash(unsigned long addr, char *buf, size_t len)
{
	int count;
	int page;
	int pages;
	int retval;
	unsigned char ibuf[32];
	unsigned char obuf[256];
	unsigned long fmr;
	unsigned long fcr;
	unsigned long fsr;
	unsigned long pagesize = 256;
	unsigned long cidr;
	unsigned long sr;
	int tmpverbose = verbose;

	if (addr & 0x3)
		return 0;

	if ((ucprot(-1) == PROT_XM) && (!ucconnect())) {
		fprintf(stderr, "Can not connect!\n");
		return 0;
	}

	VERBOSE(0, "Detecting microcontroller: ");

	retval = ucread(CIDR, &cidr);
	if (retval < 0)
		return retval;
	VERBOSE(0, "0x%08X\n", cidr);

	ucshowproc(cidr, "");
	if (ucnvpsiz(cidr) < 0) {
		fprintf(stderr, "Microcontroller have no embedded Flash!\n");
		return 0;
	}
	if (!ucarch(cidr)) {
		fprintf(stderr, "Unsupported architecture!\n");
		return 0;
	}
	VERBOSE(0, "\n%s%d detected.\n\n", ucarch(cidr), ucnvpsiz(cidr));
	pagesize = ucnvpsiz(cidr) >= 128 ? 256 : 128;

	VERBOSE(0, "Loading sambaflash\n");
	retval = ucsend(SAMBAFLASH, (char *)sambaflash, sambaflash_size);
	if (retval < 0)
		return retval;

	VERBOSE(0, "\n");

	pages = len / pagesize;
	if (len % pagesize)
		pages++;
	VERBOSE(0, "Programming Flash with page size %d\n", pagesize);
	VERBOSE(0, "Bytes: %d, Pages: %d\n", len, pages);
	page = 1;
	count = len;
	while (count > 0) {
		VERBOSE(0, "\rProgramming page %d / %d ", page, pages);
		memcpy(obuf, buf, pagesize < count ? pagesize : count);
		verbose = -2;
		retval = ucsend(PAGEBUF, obuf, pagesize);
		verbose = tmpverbose;
		if (retval < 0)
			return retval;
		retval = ucwrite(PAGEADDR, addr);
		if (retval < 0)
			return retval;
		retval = ucwrite(PAGESIZE, pagesize);
		if (retval < 0)
			return retval;
		retval = ucgo(SAMBAFLASH);
		if (retval < 0)
			return retval;
		usleep(100000);
		if ((ucprot(-1) == PROT_XM) && (!ucconnect())) {
			fprintf(stderr, "Can not reconnect!\n");
			return 0;
		}
		retval = ucread(RETVAL, &sr);
		if (retval < 0)
			return retval;
		VERBOSE(1, "sambaflash retval = 0x%08X\n", sr);
		retval = (signed int)sr;
		if (retval) {
			fprintf(stderr, "\nError while programming page!\nReturn value: %d\n", retval);
			return 0;
		}
		count -= pagesize;
		buf += pagesize;
		addr += pagesize;
		page++;
	}

	VERBOSE(0, "\nFinished\n");

	return 0;
}

void ucshowproc(unsigned long cidr, char *prefix)
{
	printf("%sVersion:\t%d\n", prefix, CIDR_VERSION(cidr));

	printf("%sProcessor:\t", prefix);
	switch (CIDR_EPROC(cidr)) {
		case 1:
			printf("ARM946E-S");
			break;
		case 2:
			printf("ARM7TDMI");
			break;
		case 4:
			printf("ARM920T");
			break;
		case 5:
			printf("ARM926EJ-S");
			break;
		default:
			printf("Unknown");
			break;
	}
	printf("\n");

	printf("%sNVP Type:\t", prefix);
	switch (CIDR_NVPTYP(cidr)) {
		case 0:
			printf("ROM");
			break;
		case 1:
			printf("ROMless or on-chip Flash");
			break;
		case 4:
			printf("SRAM emulating ROM");
			break;
		case 2:
			printf("Embedded Flash");
			break;
		case 3:
			printf("ROM or Embedded Flash");
			break;
		default:
			printf("Unrecognized");
			break;
	}
	printf("\n");

	printf("%sNVP Size:\t", prefix);
	switch(CIDR_NVPSIZ(cidr)) {
		case 0:
			printf("None");
			break;
		case 1:
			printf("8K");
			break;
		case 2:
			printf("16K");
			break;
		case 3:
			printf("32K");
			break;
		case 5:
			printf("64K");
			break;
		case 7:
			printf("128K");
			break;
		case 9:
			printf("256K");
			break;
		case 10:
			printf("512K");
			break;
		case 12:
			printf("1024K");
			break;
		case 14:
			printf("2048K");
			break;
		default:
			printf("Unrecognized");
			break;
	}
	printf("\n");

	printf("%sNVP2 Size:\t", prefix);
	switch (CIDR_NVPSIZ2(cidr)) {
		case 0:
			printf("None");
			break;
		case 1:
			printf("8K");
			break;
		case 2:
			printf("16K");
			break;
		case 3:
			printf("32K");
			break;
		case 5:
			printf("64K");
			break;
		case 7:
			printf("128K");
			break;
		case 9:
			printf("256K");
			break;
		case 10:
			printf("512K");
			break;
		case 12:
			printf("1024K");
			break;
		case 14:
			printf("2048K");
			break;
		default:
			printf("Unrecognized");
			break;
	}
	printf("\n");

	printf("%sSRAM Size:\t", prefix);
	switch (CIDR_SRAMSIZ(cidr)) {
		case 1:
			printf("1K");
			break;
		case 2:
			printf("2K");
			break;
		case 4:
			printf("112K");
			break;
		case 5:
			printf("4K");
			break;
		case 6:
			printf("80K");
			break;
		case 7:
			printf("160K");
			break;
		case 8:
			printf("8K");
			break;
		case 9:
			printf("16K");
			break;
		case 10:
			printf("32K");
			break;
		case 11:
			printf("64K");
			break;
		case 12:
			printf("128K");
			break;
		case 13:
			printf("256K");
			break;
		case 14:
			printf("96K");
			break;
		case 15:
			printf("512K");
			break;
		default:
			printf("Unrecognized");
			break;
	}
	printf("\n");

	printf("%sArchitecture:\t", prefix);
	switch (CIDR_ARCH(cidr)) {
		case 0xF0:
			printf("AT75Cxx");
			break;
		case 0x40:
			printf("AT91x40");
			break;
		case 0x63:
			printf("AT91x63");
			break;
		case 0x55:
			printf("AT91x55");
			break;
		case 0x42:
			printf("AT91x42");
			break;
		case 0x92:
			printf("AT91x92");
			break;
		case 0x34:
			printf("AT91x34");
			break;
		case 0x60:
			printf("AT91SAM7Axx");
			break;
		case 0x70:
			printf("AT91SAM7Sxx");
			break;
		case 0x71:
			printf("AT91SAM7XC");
			break;
		case 0x72:
			printf("AT91SAM7SExx");
			break;
		case 0x73:
			printf("AT91SAM7Lxx");
			break;
		case 0x75:
			printf("AT91SAM7Xxx");
			break;
		case 0x19:
			printf("AT91SAM9xx");
			break;
		default:
			printf("Unrecognized");
			break;
	}
	printf("\n");

	printf("%sExtended CIDR\t%s\n", prefix, CIDR_EXT(cidr) ? "yes" : "no");
}

int ucnvpsiz(unsigned long cidr)
{
	switch(CIDR_NVPSIZ(cidr)) {
		case 1:
			return 8;
			break;
		case 2:
			return 16;
			break;
		case 3:
			return 32;
			break;
		case 5:
			return 64;
			break;
		case 7:
			return 128;
			break;
		case 9:
			return 256;
			break;
		case 10:
			return 512;
			break;
		case 12:
			return 1024;
			break;
		case 14:
			return 2048;
			break;
	}
	return -1;
}

char *ucarch(unsigned long cidr)
{
	switch (CIDR_ARCH(cidr)) {
		case 0x60:
			return "AT91SAM7A";
			break;
		case 0x70:
			return "AT91SAM7S";
			break;
		case 0x71:
			return "AT91SAM7XC";
			break;
		case 0x72:
			return "AT91SAM7SE";
			break;
		case 0x73:
			return "AT91SAM7L";
			break;
		case 0x75:
			return "AT91SAM7X";
			break;
	}

	return NULL;
}

int ucgo(unsigned long addr)
{
	int retval;
	char obuf[32];
	char ibuf[32];

	if (addr & 0x3)
		return 0;

	if ((ucprot(-1) == PROT_XM) && (!ucconnect())) {
		fprintf(stderr, "Can not connect!\n");
		return ERR_CONNECT;
	}

	sprintf(obuf, "G%08lX#", addr);

	retval = comm_write(obuf, strlen(obuf));

	return retval;
}

int ucflashinit()
{
	int retval;
	int i;
	unsigned long fsr;

	VERBOSE(3, "Initializing Flash\n");

	if ((ucprot(-1) == PROT_XM) && (!ucconnect())) {
		fprintf(stderr, "Can not connect!\n");
		return ERR_CONNECT;
	}

	i = 0;
	while (i++ < 16) {
		retval = ucread(FSR, &fsr);
		if (retval < 0)
			return retval;
		if (fsr & MC_FSR_FRDY)
			break;
	}
	if (i >= 16) {
		fprintf(stderr, "EFC is not ready (FRDY = 0) before initialization\n");
		return ERR_READY;
	}

	VERBOSE(3, "Setup EFC...\n");
	retval = ucwrite(FMR, (MC_FMR_FMCN_MASK & (100 << MC_FMR_FMCN_SHIFT)) | (1 < MC_FMR_FWS_SHIFT));
	if (retval < 0)
		return retval;

	VERBOSE(3, "Wait for EFC initialization...\n");
	i = 0;
	while (i++ < 16) {
		retval = ucread(FSR, &fsr);
		if (retval < 0)
			return retval;
		if (fsr & MC_FSR_FRDY)
			break;
	}
	if (i >= 16) {
		fprintf(stderr, "EFC is not ready (FRDY = 0) after initialization\n");
		return ERR_READY;
	}
	return 0;
}

int ucsetlocks(unsigned long value)
{
	int retval;
	int i;
	unsigned long fsr;
	unsigned long fcr;
	unsigned long cidr;
	unsigned long locks;
	unsigned long pagesize;
	unsigned long data;
	unsigned long page;
	unsigned int mask;

	if ((ucprot(-1) == PROT_XM) && (!ucconnect())) {
		fprintf(stderr, "Can not connect!\n");
		return ERR_CONNECT;
	}

	VERBOSE(3, "Reading CIDR...\n");
	retval = ucread(CIDR, &cidr);
	if (retval)
		return retval;

	VERBOSE(3, "Detecting architecture...\n");
	if (!ucarch(cidr)) {
		fprintf(stderr, "Unrecognized architecture!\n");
		return ERR_UNRECOGNIZED;
	}

	VERBOSE(3, "Checking if microcontroller has Flash...\n");
	if (ucnvpsiz(cidr) <= 0) {
		fprintf(stderr, "Microcontroller seems to be FlashLess!\n");
		return ERR_UNRECOGNIZED;
	}
	pagesize = ucnvpsiz(cidr) >= 128 ? 256 : 128;

	switch (ucnvpsiz(cidr)) {
		case 256:
		case 64:
			locks = 16;
			break;
		case 128:
		case 32:
			locks = 8;
			break;
		default:
			fprintf(stderr, "WARNING!!!\nCan not determine LOCKS bits count! Guess to 16 which may be wrong!\n");
			locks = 16;
			break;
	}

	if (value > ((1 << locks) - 1)) {
		fprintf(stderr, "LOCKS value out of range.\n");
		return ERR_INCOMPLETE;
	}

	VERBOSE(0, "%s%d detected with %d LOCKS bits, page size %d bytes and %d pages per lock region.\n", ucarch(cidr), ucnvpsiz(cidr), locks, pagesize, pagesize >> 2);

	retval = ucflashinit();
	if (retval)
		return retval;

	value <<= MC_FSR_LOCK_SHIFT;
	for (i = 0; i < locks; i++) {
		retval = ucread(FSR, &fsr);
		if (retval)
			return retval;

		mask = (1 << (i + MC_FSR_LOCK_SHIFT));
		if ((value & mask) != (fsr & mask))
		{
			VERBOSE(0, "%s \tLOCKS%d\n", (value & mask) ? "Locking" : "Unlocking", i);
			page = i * (pagesize >> 2);
			fcr = (0x5A << MC_FCR_KEY_SHIFT) | (page << MC_FCR_PAGEN_SHIFT) | ((value & mask) ? MC_FCR_FCMD_SLB : MC_FCR_FCMD_CLB);
			retval = ucwrite(FCR, fcr);
			if (retval)
				return retval;
			usleep(100000);
			retval = ucread(FSR, &data);
			if (retval)
				return retval;
			if (data & MC_FSR_PROGE) {
				fprintf(stderr, "Programming error for LOCKS%d\n", i);
				return ERR_INCOMPLETE;
			}
		}
	}

	return 0;
}

int ucsetnvm(unsigned long value)
{
	int retval;
	unsigned long data;

	VERBOSE(3, "Setting GPNVM bits\n");
	if (value > 7) {
		fprintf(stderr, "GPNVM value out of range.\n");
		return ERR_INCOMPLETE;
	}

	if ((ucprot(-1) == PROT_XM) && (!ucconnect())) {
		fprintf(stderr, "Can not connect!\n");
		return ERR_CONNECT;
	}

	value <<= MC_FSR_GPNVM_SHIFT;

	retval = ucflashinit();
	if (retval)
		return retval;

	retval = ucread(FSR, &data);
	if (retval)
		return retval;

	if ((value & MC_FSR_GPNVM0) != (data & MC_FSR_GPNVM0)) {
		VERBOSE(0, "Set \tGPVNM0 = %d\n", value & MC_FSR_GPNVM0 ? 1 : 0);
		retval = ucwrite(FCR, (0x5A << MC_FCR_KEY_SHIFT) | (0 << MC_FCR_PAGEN_SHIFT) | ((value & MC_FSR_GPNVM0) ? MC_FCR_FCMD_SGPB : MC_FCR_FCMD_CGPB) );
		if (retval)
			return retval;
		usleep(100000);
		retval = ucread(FSR, &data);
		if (retval)
			return retval;
		if (data & MC_FSR_PROGE) {
			fprintf(stderr, "Programming error\n");
			return ERR_READY;
		}
		if (data & MC_FSR_LOCKE) {
			fprintf(stderr, "Lock error\n");
			return ERR_READY;
		}
	}

	if ((value & MC_FSR_GPNVM1) != (data & MC_FSR_GPNVM1)) {
		VERBOSE(0, "Set \tGPVNM1 = %d\n", value & MC_FSR_GPNVM1 ? 1 : 0);
		retval = ucwrite(FCR, (0x5A << MC_FCR_KEY_SHIFT) | (1 << MC_FCR_PAGEN_SHIFT) | ((value & MC_FSR_GPNVM1) ? MC_FCR_FCMD_SGPB : MC_FCR_FCMD_CGPB) );
		if (retval)
			return retval;
		usleep(100000);
		retval = ucread(FSR, &data);
		if (retval)
			return retval;
		if (data & MC_FSR_PROGE) {
			fprintf(stderr, "Programming error\n");
			return ERR_READY;
		}
		if (data & MC_FSR_LOCKE) {
			fprintf(stderr, "Lock error\n");
			return ERR_READY;
		}
	}

	if ((value & MC_FSR_GPNVM2) != (data & MC_FSR_GPNVM2)) {
		VERBOSE(0, "Set \tGPVNM2 = %d\n", value & MC_FSR_GPNVM2 ? 1 : 0);
		retval = ucwrite(FCR, (0x5A << MC_FCR_KEY_SHIFT) | (2 << MC_FCR_PAGEN_SHIFT) | ((value & MC_FSR_GPNVM2) ? MC_FCR_FCMD_SGPB : MC_FCR_FCMD_CGPB) );
		if (retval)
			return retval;
		usleep(100000);
		retval = ucread(FSR, &data);
		if (retval)
			return retval;
		if (data & MC_FSR_PROGE) {
			fprintf(stderr, "Programming error\n");
			return ERR_READY;
		}
		if (data & MC_FSR_LOCKE) {
			fprintf(stderr, "Lock error\n");
			return ERR_READY;
		}
	}

	return 0;
}

/* End of file */
