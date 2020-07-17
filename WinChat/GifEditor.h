#ifndef __GIF_EDITOR_H__
#define __GIF_EDITOR_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <io.h>
#include "getopt.h"
// #include "getarg.h"
#include "gif_lib.h"

#define MAX_FRAME_NUM 200
#define PROGRAM_NAME	"giftool"

#define GIF_WIDTH 320
#define GIF_HEIGHT 240
#define GIF_FRAME 100
#define MAX_OPERATIONS	256
#define MAX_IMAGES	2048
#define PrintGifError(x) \
	do{ \
		FILE *fp = fopen("log.txt","a"); \
		int e = GetLastError(); \
		char buf[32] = {0}; \
		snprintf(buf, sizeof(buf), "%d", e); \
		fwrite(buf, strlen(buf), 1, fp); \
		fclose(fp); \
	} while (0)
#define GIF_EXIT()

typedef struct {
	char gitdir[MAX_PATH];
	int width;
	int height;
	unsigned char *frame[GIF_FRAME];
}GifBitMap;

int GifRead(DWORD *arg);
int SetRecord(int frameIdx);
int GetRecord();
int GifWrite(DWORD *arg, int Width, int Height);

#endif