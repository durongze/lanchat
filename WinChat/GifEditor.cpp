/****************************************************************************

giftool.c - GIF transformation tool.

****************************************************************************/
#include "GifEditor.h"

void DumpScreen2RGBA(unsigned char *GrbBuffer,
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

int HandleImageDesc(GifFileType *GifFile, GifRowType *ScreenBuffer, unsigned char *GrbBuffer)
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

int FreeAllFrame(GifBitMap *pGrbBuffer)
{
	int i = 0;
	for (i = 0; i < sizeof(pGrbBuffer->frame) / sizeof(unsigned char*); i++) {
		if (pGrbBuffer->frame[i] != NULL) {
			free(pGrbBuffer->frame[i]);
		}
	}
	memset(pGrbBuffer->frame, 0, sizeof(pGrbBuffer->frame));
	return 0;
}

GifRowType *CreateScreenBuffer(GifFileType *GifFile)
{
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
	return ScreenBuffer;
}

int GifRead(DWORD *arg)
{
	int error, frameIdx = 0;
	GifBitMap *pGrbBuffer = (GifBitMap*)arg;
	FreeAllFrame(pGrbBuffer);
	GifFileType *GifFile = DGifOpenFileName(pGrbBuffer->gitdir, &error);
	if (GifFile == NULL) {
		PrintGifError();
	}
	GifRowType *ScreenBuffer = CreateScreenBuffer(GifFile);
	GifRecordType RecordType;
	do {
		if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) {
			PrintGifError();
			exit(EXIT_FAILURE);
		}
		switch (RecordType) {
			case IMAGE_DESC_RECORD_TYPE:
				std::cout << "IMAGE_DESC_RECORD_TYPE" << std::endl;
				pGrbBuffer->width = GifFile->SWidth;
				pGrbBuffer->height = GifFile->SHeight;
				pGrbBuffer->frame[frameIdx] = (unsigned char*)malloc(GifFile->SWidth * GifFile->SHeight * 4);
				HandleImageDesc(GifFile, ScreenBuffer, pGrbBuffer->frame[frameIdx]);
				frameIdx += frameIdx + 1 >= sizeof(pGrbBuffer->frame) / sizeof(unsigned char*) ? -frameIdx : 1;
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

// #include "getarg.h"
#include "gif_lib.h"

#define PROGRAM_NAME	"gifsponge"

int GifWrite(DWORD *arg)
{
	int	i, ErrorCode;
	GifFileType *GifFileIn = (GifFileType *)NULL;
	GifFileType *GifFileOut = (GifFileType *)NULL;

	if ((GifFileIn = DGifOpenFileHandle(0, &ErrorCode)) == NULL) {
		PrintGifError(ErrorCode);
		exit(EXIT_FAILURE);
	}
	if (DGifSlurp(GifFileIn) == GIF_ERROR) {
		PrintGifError(GifFileIn->Error);
		exit(EXIT_FAILURE);
	}
	if ((GifFileOut = EGifOpenFileHandle(1, &ErrorCode)) == NULL) {
		PrintGifError(ErrorCode);
		exit(EXIT_FAILURE);
	}

	/*
	* Your operations on in-core structures go here.
	* This code just copies the header and each image from the incoming file.
	*/
	GifFileOut->SWidth = GifFileIn->SWidth;
	GifFileOut->SHeight = GifFileIn->SHeight;
	GifFileOut->SColorResolution = GifFileIn->SColorResolution;
	GifFileOut->SBackGroundColor = GifFileIn->SBackGroundColor;
	if (GifFileIn->SColorMap) {
		GifFileOut->SColorMap = GifMakeMapObject(
			GifFileIn->SColorMap->ColorCount,
			GifFileIn->SColorMap->Colors);
	}
	else {
		GifFileOut->SColorMap = NULL;
	}

	for (i = 0; i < GifFileIn->ImageCount; i++)
		(void) GifMakeSavedImage(GifFileOut, &GifFileIn->SavedImages[i]);

	/*
	* Note: don't do DGifCloseFile early, as this will
	* deallocate all the memory containing the GIF data!
	*
	* Further note: EGifSpew() doesn't try to validity-check any of this
	* data; it's *your* responsibility to keep your changes consistent.
	* Caveat hacker!
	*/
	if (EGifSpew(GifFileOut) == GIF_ERROR)
		PrintGifError(GifFileOut->Error);

	if (DGifCloseFile(GifFileIn, &ErrorCode) == GIF_ERROR)
		PrintGifError(ErrorCode);

	return 0;
}
/* end */
