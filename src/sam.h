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

#ifndef _SAM_H
#define _SAM_H

#include <sys/types.h>

#include "defs.h"

#define DEFAULT_TIMEOUT		500000	/* 0.5 sec */

#define PROT_RAW		0
#define PROT_XM			1

#define XM_SIZE			128
#define XM_MASK			0x7F
#define SOH			1
#define EOT			4
#define ACK			6

#define ERR_TIMEOUT		(ERROR_UC + 0x00)
#define ERR_ALIGN		(ERROR_UC + 0x01)
#define ERR_INCOMPLETE		(ERROR_UC + 0x02)
#define ERR_UNRECOGNIZED	(ERROR_UC + 0x03)
#define ERR_CONNECT		(ERROR_UC + 0x04)
#define ERR_READY		(ERROR_UC + 0x05)

#define FMR			0xFFFFFF60
#define FCR			0xFFFFFF64
#define FSR			0xFFFFFF68
#define CIDR			0xFFFFF240

#define CIDR_VERSION(x)		(x & 0x1F)
#define CIDR_EPROC(x)		((x >> 5) & 0x7)
#define CIDR_NVPSIZ(x)		((x >> 8) & 0xF)
#define CIDR_NVPSIZ2(x)		((x >> 12) & 0xF)
#define CIDR_SRAMSIZ(x)		((x >> 16) & 0xF)
#define CIDR_ARCH(x)		((x >> 20) & 0xFF)
#define CIDR_NVPTYP(x)		((x >> 28) & 0x7)
#define CIDR_EXT(x)		((x >> 31) & 0x1)

#define MC_FMR_FRDY		0x00000001
#define MC_FMR_LOCKE		0x00000004
#define MC_FMR_PROGE		0x00000008
#define MC_FMR_NEBP		0x00000080
#define MC_FMR_FWS_MASK		0x00000300
#define MC_FMR_FWS_SHIFT	 8
#define MC_FMR_FMCN_MASK	 0x00FF0000
#define MC_FMR_FMCN_SHIFT	16

#define MC_FCR_FCMD_NOP		0x00000000
#define MC_FCR_FCMD_WP		0x00000001
#define MC_FCR_FCMD_SLB		0x00000002
#define MC_FCR_FCMD_WPL		0x00000003
#define MC_FCR_FCMD_CLB		0x00000004
#define MC_FCR_FCMD_EA		0x00000008
#define MC_FCR_FCMD_SGPB	0x0000000B
#define MC_FCR_FCMD_CGPB	0x0000000D
#define MC_FCR_FCMD_SSB		0x0000000F
#define MC_FCR_PAGEN_MASK	0x0003FF00
#define MC_FCR_PAGEN_SHIFT	8
#define MC_FCR_KEY_MASK		0xFF000000
#define MC_FCR_KEY_SHIFT	24

#define MC_FSR_FRDY		0x00000001
#define MC_FSR_LOCKE		0x00000004
#define MC_FSR_PROGE		0x00000008
#define MC_FSR_SECURITY		0x00000010
#define MC_FSR_GPNVM_SHIFT	8
#define MC_FSR_GPNVM0		0x00000100
#define MC_FSR_GPNVM1		0x00000200
#define MC_FSR_GPNVM2		0x00000400
#define MC_FSR_LOCK_MASK	0xFFFF0000
#define MC_FSR_LOCK_SHIFT	16
#define MC_FSR_LOCK0		0x00010000
#define MC_FSR_LOCK1		0x00020000
#define MC_FSR_LOCK2		0x00040000
#define MC_FSR_LOCK3		0x00080000
#define MC_FSR_LOCK4		0x00100000
#define MC_FSR_LOCK5		0x00200000
#define MC_FSR_LOCK6		0x00400000
#define MC_FSR_LOCK7		0x00800000
#define MC_FSR_LOCK8		0x01000000
#define MC_FSR_LOCK9		0x02000000
#define MC_FSR_LOCK10		0x04000000
#define MC_FSR_LOCK11		0x08000000
#define MC_FSR_LOCK12		0x10000000
#define MC_FSR_LOCK13		0x20000000
#define MC_FSR_LOCK14		0x40000000
#define MC_FSR_LOCK15		0x80000000

#define SAMBAFLASH		0x00202000
#define PAGEADDR		0x00204000
#define PAGESIZE		0x00204004
#define RETVAL			0x00204008
#define PAGEBUF			0x0020400C

int ucprot(int p);

int ucread(unsigned long addr, unsigned long *data);
int ucwrite(unsigned long addr, unsigned long data);
int ucwaitresp(char resp);
int ucconnect();
int ucsend(unsigned long addr, char *buf, size_t len);
int ucrecv(unsigned long addr, char *buf, size_t len);
int ucflash(unsigned long addr, char *buf, size_t len);
int ucgo(unsigned long addr);
void ucshowproc(unsigned long cidr, char *prefix);
int ucnvpsiz(unsigned long cidr);
char *ucarch(unsigned long cidr);
int ucsetlocks(unsigned long value);
int ucsetnvm(unsigned long value);

void uc_info();

#endif /* _SAM_H */
