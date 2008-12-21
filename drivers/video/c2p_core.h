/*
 *  Fast C2P (Chunky-to-Planar) Conversion
 *
 *  Copyright (C) 2003-2008 Geert Uytterhoeven
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of this archive
 *  for more details.
 */


    /*
     *  Basic transpose step
     */

#define _transp(d, i1, i2, shift, mask)				\
	do {							\
		u32 t = (d[i1] ^ (d[i2] >> shift)) & mask;	\
		d[i1] ^= t;					\
		d[i2] ^= t << shift;				\
	} while (0)


static inline u32 get_mask(int n)
{
	switch (n) {
	case 1:
		return 0x55555555;
		break;

	case 2:
		return 0x33333333;
		break;

	case 4:
		return 0x0f0f0f0f;
		break;

	case 8:
		return 0x00ff00ff;
		break;

	case 16:
		return 0x0000ffff;
		break;
	}
	return 0;
}


    /*
     *  Transpose operations on 8 32-bit words
     */

#define transp8_nx1(d, n)					\
	do {							\
		u32 mask = get_mask(n);				\
		/* First block */				\
		_transp(d, 0, 1, n, mask);			\
		/* Second block */				\
		_transp(d, 2, 3, n, mask);			\
		/* Third block */				\
		_transp(d, 4, 5, n, mask);			\
		/* Fourth block */				\
		_transp(d, 6, 7, n, mask);			\
	} while (0)

#define transp8_nx2(d, n)					\
	do {							\
		u32 mask = get_mask(n);				\
		/* First block */				\
		_transp(d, 0, 2, n, mask);			\
		_transp(d, 1, 3, n, mask);			\
		/* Second block */				\
		_transp(d, 4, 6, n, mask);			\
		_transp(d, 5, 7, n, mask);			\
	} while (0)

#define transp8_nx4(d, n)					\
	do {							\
		u32 mask = get_mask(n);				\
		/* Single block */				\
		_transp(d, 0, 4, n, mask);			\
		_transp(d, 1, 5, n, mask);			\
		_transp(d, 2, 6, n, mask);			\
		_transp(d, 3, 7, n, mask);			\
	} while (0)

#define transp8(d, n, m)	transp8_nx ## m(d, n)


    /*
     *  Compose two values, using a bitmask as decision value
     *  This is equivalent to (a & mask) | (b & ~mask)
     */

static inline unsigned long comp(unsigned long a, unsigned long b,
				 unsigned long mask)
{
	return ((a ^ b) & mask) ^ b;
}
