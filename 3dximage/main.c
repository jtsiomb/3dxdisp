#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <imago2.h>
#include "3dxdisp.h"

struct rect {
	int x, y, w, h;
};

static int dump_pgm(const char *fname, int width, int height, unsigned char *pixels);
static int dump_pbm(const char *fname, int width, int height, unsigned char *pixels);
static int parse_args(int argc, char **argv);

static const char *fname;
static struct rect rect = {0, 0, 240, 64};
static int dither = IMG_DITHER_FLOYD_STEINBERG;
static int invert, dryrun, fullimg;
static float colscale = 1.0f;
static float gammaval = 1.0f;

int main(int argc, char **argv)
{
	struct img_pixmap img, srcimg;
	int i, j, col;
	int x0, x1, y0, y1;
	float tx, ty, c00, c01, c10, c11, c0, c1;
	unsigned char *src, *pptr;

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	if(!dryrun && tdx_open() == -1) {
		fprintf(stderr, "failed to open space pilot device\n");
		return 1;
	}

	img_init(&img);
	img_set_pixels(&img, 240, 64, IMG_FMT_GREY8, 0);

	img_init(&srcimg);
	if(img_load(&srcimg, fname) == -1) {
		fprintf(stderr, "failed to load image: %s\n", fname);
		return 1;
	}
	img_convert(&srcimg, IMG_FMT_GREY8);

	if(fullimg) {
		rect.x = rect.y = 0;
		rect.w = srcimg.width;
		rect.h = srcimg.height;
	} else {
		if(rect.x < 0 || rect.y < 0) {
			fprintf(stderr, "invalid offset: %d,%d\n", rect.x, rect.y);
			return 1;
		}
		if(rect.x + rect.w > srcimg.width || rect.y + rect.h > srcimg.height) {
			fprintf(stderr, "source rect: %d,%d %dx%d outside of the %dx%d image bounds\n",
					rect.x, rect.y, rect.w, rect.h, srcimg.width, srcimg.height);
			return 1;
		}
	}

	pptr = img.pixels;
	src = (unsigned char*)srcimg.pixels + rect.y * srcimg.width + rect.x;
	for(i=0; i<64; i++) {
		ty = fmod((float)i / 64.0f, 1.0f);
		y0 = (float)i / 64.0f * rect.h;
		y1 = (float)(i + 1) / 64.0f * rect.h;

		if(y1 >= srcimg.height) y1 = srcimg.height - 1;

		for(j=0; j<240; j++) {
			tx = fmod((float)j / 240.0f, 1.0f);
			x0 = (float)j / 240.0f * rect.w;
			x1 = (float)(j + 1) / 240.0f * rect.w;

			if(x1 >= srcimg.width) x1 = srcimg.width -1;

			c00 = src[y0 * srcimg.width + x0];
			c01 = src[y1 * srcimg.width + x0];
			c10 = src[y0 * srcimg.width + x1];
			c11 = src[y1 * srcimg.width + x1];

			c0 = c00 + (c01 - c00) * ty;
			c1 = c10 + (c11 - c10) * ty;
			col = (int)pow((c0 + (c1 - c0) * tx) * colscale, gammaval);
			*pptr++ = col < 0 ? 0 : (col > 255 ? 255 : col);
		}
	}

	dump_pgm("/tmp/3dximage.pgm", 240, 64, img.pixels);
	img_quantize(&img, 2, dither);

	if(invert) {
		pptr = img.pixels;
		for(i=0; i<240*64; i++) {
			pptr[i] = !pptr[i];
		}
	}

	dump_pbm("/tmp/3dximage.pbm", 240, 64, img.pixels);

	if(!dryrun) {
		tdx_blit(0, 0, 240, 64, img.pixels);
		tdx_update();
		tdx_close();
	}
	return 0;
}

static int dump_pgm(const char *fname, int width, int height, unsigned char *pixels)
{
	int i, npix = width * height;
	FILE *fp;

	if(!(fp = fopen(fname, "wb"))) {
		fprintf(stderr, "failed to dump: %s\n", fname);
		return -1;
	}

	fprintf(fp, "P5\n%d %d\n255\n", width, height);
	for(i=0; i<npix; i++) {
		fputc(*pixels++, fp);
	}
	fclose(fp);
	return 0;
}

static int dump_pbm(const char *fname, int width, int height, unsigned char *pixels)
{
	int i, j;
	FILE *fp;

	if(!(fp = fopen(fname, "w"))) {
		fprintf(stderr, "failed to dump: %s\n", fname);
		return -1;
	}

	fprintf(fp, "P1\n%d %d\n", width, height);
	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			fprintf(fp, "%d%c", *pixels++ ? 0 : 1, j < width - 1 ? ' ' : '\n');
		}
		fputc('\n', fp);
	}
	fclose(fp);
	return 0;
}

static int parse_args(int argc, char **argv)
{
	int i;

	static const char *usage_fmt = "Usage: %s [options] <image file>\n"
		"Options:\n"
		"  -level <factor>: adjust brigthness level before converting to b/w\n"
		"  -gamma <value>: adjust the gamma value before converting to b/w\n"
		"  -nodither: don't use dithering when converting image to b/w\n"
		"  -invert: invert colors after conversion\n"
		"  -offset <X,Y>: specify offset in source image\n"
		"  -geometry <WxH[+X+Y]>: specify source rectangle to display\n"
		"  -full: automatically set the geometry to fit the whole source image\n"
		"  -dryrun: don't upload to device, just output the final image\n"
		"  -h,-help: print usage information and exit\n";

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(strcmp(argv[i], "-level") == 0) {
				if(!argv[++i] || (colscale = atof(argv[i])) <= 0.0f) {
					fprintf(stderr, "-level must be followed by an adjustment factor\n");
					return -1;
				}

			} else if(strcmp(argv[i], "-gamma") == 0) {
				if(!argv[++i] || (gammaval = atof(argv[i])) <= 0.0f) {
					fprintf(stderr, "-gamma must be followed by a gamma value\n");
					return -1;
				}

			} else if(strcmp(argv[i], "-nodither") == 0) {
				dither = IMG_DITHER_NONE;

			} else if(strcmp(argv[i], "-invert") == 0) {
				invert = 1;

			} else if(strcmp(argv[i], "-offset") == 0) {
				if(!argv[++i] || sscanf(argv[i], "%d,%d", &rect.x, &rect.y) != 2) {
					fprintf(stderr, "-offset must be followed by X,Y\n");
					return -1;
				}

			} else if(strcmp(argv[i], "-geometry") == 0) {
				if(!argv[++i] || sscanf(argv[i], "%dx%d+%d+%d", &rect.w, &rect.h, &rect.x, &rect.y) < 2) {
					fprintf(stderr, "-geometry must be followed by WxH+X+Y\n");
					return -1;
				}

			} else if(strcmp(argv[i], "-full") == 0) {
				fullimg = 1;

			} else if(strcmp(argv[i], "-dryrun") == 0) {
				dryrun = 1;

			} else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
				printf(usage_fmt, argv[0]);
				exit(0);

			} else {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				fprintf(stderr, usage_fmt, argv[0]);
				return -1;
			}

		} else {
			if(fname) {
				fprintf(stderr, "unexpected argument: %s\n", argv[i]);
				fprintf(stderr, usage_fmt, argv[0]);
				return -1;
			}
			fname = argv[i];
		}
	}

	if(!fname) {
		fprintf(stderr, "pass an image filename to display on the space pilot\n");
		return -1;
	}
	return 0;
}
