/*
3dxdisp - library for controlling the 3DConnexion SpacePilot display
Copyright (C) 2018 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, modify, and/or redistribute
it under the terms of the MIT/X11 license. See LICENSE for details.
If you intend to redistribute parts of the code without the LICENSE file
replace this paragraph with the full contents of the LICENSE file.
*/
#ifndef LIB3DXDISP_H_
#define LIB3DXDISP_H_

#ifdef __cplusplus
extern "C" {
#endif

int tdx_open(void);
int tdx_open_file(const char *dev_path);
void tdx_close(void);

void tdx_clear(unsigned char c);
void tdx_rect(int x, int y, int w, int h, unsigned char c);
void tdx_line(int x0, int y0, int x1, int y1, unsigned char c);

void tdx_pixel(int x, int y, unsigned char c);

void tdx_blit(int x, int y, int w, int h, unsigned char *pix);
void tdx_blit_packed(int x, int y, int w, int h, unsigned char *pix);

void tdx_update(void);

#ifdef __cplusplus
}
#endif

#endif	/* LIB3DXDISP_H_ */
