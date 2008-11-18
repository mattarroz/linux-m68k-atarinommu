/*
 *  Fast C2P (Chunky-to-Planar) Conversion
 *
 *  Copyright (C) 2003-2008 Geert Uytterhoeven
 *
 *  NOTES:
 *    - This code was inspired by Scout's C2P tutorial
 *    - It assumes to run on a big endian system
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of this archive
 *  for more details.
 */

#include <linux/module.h>
#include <linux/string.h>

#include "c2p.h"


    /*
     *  Basic transpose step
     */

#define _transp(d, i1, i2, shift, mask)			\
    do {						\
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

#define transp8_nx1(d, n)				\
    do {						\
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

#define transp8_nx2(d, n)				\
    do {						\
	u32 mask = get_mask(n);				\
	/* First block */				\
	_transp(d, 0, 2, n, mask);			\
	_transp(d, 1, 3, n, mask);			\
	/* Second block */				\
	_transp(d, 4, 6, n, mask);			\
	_transp(d, 5, 7, n, mask);			\
    } while (0)

#define transp8_nx4(d, n)				\
    do {						\
	u32 mask = get_mask(n);				\
	/* Single block */				\
	_transp(d, 0, 4, n, mask);			\
	_transp(d, 1, 5, n, mask);			\
	_transp(d, 2, 6, n, mask);			\
	_transp(d, 3, 7, n, mask);			\
    } while (0)

#define transp8(d, n, m)	transp8_nx ## m(d, n)


#define transp4_nx1(d, n)				\
    do {						\
	u32 mask = get_mask(n);				\
	/* First block */				\
	_transp(d, 0, 1, n, mask);			\
	/* Second block */				\
	_transp(d, 2, 3, n, mask);			\
    } while (0)

#define transp4_nx2(d, n)				\
    do {						\
	u32 mask = get_mask(n);				\
	/* Single block */				\
	_transp(d, 0, 2, n, mask);			\
	_transp(d, 1, 3, n, mask);			\
    } while (0)

#define transp4(d, n, m)	transp4_nx ## m(d, n)


#define transp4x_nx2(d, n)				\
    do {						\
	u32 mask = get_mask(n);				\
	/* Single block */				\
	_transp(d, 2, 0, n, mask);			\
	_transp(d, 3, 1, n, mask);			\
    } while (0)

#define transp4x(d, n, m)	transp4x_nx ## m(d, n)


    /*
     *  Perform a full C2P step on 32 8-bit pixels, stored in 8 32-bit words
     *  containing
     *    - 32 8-bit chunky pixels on input
     *    - permutated planar data (1 plane per 32-bit word) on output
     */

static void c2p_32x8(u32 d[8])
{
    transp8(d, 16, 4);
    transp8(d, 8, 2);
    transp8(d, 4, 1);
    transp8(d, 2, 4);
    transp8(d, 1, 2);
}


    /*
     *  Perform a full C2P step on 16 8-bit pixels, stored in 4 32-bit words
     *  containing
     *    - 16 8-bit chunky pixels on input
     *    - permutated planar data (2 planes per 32-bit word) on output
     */

static void c2p_16x8(u32 d[4])
{
    transp4(d, 8, 2);
    transp4(d, 1, 2);
    transp4x(d, 16, 2);
    transp4x(d, 2, 2);
    transp4(d, 4, 1);
}


    /*
     *  Array containing the permutation indices of the planar data after c2p
     */

static const int perm_c2p_32x8[8] = { 7, 5, 3, 1, 6, 4, 2, 0 };
static const int perm_c2p_16x8[4] = { 1, 3, 0, 2 };


    /*
     *  Compose two values, using a bitmask as decision value
     *  This is equivalent to (a & mask) | (b & ~mask)
     */

static inline unsigned long comp(unsigned long a, unsigned long b,
				 unsigned long mask)
{
	return ((a ^ b) & mask) ^ b;
}


    /*
     *  Store a full block of planar data after c2p conversion
     */

static inline void store_planar(void *dst, u32 dst_inc, u32 bpp, u32 d[8])
{
    int i;

    for (i = 0; i < bpp; i++, dst += dst_inc)
	*(u32 *)dst = d[perm_c2p_32x8[i]];
}


    /*
     *  Store a partial block of planar data after c2p conversion
     */

static inline void store_planar_masked(void *dst, u32 dst_inc, u32 bpp,
				       u32 d[8], u32 mask)
{
    int i;

    for (i = 0; i < bpp; i++, dst += dst_inc)
	*(u32 *)dst = comp(d[perm_c2p_32x8[i]], *(u32 *)dst, mask);
}


    /*
     *  Store a full block of iplan2 data after c2p conversion
     */

static inline void store_iplan2(void *dst, u32 bpp, u32 d[4])
{
    int i;

    for (i = 0; i < bpp/2; i++, dst += 4)
	*(u32 *)dst = d[perm_c2p_16x8[i]];
}


    /*
     *  Store a partial block of iplan2 data after c2p conversion
     */

static inline void store_iplan2_masked(void *dst, u32 bpp, u32 d[4], u32 mask)
{
    int i;

    for (i = 0; i < bpp/2; i++, dst += 4)
	*(u32 *)dst = comp(d[perm_c2p_16x8[i]], *(u32 *)dst, mask);
}


    /*
     *  c2p_planar - Copy 8-bit chunky image data to a planar frame buffer
     *  @dst: Starting address of the planar frame buffer
     *  @dx: Horizontal destination offset (in pixels)
     *  @dy: Vertical destination offset (in pixels)
     *  @width: Image width (in pixels)
     *  @height: Image height (in pixels)
     *  @dst_nextline: Frame buffer offset to the next line (in bytes)
     *  @dst_nextplane: Frame buffer offset to the next plane (in bytes)
     *  @src_nextline: Image offset to the next line (in bytes)
     *  @bpp: Bits per pixel of the planar frame buffer (1-8)
     */

void c2p_planar(void *dst, const void *src, u32 dx, u32 dy, u32 width,
		u32 height, u32 dst_nextline, u32 dst_nextplane,
		u32 src_nextline, u32 bpp)
{
    union {
	u8 pixels[32];
	u32 words[8];
    } d;
    u32 dst_idx, first, last, w;
    const u8 *c;
    void *p;

    dst += dy*dst_nextline+(dx & ~31);
    dst_idx = dx % 32;
    first = 0xffffffffU >> dst_idx;
    last = ~(0xffffffffU >> ((dst_idx+width) % 32));
    while (height--) {
	c = src;
	p = dst;
	w = width;
	if (dst_idx+width <= 32) {
	    /* Single destination word */
	    first &= last;
	    memset(d.pixels, 0, sizeof(d));
	    memcpy(d.pixels+dst_idx, c, width);
	    c += width;
	    c2p_32x8(d.words);
	    store_planar_masked(p, dst_nextplane, bpp, d.words, first);
	    p += 4;
	} else {
	    /* Multiple destination words */
	    w = width;
	    /* Leading bits */
	    if (dst_idx) {
		w = 32 - dst_idx;
		memset(d.pixels, 0, dst_idx);
		memcpy(d.pixels+dst_idx, c, w);
		c += w;
		c2p_32x8(d.words);
		store_planar_masked(p, dst_nextplane, bpp, d.words, first);
		p += 4;
		w = width-w;
	    }
	    /* Main chunk */
	    while (w >= 32) {
		memcpy(d.pixels, c, 32);
		c += 32;
		c2p_32x8(d.words);
		store_planar(p, dst_nextplane, bpp, d.words);
		p += 4;
		w -= 32;
	    }
	    /* Trailing bits */
	    w %= 32;
	    if (w > 0) {
		memcpy(d.pixels, c, w);
		memset(d.pixels+w, 0, 32-w);
		c2p_32x8(d.words);
		store_planar_masked(p, dst_nextplane, bpp, d.words, last);
	    }
	}
	src += src_nextline;
	dst += dst_nextline;
    }
}
EXPORT_SYMBOL_GPL(c2p_planar);


    /*
     *  c2p_iplan2 - Copy 8-bit chunky image data to an interleaved planar
     *  frame buffer with 2 bytes of interleave
     *  @dst: Starting address of the planar frame buffer
     *  @dx: Horizontal destination offset (in pixels)
     *  @dy: Vertical destination offset (in pixels)
     *  @width: Image width (in pixels)
     *  @height: Image height (in pixels)
     *  @dst_nextline: Frame buffer offset to the next line (in bytes)
     *  @src_nextline: Image offset to the next line (in bytes)
     *  @bpp: Bits per pixel of the planar frame buffer (2, 4, or 8)
     */

void c2p_iplan2(void *dst, const void *src, u32 dx, u32 dy, u32 width,
		u32 height, u32 dst_nextline, u32 src_nextline, u32 bpp)
{
    union {
	u8 pixels[16];
	u32 words[4];
    } d;
    u32 dst_idx, first, last, w;
    const u8 *c;
    void *p;

    dst += dy*dst_nextline+(dx & ~15)*bpp;
    dst_idx = dx % 16;
    first = 0xffffU >> dst_idx;
    first |= first << 16;
    last = 0xffffU ^ (0xffffU >> ((dst_idx+width) % 16));
    last |= last << 16;
    while (height--) {
	c = src;
	p = dst;
	w = width;
	if (dst_idx+width <= 16) {
	    /* Single destination word */
	    first &= last;
	    memset(d.pixels, 0, sizeof(d));
	    memcpy(d.pixels+dst_idx, c, width);
	    c += width;
	    c2p_16x8(d.words);
	    store_iplan2_masked(p, bpp, d.words, first);
	    p += bpp*2;
	} else {
	    /* Multiple destination words */
	    w = width;
	    /* Leading bits */
	    if (dst_idx) {
		w = 16 - dst_idx;
		memset(d.pixels, 0, dst_idx);
		memcpy(d.pixels+dst_idx, c, w);
		c += w;
		c2p_16x8(d.words);
		store_iplan2_masked(p, bpp, d.words, first);
		p += bpp*2;
		w = width-w;
	    }
	    /* Main chunk */
	    while (w >= 16) {
		memcpy(d.pixels, c, 16);
		c += 16;
		c2p_16x8(d.words);
		store_iplan2(p, bpp, d.words);
		p += bpp*2;
		w -= 16;
	    }
	    /* Trailing bits */
	    w %= 16;
	    if (w > 0) {
		memcpy(d.pixels, c, w);
		memset(d.pixels+w, 0, 16-w);
		c2p_16x8(d.words);
		store_iplan2_masked(p, bpp, d.words, last);
	    }
	}
	src += src_nextline;
	dst += dst_nextline;
    }
}
EXPORT_SYMBOL_GPL(c2p_iplan2);

MODULE_LICENSE("GPL");
