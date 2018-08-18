/*
3dxdisp - library for controlling the 3DConnexion SpacePilot display
Copyright (C) 2018 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, modify, and/or redistribute
it under the terms of the MIT/X11 license. See LICENSE for details.
If you intend to redistribute parts of the code without the LICENSE file
replace this paragraph with the full contents of the LICENSE file.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "3dxdisp.h"
#include "hidapi/hidapi.h"

#define SPILOT_VID	0x046d
#define SPILOT_PID	0xc625

#define REP_LCD_POS			0xc
#define REP_LCD_DATA		0xd
#define REP_LCD_DATA_PACKED	0xe


#define FBWIDTH		240
#define FBHEIGHT	64

#define FBSIZE		(FBWIDTH * FBHEIGHT)

static void update_row(int row, unsigned char *fbptr);
static void invalidate(int x, int y, int w, int h);
static int set_start_pos(int row, int col);
static int send_unpacked(const unsigned char *data);
static int send_packed(uint32_t pat, uint32_t counts);

static hid_device *hid;
static unsigned char fb[FBSIZE];

int tdx_open(void)
{
	tdx_close();

	if(!(hid = hid_open(SPILOT_VID, SPILOT_PID, 0))) {
		fprintf(stderr, "tdx_open: failed to open space pilot HID device\n");
		return -1;
	}
	hid_set_nonblocking(hid, 1);

	return 0;
}

void tdx_close(void)
{
	if(hid) {
		hid_close(hid);
	}
}


void tdx_clear(unsigned char c)
{
	memset(fb, c, FBSIZE);
	invalidate(0, 0, FBWIDTH, FBHEIGHT);
}

void tdx_rect(int x, int y, int w, int h, unsigned char c)
{
	int i;
	unsigned char *ptr = fb + y * FBWIDTH + x;

	for(i=0; i<h; i++) {
		memset(ptr, c, w);
		ptr += FBWIDTH;
	}

	invalidate(x, y, w, h);
}

void tdx_line(int x0, int y0, int x1, int y1, unsigned char c)
{
	/* TODO */
}


void tdx_pixel(int x, int y, unsigned char c)
{
	fb[y * FBWIDTH + x] = c;
}


void tdx_blit(int x, int y, int w, int h, unsigned char *pix)
{
	int i;
	unsigned char *ptr = fb + y * FBWIDTH + x;

	for(i=0; i<h; i++) {
		memcpy(ptr, pix, w);
		ptr += FBWIDTH;
		pix += w;
	}
}

void tdx_blit_packed(int x, int y, int w, int h, unsigned char *pix)
{
	int i, j, k;
	unsigned char *dest = fb + y * FBWIDTH + x;

	for(i=0; i<h; i++) {
		for(j=0; j<w/8; j++) {
			unsigned char p = *pix++;
			for(k=0; k<8; k++) {
				*dest++ = p & 1;
				p >>= 1;
			}
		}
		dest += FBWIDTH - w;
	}
}

void tdx_update(void)
{
	int i;
	unsigned char *fbptr = fb;

	for(i=0; i<8; i++) {
		update_row(i, fbptr);
		fbptr += FBWIDTH * 8;
	}

	/* TODO validate all */
}

static inline unsigned char column_pixels(unsigned char *fbptr)
{
	int i;
	unsigned char res = 0;

	for(i=0; i<8; i++) {
		res |= (*fbptr ? 1 : 0) << i;
		fbptr += FBWIDTH;
	}
	return res;
}

static void update_row(int row, unsigned char *fbptr)
{
	int start, i, j;
	unsigned char colpix[FBWIDTH];

	unsigned char pkpat[3], pkrep[3];
	int pkidx, pkadv;

	set_start_pos(row, 0);

	for(i=0; i<FBWIDTH; i++) {
		unsigned char c = 0;
		unsigned char *p = fbptr;
		for(j=0; j<8; j++) {
			c |= (*p ? 1 : 0) << j;
			p += FBWIDTH;
		}
		colpix[i] = c;
		fbptr++;
	}

	start = 0;
	while(start < FBWIDTH) {
		/* try to make a packed packet, see if it helps */
		pkidx = 0;
		pkpat[0] = colpix[start];
		pkpat[1] = pkpat[2] = 0;
		pkrep[0] = 1;
		pkrep[1] = pkrep[2] = 0;

		for(i=start+1; i<FBWIDTH; i++) {
			if(colpix[i] == pkpat[pkidx]) {
				pkrep[pkidx]++;
			} else {
				if(++pkidx >= 3) break;
				pkpat[pkidx] = colpix[i];
				pkrep[pkidx] = 1;
			}
		}
		pkadv = i - start;

		if(pkadv > 7 || FBWIDTH - i < 7) {
			/* send packed */
			uint32_t pat = pkpat[0] | ((uint32_t)pkpat[1] << 8) | ((uint32_t)pkpat[2] << 16);
			uint32_t cnt = pkrep[0] | ((uint32_t)pkrep[1] << 8) | ((uint32_t)pkrep[2] << 16);
			send_packed(pat, cnt);
			start += pkadv;
		} else {
			/* send unpacked */
			send_unpacked(colpix + start);
			start += 7;
		}
	}
}

static void invalidate(int x, int y, int w, int h)
{
}

static int set_start_pos(int row, int col)
{
	unsigned char buf[4];

	buf[0] = REP_LCD_POS;
	buf[1] = row;
	buf[2] = col;
	buf[3] = 0;

	if(hid_send_feature_report(hid, buf, 4) == -1) {
		fprintf(stderr, "set_start_pos failed\n");
		return -1;
	}
	return 0;
}

/* data must be 7 bytes corresponding to 7 8-pixel-tall columns */
static int send_unpacked(const unsigned char *data)
{
	unsigned char buf[8];
	int i;

	buf[0] = REP_LCD_DATA;
	for(i=1; i<8; i++) {
		buf[i] = *data++;
	}

	if(hid_send_feature_report(hid, buf, 8) == -1) {
		fprintf(stderr, "send_unpacked failed\n");
		return -1;
	}
	return 0;
}

/* pat [0,24] contains 3 packed bytes of patterns
 * counts [0,24] contains 3 packed repeat counts for each pattern
 */
static int send_packed(uint32_t pat, uint32_t counts)
{
	int i, nbytes;
	unsigned char buf[8];

	buf[0] = REP_LCD_DATA_PACKED;
	nbytes = 1;

	for(i=0; i<3; i++) {
		if(counts & 0xff) {
			buf[nbytes++] = counts & 0xff;
			buf[nbytes++] = pat & 0xff;
		}
		counts >>= 8;
		pat >>= 8;
	}

	if(hid_send_feature_report(hid, buf, nbytes) == -1) {
		fprintf(stderr, "send_packed failed\n");
		return -1;
	}
	return 0;
}
