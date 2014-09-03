#ifndef _CRC1021_H
#define _CRC1021_H

#include <sys/types.h>

#define CRC16POLY   0x1021              /* CRC 16  polynom */

unsigned short crc_1021(unsigned short crc, const unsigned char *buffer, size_t len);

#endif /* _CRC1021_H */
