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

void DumpScreen2RGBA(UINT8* GrbBuffer,
	ColorMapObject *ColorMap, GifRowType *ScreenBuffer, int ScreenWidth, int ScreenHeight)
{
    int i, j;
    GifRowType GifRow;
    static GifColorType *ColorMapEntry;
    unsigned char *BufferP;
	for (i = 0; i < ScreenHeight; i++) {
		GifRow = ScreenBuffer[i];
		BufferP = GrbBuffer + i * (ScreenWidth * 4);
		for (j = 0; j < ScreenWidth; j++) {
			ColorMapEntry = &ColorMap->Colors[GifRow[j]];
			*BufferP++ = ColorMapEntry->Blue;
			*BufferP++ = ColorMapEntry->Green;
			*BufferP++ = ColorMapEntry->Red;
			*BufferP++ = 0xff;
		}
	}
}

int HandleImageDesc(GifFileType *GifFile, GifRowType *ScreenBuffer, UINT8* GrbBuffer)
{
	if (DGifGetImageDesc(GifFile) == GIF_ERROR) {
		PrintGifError();
		exit(EXIT_FAILURE);
	}
	GifWord i, j, Count;
	GifWord Row = GifFile->Image.Top; /* Image Position relative to Screen. */
	GifWord Col = GifFile->Image.Left;
	GifWord Width = GifFile->Image.Width;
	GifWord Height = GifFile->Image.Height;
	// printf("\n%s: Image %d at (%d, %d) [%dx%d]: ", PROGRAM_NAME, ++ImageNum, Col, Row, Width, Height);
	if (GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth ||
		GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight) {
		// fprintf(stderr, "Image %d is not confined to screen dimension, aborted.\n", ImageNum);
		exit(EXIT_FAILURE);
	}
	if (GifFile->Image.Interlace) {
		int InterlacedOffset[] = { 0, 4, 2, 1 };
		int InterlacedJumps[] = { 8, 8, 4, 2 };
		/* Need to perform 4 passes on the images: */
		for (Count = i = 0; i < 4; i++)
		for (j = Row + InterlacedOffset[i]; j < Row + Height; j += InterlacedJumps[i]) {
			printf("\b\b\b\b%-4d", Count++);
			if (DGifGetLine(GifFile, &ScreenBuffer[j][Col], Width) == GIF_ERROR) {
				PrintGifError();
				exit(EXIT_FAILURE);
			}
		}
	} else {
		for (i = 0; i < Height; i++) {
			printf("\b\b\b\b%-4d", i);
			if (DGifGetLine(GifFile, &ScreenBuffer[Row++][Col], Width) == GIF_ERROR) {
				PrintGifError();
				exit(EXIT_FAILURE);
			}
		}
	}
	/* Get the color map */
	ColorMapObject *ColorMap = (GifFile->Image.ColorMap	? GifFile->Image.ColorMap : GifFile->SColorMap);
	if (ColorMap == NULL) {
		fprintf(stderr, "Gif Image does not have a colormap\n");
		exit(EXIT_FAILURE);
	}
	DumpScreen2RGBA(GrbBuffer, ColorMap, ScreenBuffer, GifFile->SWidth, GifFile->SHeight);
	HBITMAP hBitmap = CreateBitmap(GifFile->SWidth, GifFile->SHeight, 1, 32, GrbBuffer);
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, hBitmap);
	CloseClipboard();
	return 0;
}

int HandleExt(GifFileType *GifFile)
{
	int ExtFunction;
	GifByteType *ExtData;
	if (DGifGetExtension(GifFile, &ExtFunction, &ExtData) == GIF_ERROR)
		return (GIF_ERROR);
	/* Create an extension block with our data */
	if (ExtData != NULL) {
		if (GifAddExtensionBlock(&GifFile->ExtensionBlockCount,
			&GifFile->ExtensionBlocks,
			ExtFunction, ExtData[0], &ExtData[1])
			== GIF_ERROR)
			return (GIF_ERROR);
	}
	while (ExtData != NULL) {
		if (DGifGetExtensionNext(GifFile, &ExtData) == GIF_ERROR)
			return (GIF_ERROR);
		/* Continue the extension block */
		if (ExtData != NULL)
			if (GifAddExtensionBlock(&GifFile->ExtensionBlockCount,
				&GifFile->ExtensionBlocks,
				CONTINUE_EXT_FUNC_CODE,
				ExtData[0], &ExtData[1]) == GIF_ERROR)
				return (GIF_ERROR);
	}
	return 0;
}

int gif_main(int argc, char **argv)
{
	int error;
	unsigned char *GrbBuffer = NULL;
	GifFileType *GifFile;
	char *fileName = "man.gif";
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
				std::cout << "IMAGE_DESC_RECORD_TYPE" << std::endl;
				GrbBuffer = (unsigned char*)malloc(GifFile->SWidth * GifFile->SHeight * 4);
				HandleImageDesc(GifFile, ScreenBuffer, GrbBuffer);
				delete GrbBuffer;
				break;
			case EXTENSION_RECORD_TYPE:
				std::cout << "EXTENSION_RECORD_TYPE" << std::endl;
				HandleExt(GifFile);
				break;
			case TERMINATE_RECORD_TYPE:
				std::cout << "TERMINATE_RECORD_TYPE" << std::endl;
				break;
			default:
				break;
		}
	} while (RecordType != TERMINATE_RECORD_TYPE);
	return 0;
}

/* end */
