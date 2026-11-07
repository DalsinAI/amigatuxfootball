/* SDL_RWFromFP and SDL_RWFromFile for Tux Football on AmigaOS
 * (-Wl,--wrap=SDL_RWFromFP,SDL_RWFromFile), from the Team's Abe's Amazing
 * Adventure, Holotz's Castle, Gigalomania and Eat The Whistle ports. Tux
 * Football falls back to a RWops when IMG_Load gives up on a picture;
 * OpenGPU's SDL 1.2 (sdl12-compat) hands those to SDL2.module, and in Abe's
 * lab the first read never returned.
 * Only the game's own code reads these RWops, so they are plain stdio here.
 * The AmiCradle Team. GPL 3 or later (Gigalomania is GPL 2 or later). */
#include <stdio.h>
#include <SDL.h>

static int SDLCALL rw_seek(SDL_RWops *rw, int offset, int whence)
{
	FILE *f = rw->hidden.stdio.fp;
	int w = whence == RW_SEEK_CUR ? SEEK_CUR : whence == RW_SEEK_END ? SEEK_END : SEEK_SET;
	if (fseek(f, offset, w) != 0) {
		SDL_SetError("fseek failed");
		return -1;
	}
	return (int)ftell(f);
}

static int SDLCALL rw_read(SDL_RWops *rw, void *ptr, int size, int maxnum)
{
	if (size <= 0 || maxnum <= 0)
		return 0;
	return (int)fread(ptr, (size_t)size, (size_t)maxnum, rw->hidden.stdio.fp);
}

static int SDLCALL rw_write(SDL_RWops *rw, const void *ptr, int size, int num)
{
	if (size <= 0 || num <= 0)
		return 0;
	return (int)fwrite(ptr, (size_t)size, (size_t)num, rw->hidden.stdio.fp);
}

static int SDLCALL rw_close(SDL_RWops *rw)
{
	int r = 0;
	if (rw) {
		if (rw->hidden.stdio.autoclose)
			r = fclose(rw->hidden.stdio.fp) == 0 ? 0 : -1;
		SDL_FreeRW(rw);
	}
	return r;
}

static SDL_RWops *rw_from(FILE *f, int autoclose)
{
	SDL_RWops *rw = SDL_AllocRW();
	if (!rw)
		return NULL;
	rw->seek = rw_seek;
	rw->read = rw_read;
	rw->write = rw_write;
	rw->close = rw_close;
	rw->hidden.stdio.fp = f;
	rw->hidden.stdio.autoclose = autoclose;
	return rw;
}

SDL_RWops *__wrap_SDL_RWFromFP(FILE *fp, int autoclose)
{
	if (!fp) {
		SDL_SetError("SDL_RWFromFP: no file");
		return NULL;
	}
	return rw_from(fp, autoclose);
}

SDL_RWops *__wrap_SDL_RWFromFile(const char *file, const char *mode)
{
	FILE *f;
	SDL_RWops *rw;
	if (!file || !mode) {
		SDL_SetError("SDL_RWFromFile: no file name or mode");
		return NULL;
	}
	f = fopen(file, mode);
	if (!f) {
		SDL_SetError("Couldn't open %s", file);
		return NULL;
	}
	rw = rw_from(f, 1);
	if (!rw)
		fclose(f);
	return rw;
}
