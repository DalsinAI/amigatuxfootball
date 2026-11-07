/* IMG_Load for Tux Football on AmigaOS (see include/SDL/SDL_image.h), from
 * the Team's VoR, Ceferino and Penguin Command ports: there is no SDL_image 1.2 for
 * OpenGPU's SDL 1.2. PNG on libpng, as SDL_image makes them: palette PNGs
 * become 8-bit surfaces with the PNG's palette, others 32-bit (RGBA when
 * they have alpha), and a tRNS colour is the surface's colour key; JPEG on
 * libjpeg too; Truevision TGA (raw or RLE, 24 or 32 bits), new here, for
 * Tux Football's two goal-post pictures.
 * The AmiCradle Team. GPL 2 or later, like Tux Football. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <png.h>
#include <jpeglib.h>
#include "SDL_image.h"

static void rw_read(png_structp png, png_bytep data, png_size_t len)
{
	SDL_RWops *rw = (SDL_RWops *)png_get_io_ptr(png);
	if (SDL_RWread(rw, data, 1, (int)len) < 1)
		png_error(png, "read error");
}

static SDL_Surface *load_png(FILE *f, SDL_RWops *rw, const char *file)
{
	png_structp png = NULL;
	png_infop info = NULL;
	SDL_Surface *volatile surf = NULL;
	png_bytep *volatile rows = NULL;
	png_uint_32 w, h, y;
	int depth, ctype;

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png)
		info = png_create_info_struct(png);
	if (!png || !info) {
		SDL_SetError("IMG_Load: out of memory");
		goto out;
	}
	if (setjmp(png_jmpbuf(png))) {
		SDL_SetError("IMG_Load: bad PNG %s", file);
		if (surf)
			SDL_FreeSurface(surf);
		surf = NULL;
		goto out;
	}
	if (f)
		png_init_io(png, f);
	else
		png_set_read_fn(png, rw, rw_read);
	png_read_info(png, info);
	png_get_IHDR(png, info, &w, &h, &depth, &ctype, NULL, NULL, NULL);
	png_set_interlace_handling(png);
	if (ctype == PNG_COLOR_TYPE_PALETTE) {
		png_colorp pal;
		int n, i;
		SDL_Color colors[256];

		if (depth < 8)
			png_set_packing(png);
		png_read_update_info(png, info);
		surf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
		if (!surf)
			goto out;
		if (png_get_PLTE(png, info, &pal, &n)) {
			if (n > 256)
				n = 256;
			for (i = 0; i < n; i++) {
				colors[i].r = pal[i].red;
				colors[i].g = pal[i].green;
				colors[i].b = pal[i].blue;
				colors[i].unused = 0;
			}
			SDL_SetColors(surf, colors, 0, n);
		}
		/* tRNS: one fully transparent colour is the colour key, as
		 * SDL_image makes it (Biniax-2's font and elements) */
		if (png_get_valid(png, info, PNG_INFO_tRNS)) {
			png_bytep trans;
			int ntrans, key = -1;
			png_get_tRNS(png, info, &trans, &ntrans, NULL);
			for (i = 0; i < ntrans; i++) {
				if (trans[i] == 0) {
					if (key >= 0) {
						key = -1;
						break;
					}
					key = i;
				} else if (trans[i] != 255) {
					key = -1;
					break;
				}
			}
			if (key >= 0)
				SDL_SetColorKey(surf, SDL_SRCCOLORKEY, (Uint32)key);
		}
	} else if (ctype & PNG_COLOR_MASK_ALPHA) {
		/* 32-bit RGBA */
		if (depth == 16)
			png_set_strip_16(png);
		if (ctype == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(png);
		png_read_update_info(png, info);
		surf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
			0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
		if (!surf)
			goto out;
	} else {
		/* RGB (or grey) in 32 bits, its tRNS colour the colour key, as
		 * SDL_image makes it (Biniax-2's window and markers) */
		png_color_16p tc = NULL;
		int has_key = 0;
		Uint8 kr = 0, kg = 0, kb = 0;

		if (png_get_valid(png, info, PNG_INFO_tRNS) &&
		    png_get_tRNS(png, info, NULL, NULL, &tc) && tc) {
			int sh = depth == 16 ? 8 : 0;
			has_key = 1;
			if (ctype == PNG_COLOR_TYPE_GRAY) {
				int g = tc->gray >> sh;
				if (depth < 8)
					g = g * 255 / ((1 << depth) - 1);
				kr = kg = kb = (Uint8)g;
			} else {
				kr = (Uint8)(tc->red >> sh);
				kg = (Uint8)(tc->green >> sh);
				kb = (Uint8)(tc->blue >> sh);
			}
		}
		if (depth == 16)
			png_set_strip_16(png);
		if (ctype == PNG_COLOR_TYPE_GRAY) {
			if (depth < 8)
				png_set_expand_gray_1_2_4_to_8(png);
			png_set_gray_to_rgb(png);
		}
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
		png_read_update_info(png, info);
		surf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
			0xFF000000, 0x00FF0000, 0x0000FF00, 0);
		if (!surf)
			goto out;
		if (has_key)
			SDL_SetColorKey(surf, SDL_SRCCOLORKEY, SDL_MapRGB(surf->format, kr, kg, kb));
	}
	rows = malloc(sizeof(png_bytep) * h);
	if (!rows) {
		SDL_FreeSurface(surf);
		surf = NULL;
		goto out;
	}
	for (y = 0; y < h; y++)
		rows[y] = (png_bytep) surf->pixels + y * surf->pitch;
	png_read_image(png, rows);
	png_read_end(png, NULL);
out:
	free(rows);
	if (png)
		png_destroy_read_struct(&png, info ? &info : NULL, NULL);
	return surf;
}

struct jpeg_fail {
	struct jpeg_error_mgr pub;
	jmp_buf jump;
};

static void jpeg_fail_exit(j_common_ptr cinfo)
{
	longjmp(((struct jpeg_fail *)cinfo->err)->jump, 1);
}

static SDL_Surface *load_jpeg(FILE *f, const char *file)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_fail err;
	SDL_Surface *volatile surf = NULL;

	cinfo.err = jpeg_std_error(&err.pub);
	err.pub.error_exit = jpeg_fail_exit;
	if (setjmp(err.jump)) {
		SDL_SetError("IMG_Load: bad JPEG %s", file);
		if (surf)
			SDL_FreeSurface(surf);
		jpeg_destroy_decompress(&cinfo);
		return NULL;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, f);
	jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space = JCS_RGB;
	jpeg_start_decompress(&cinfo);
	/* R, G, B bytes in memory: on the big-endian 68k R is the top byte */
	surf = SDL_CreateRGBSurface(SDL_SWSURFACE, cinfo.output_width, cinfo.output_height, 24,
	                            0xff0000, 0x00ff00, 0x0000ff, 0);
	if (!surf) {
		jpeg_destroy_decompress(&cinfo);
		return NULL;
	}
	while (cinfo.output_scanline < cinfo.output_height) {
		JSAMPROW row = (JSAMPROW)((Uint8 *)surf->pixels + cinfo.output_scanline * surf->pitch);
		jpeg_read_scanlines(&cinfo, &row, 1);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	return surf;
}

/* TGA, types 2 (raw) and 10 (RLE), 24 or 32 bits, from memory. 24-bit
 * pictures become 24-bit surfaces, 32-bit ones RGBA; R, G, B, A in memory. */
static SDL_Surface *load_tga(const Uint8 *d, long len, const char *file)
{
	int type, w, h, bpp, top, x, y, left = 0, run = 0, fresh = 0;
	long at;
	SDL_Surface *surf;
	Uint8 px[4] = { 0, 0, 0, 255 };

	if (len < 18 || d[1] != 0 || (d[2] != 2 && d[2] != 10) || (d[16] != 24 && d[16] != 32)) {
		SDL_SetError("IMG_Load: %s is not a truecolour TGA", file);
		return NULL;
	}
	type = d[2];
	w = d[12] | d[13] << 8;
	h = d[14] | d[15] << 8;
	bpp = d[16] / 8;
	top = (d[17] & 0x20) != 0;
	at = 18 + d[0];
	if (bpp == 4)
		surf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
		                            0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	else
		surf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24, 0xff0000, 0x00ff00, 0x0000ff, 0);
	if (!surf)
		return NULL;
	for (y = 0; y < h; y++) {
		Uint8 *row = (Uint8 *)surf->pixels + (top ? y : h - 1 - y) * surf->pitch;
		for (x = 0; x < w; x++) {
			if (type == 10 && left == 0) {	/* an RLE packet's count */
				if (at >= len)
					goto short_file;
				left = (d[at] & 0x7f) + 1;
				run = (d[at] & 0x80) != 0;
				at++;
				fresh = 1;
			}
			if (type == 2 || !run || fresh) {	/* the next pixel from the file */
				if (at + bpp > len)
					goto short_file;
				px[0] = d[at + 2]; px[1] = d[at + 1]; px[2] = d[at];
				if (bpp == 4)
					px[3] = d[at + 3];
				at += bpp;
				fresh = 0;
			}
			memcpy(row + x * bpp, px, bpp);
			if (type == 10)
				left--;
		}
	}
	return surf;
short_file:
	SDL_SetError("IMG_Load: %s is cut short", file);
	SDL_FreeSurface(surf);
	return NULL;
}

static SDL_Surface *load_tga_file(FILE *f, const char *file)
{
	long len;
	Uint8 *d;
	SDL_Surface *surf;

	fseek(f, 0, SEEK_END);
	len = ftell(f);
	rewind(f);
	d = (Uint8 *)malloc(len > 0 ? len : 1);
	if (!d || fread(d, 1, len, f) != (size_t)len) {
		free(d);
		SDL_SetError("IMG_Load: can't read %s", file);
		return NULL;
	}
	surf = load_tga(d, len, file);
	free(d);
	return surf;
}

static int is_tga_name(const char *file)
{
	size_t n = strlen(file);
	return n > 4 && (!strcmp(file + n - 4, ".tga") || !strcmp(file + n - 4, ".TGA"));
}

SDL_Surface *IMG_Load(const char *file)
{
	FILE *f = fopen(file, "rb");
	SDL_Surface *surf;
	unsigned char magic[2] = { 0, 0 };

	if (!f) {
		SDL_SetError("IMG_Load: can't open %s", file);
		return NULL;
	}
	if (fread(magic, 1, 2, f) != 2)
		magic[0] = 0;
	rewind(f);
	if (magic[0] == 0xff && magic[1] == 0xd8)
		surf = load_jpeg(f, file);
	else if (is_tga_name(file))
		surf = load_tga_file(f, file);
	else
		surf = load_png(f, NULL, file);
	fclose(f);
	return surf;
}

SDL_Surface *IMG_LoadPNG_RW(SDL_RWops *src)
{
	if (!src) {
		SDL_SetError("IMG_LoadPNG_RW: no RWops");
		return NULL;
	}
	return load_png(NULL, src, "RWops");
}

SDL_Surface *IMG_LoadTGA_RW(SDL_RWops *src)
{
	Uint8 *d = NULL;
	long len = 0, cap = 0;
	int got;
	SDL_Surface *surf;

	if (!src) {
		SDL_SetError("IMG_LoadTGA_RW: no RWops");
		return NULL;
	}
	do {
		if (len + 65536 > cap) {
			Uint8 *nd = (Uint8 *)realloc(d, cap = len + 65536);
			if (!nd) {
				free(d);
				SDL_SetError("IMG_LoadTGA_RW: out of memory");
				return NULL;
			}
			d = nd;
		}
		got = SDL_RWread(src, d + len, 1, (int)(cap - len));
		if (got > 0)
			len += got;
	} while (got > 0);
	surf = load_tga(d, len, "RWops");
	free(d);
	return surf;
}

int IMG_InvertAlpha(int on)
{
	(void)on;
	return 1;
}

const SDL_version *IMG_Linked_Version(void)
{
	static SDL_version v = { 1, 2, 12 };
	return &v;
}
