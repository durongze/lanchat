#ifndef __FREE_TYPE_BMP_H__
#define __FREE_TYPE_BMP_H__

#include "ft2build.h"
#include "freetype/freetype.h"
#include <freetype/ftglyph.h>
#include <windows.h>
#include <iostream>

typedef struct tagIMAGEDATA
{
	BYTE blue;
	BYTE green;
	BYTE red;
	BYTE alpha; // 本地图片不需要，桌面截图需要
}IMAGEDATA;

int WordInsertToBmp(char *strFile, char *dstFile);
int WriteWordToBmp(BITMAPINFOHEADER& strInfo, IMAGEDATA*& arrayColor);

#endif