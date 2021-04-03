/****************************************************************************

giftool.c - GIF transformation tool.

****************************************************************************/
#include "GifEditor.h"
#include <time.h>

static int g_frameIdx = -1;
//将string转换成wstring  
std::wstring string2wstring(std::string str)
{
	std::wstring result;
	//获取缓冲区大小，并申请空间，缓冲区大小按字符计算  
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	TCHAR* buffer = new TCHAR[len + 1];
	//多字节编码转换成宽字节编码  
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
	buffer[len] = '\0';             //添加字符串结尾  
									//删除缓冲区并返回值  
	result.append(buffer);
	delete[] buffer;
	return result;
}

//将wstring转换成string  
std::string wstring2string(std::wstring wstr)
{
	std::string result;
	//获取缓冲区大小，并申请空间，缓冲区大小事按字节计算的  
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
	char* buffer = new char[len + 1];
	//宽字节编码转换成多字节编码  
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
	buffer[len] = '\0';
	//删除缓冲区并返回值  
	result.append(buffer);
	delete[] buffer;
	return result;
}
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
ColorMapObject *g_pColorMap = NULL;

int HandleImageSave(std::string imgFile, GifFileType *GifFile, unsigned char *GrbBuffer, uint8_t pixByte)
{
	HANDLE hFile = CreateFile(string2wstring(imgFile + ".bmp").c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL) {
		return -1;
	};
	int bmp_size = GifFile->SWidth * GifFile->SHeight * pixByte;
	int fh_size = sizeof(BITMAPFILEHEADER);
	int ih_size = sizeof(BITMAPINFOHEADER);
	
	// 【写位图文件头】
	BITMAPFILEHEADER bmpHeader;
	bmpHeader.bfSize = fh_size + ih_size + bmp_size;  // BMP图像文件的大小
	bmpHeader.bfType = 0x4D42;  // 位图类别，根据不同的操作系统而不同，在Windows中，此字段的值总为‘BM’
	bmpHeader.bfOffBits = fh_size + ih_size;          // BMP图像数据的偏移位置
	bmpHeader.bfReserved1 = 0;  // 总为0
	bmpHeader.bfReserved2 = 0;  // 总为0
	DWORD dwBytesWritten = 0;

	WriteFile(hFile, (LPSTR)&bmpHeader, sizeof(bmpHeader), &dwBytesWritten, NULL);
	
	BITMAPINFOHEADER bmiHeader;
	bmiHeader.biSize = sizeof(bmiHeader);               // 本结构所占用字节数，即sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = GifFile->SWidth;                          // 位图宽度（单位：像素）
	bmiHeader.biHeight = GifFile->SHeight;                        // 位图高度（单位：像素）
	bmiHeader.biPlanes = 1;                             // 目标设备的级别，必须为1
	bmiHeader.biBitCount = pixByte * 8;                    // 像素的位数（每个像素所需的位数，范围：1、4、8、24、32）
	bmiHeader.biCompression = 0;                        // 压缩类型（0：不压缩 1：BI_RLE8压缩类型 2：BI_RLE4压缩类型）
	bmiHeader.biSizeImage = bmp_size;                   // 位图大小（单位：字节）
	bmiHeader.biXPelsPerMeter = 0;                      // 水平分辨率(像素/米)
	bmiHeader.biYPelsPerMeter = 0;                      // 垂直分辨率(像素/米)
	bmiHeader.biClrUsed = 0;                            // 位图实际使用的彩色表中的颜色索引数
	bmiHeader.biClrImportant = 0;                       // 对图象显示有重要影响的颜色索引的数目
														// 【写位图信息头（BITMAPINFO的bmiHeader成员）】
	WriteFile(hFile, (LPSTR)&bmiHeader, sizeof(bmiHeader), &dwBytesWritten, NULL);
	
	// 【写像素内容】
	WriteFile(hFile, GrbBuffer, bmp_size, &dwBytesWritten, NULL);
	CloseHandle(hFile);
	return 0;
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
	g_pColorMap = GifMakeMapObject(ColorMap->ColorCount, ColorMap->Colors);
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
		return 0;
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
				HandleImageSave(std::string(pGrbBuffer->gitdir) + std::to_string(frameIdx), GifFile, pGrbBuffer->frame[frameIdx]);
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

	DGifCloseFile(GifFile, &error);
	return 0;
}

int GifWriteScreen(GifFileType *pGifFile, ColorMapObject *pColorMap)
{
	EGifSetGifVersion(pGifFile, true);
	EGifPutScreenDesc(pGifFile, pGifFile->SWidth, pGifFile->SHeight, 8, 0, pColorMap);
	// （3)写入graphic control extension块，此块可以设定动画速度和透明色
	static const GraphicsControlBlock gcb = { DISPOSE_DO_NOT, false, 1, NO_TRANSPARENT_COLOR };
	static const GifByteType gcbLen = 4;
	static GifByteType gcbBytes[gcbLen];
	EGifGCBToExtension(&gcb, gcbBytes);
	EGifPutExtension(pGifFile, GRAPHICS_EXT_FUNC_CODE, gcbLen, gcbBytes);

	static const GifByteType aeLen = 11;
	static const char *aeBytes = { "NETSCAPE2.0" };
	static const GifByteType aeSubLen = 3;
	static GifByteType aeSubBytes[aeSubLen];
	aeSubBytes[0] = 0x01;
	aeSubBytes[1] = 0x00; // byte[1]是低位，byte[2]是高位，组成一个无符号16位数，决定动画循环次数
	aeSubBytes[2] = 0x00;
	EGifPutExtensionLeader(pGifFile, APPLICATION_EXT_FUNC_CODE);
	EGifPutExtensionBlock(pGifFile, aeLen, aeBytes);
	EGifPutExtensionBlock(pGifFile, aeSubLen, aeSubBytes);
	EGifPutExtensionTrailer(pGifFile);
	return 0;
}

#include "octree.h"

int GifColorTypeToNumber(GifColorType& color, Number &num, int idxNum)
{
	for (int j = 7; j >= 0; --j) {
		int v = (color.Red & (1 << j)) >> j << 2 | (color.Green & (1 << j)) >> j << 1 | (color.Blue & (1 << j)) >> j;
		num.SetBit(8 - j, v, idxNum);
	}
	return 0;
}

static Octree o;

int GifWriteImage(GifFileType *pGifFile, ColorMapObject *pColorMap, uint8_t * bits, uint8_t pixByte)
{
	// 写入image descriptor块，因为不使用局部颜色表，传入nullptr，而不传pColorMap
	EGifPutImageDesc(pGifFile, 0, 0, pGifFile->SWidth, pGifFile->SHeight, false, nullptr);
	GifColorType *pCm = pColorMap->Colors; // color map

	// 写入image data
	for (int k = 0; k < pGifFile->SWidth * pGifFile->SHeight; ++k) {
		uint8_t index = 0;
		int mindis = 1 << 30;
		uint8_t rr = *(bits + k * pixByte + 0);
		uint8_t gg = *(bits + k * pixByte + 1);
		uint8_t bb = *(bits + k * pixByte + 2);
		GifColorType color = { rr, gg, bb };
		// 将颜色匹配到颜色表的索引颜色 256色
#if 0
		for (int i = 0; i < (1 << 8); i++) {
			int dis =
				(rr - pCm[i].Red)*(rr - pCm[i].Red) +
				(gg - pCm[i].Green)*(gg - pCm[i].Green) +
				(bb - pCm[i].Blue)*(bb - pCm[i].Blue);
			if (dis < mindis) {
				mindis = dis;
				index = i;
			}
			if (dis == 0) {
				index = i;
				break;
			}
		}
#endif
		Number n;
		GifColorTypeToNumber(color, n, 0);
		index = o.GetNumberType(n);
		EGifPutPixel(pGifFile, (uint8_t)index);
	}
	return 0;
}


int ColorMapObjectToOctree(GifColorType* Colors, int ColorCount)
{
	o.Clear();
	for (int i = 0; i < ColorCount; i++) {
		Number num;
		for (int j = 7; j >= 0; --j) {
			int type = ((Colors[i].Red & (1 << j)) >> j << 2) | ((Colors[i].Green & (1 << j)) >> j << 1) | ((Colors[i].Blue & (1 << j)) >> j);
			num.SetBit(8 - j, type, i);
		}
		o.InsertNumber(num);
	}
	std::fstream fsOct;
	fsOct.open("o_log.txt", std::ios::trunc | std::ios::out | std::ios::in);
	o.Dump(fsOct);
	fsOct.close();

	return 0;
}

int GifWrite(std::string gifDir, int Width, int Height, uint8_t *bits)
{
	int errorStatus;
	static GifFileType *pGifFile = NULL;
	static ColorMapObject* pColorMap = NULL;
	if (pGifFile == NULL && g_frameIdx == 0) {
		pGifFile = EGifOpenFileName((gifDir + std::to_string(time(0)) + ".gif").c_str(), 0, &errorStatus);
		if (pGifFile == NULL) {
			return 1;
		}
		pGifFile->SWidth = Width;
		pGifFile->SHeight = Height;
		pColorMap = GifMakeMapObject(256, (GifColorType*)bits);
		// pColorMap = g_pColorMap;
		ColorMapObjectToOctree(pColorMap->Colors, pColorMap->ColorCount);
		GifWriteScreen(pGifFile, pColorMap);
	}

	if (pGifFile != NULL) {
		GifWriteImage(pGifFile, pColorMap, bits);
		// HandleImageSave(std::to_string(g_frameIdx), pGifFile, bits);
	}

	if (pGifFile != NULL && g_frameIdx++ > MAX_FRAME_NUM) {
		EGifCloseFile(pGifFile, &errorStatus);
		pGifFile = NULL;
		g_frameIdx = -1;
		GifFreeMapObject(pColorMap);
		pColorMap = NULL;
	}
	return 0;
}

int SetRecord(int frameIdx)
{
	g_frameIdx = frameIdx;
	return frameIdx;
}

int GetRecord()
{
	return g_frameIdx;
}