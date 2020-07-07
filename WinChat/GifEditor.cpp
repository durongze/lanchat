/****************************************************************************

giftool.c - GIF transformation tool.

****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <windows.h>
#include <iostream>
#include <io.h>
#include "getopt.h"
// #include "getarg.h"
#include "gif_lib.h"

#define PROGRAM_NAME	"giftool"

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
int gif_main(int argc, char **argv)
{
	int error;
	GifFileType *GifFile;
	char *fileName = "china.gif";
	GifFile = DGifOpenFileName(fileName, &error);
	if (GifFile == NULL) {
		PrintGifError();
	}
	GifRowType *ScreenBuffer = (GifRowType *)malloc(GifFile->SHeight * sizeof(GifRowType *));
	if (ScreenBuffer == NULL) {
		GIF_EXIT("Failed to allocate memory required, aborted.");
	}
	GifWord Size = GifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/
	if ((ScreenBuffer[0] = (GifRowType)malloc(Size)) == NULL) {
		GIF_EXIT("Failed to allocate memory required, aborted.");
	}
	for (int i = 0; i < GifFile->SWidth; i++) {
		/* Set its color to BackGround. */
		ScreenBuffer[0][i] = GifFile->SBackGroundColor;
	}
	for (int i = 1; i < GifFile->SHeight; i++) {
		/* Allocate the other rows, and set their color to background too: */
		if ((ScreenBuffer[i] = (GifRowType)malloc(Size)) == NULL) {
			GIF_EXIT("Failed to allocate memory required, aborted.");
		}
		memcpy(ScreenBuffer[i], ScreenBuffer[0], Size);
	}
	GifRecordType RecordType;
	do {
		if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) {
			PrintGifError();
			exit(EXIT_FAILURE);
		}
		switch (RecordType) {
			case IMAGE_DESC_RECORD_TYPE:
				break;
			case EXTENSION_RECORD_TYPE:
				break;
			case TERMINATE_RECORD_TYPE:
				break;
			default:
				break;
		}
	} while (RecordType != TERMINATE_RECORD_TYPE);
	return 0;
}

/* end */
