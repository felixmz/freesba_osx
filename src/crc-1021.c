/*
 *	This source code is licensed under the GNU General Public License,
 *	Version 2. See the file COPYING for more details.
 */

#include <crc-1021.h>

/**
 * crc_1021	Compute the CRC for the data buffer with poly 1021 using bit-at-a-time methods
 * @param	crc - previous CRC value
 * @param	buffer - data pointer
 * @param	len - number of bytes in the buffer
 * @return	the updated CRC value
 */
unsigned short crc_1021(unsigned short crc, unsigned char const *buffer, size_t len)
{
	unsigned short cmpt;

	/* For  all char */
	while (len--) {
		crc = crc ^ *buffer++ << 8;
		/* For All bit */
		for (cmpt = 0; cmpt < 8; cmpt++) {
			if (crc & 0x8000)
				crc = crc << 1 ^ CRC16POLY;
			else
				crc = crc << 1;
		} /* end bit */
	} /* Frame end */

	return crc;
}
