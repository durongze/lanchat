#include "FreeTypeBmp.h"

//显示位图文件头信息   
void showBmpHead(BITMAPFILEHEADER pBmpHead) {
	std::cout << "位图文件头:" << std::endl;
	std::cout << "bfType value is " << pBmpHead.bfType << std::endl;
	std::cout << "文件大小:" << pBmpHead.bfSize << std::endl;
	std::cout << "保留字_1:" << pBmpHead.bfReserved1 << std::endl;
	std::cout << "保留字_2:" << pBmpHead.bfReserved2 << std::endl;
	std::cout << "实际位图数据的偏移字节数:" << pBmpHead.bfOffBits << std::endl;
}

void showBmpInforHead(tagBITMAPINFOHEADER pBmpInforHead) {
	std::cout << "位图信息头:" << std::endl;
	std::cout << "结构体的长度:" << pBmpInforHead.biSize << std::endl;
	std::cout << "位图宽:" << pBmpInforHead.biWidth << std::endl;
	std::cout << "位图高:" << pBmpInforHead.biHeight << std::endl;
	std::cout << "biPlanes平面数:" << pBmpInforHead.biPlanes << std::endl;
	std::cout << "biBitCount采用颜色位数:" << pBmpInforHead.biBitCount << std::endl;
	std::cout << "压缩方式:" << pBmpInforHead.biCompression << std::endl;
	std::cout << "biSizeImage实际位图数据占用的字节数:" << pBmpInforHead.biSizeImage << std::endl;
	std::cout << "X方向分辨率:" << pBmpInforHead.biXPelsPerMeter << std::endl;
	std::cout << "Y方向分辨率:" << pBmpInforHead.biYPelsPerMeter << std::endl;
	std::cout << "使用的颜色数:" << pBmpInforHead.biClrUsed << std::endl;
	std::cout << "重要颜色数:" << pBmpInforHead.biClrImportant << std::endl;
}

int OpenFreeType(FT_Library& pFTLib, FT_Face& pFTFace)
{
	FT_Error error = 0;
	//Init FreeType Lib to manage memory
	error = FT_Init_FreeType(&pFTLib);
	if (error) {
		std::cout << "FT_Init_FreeType " << error << std::endl;
		return   -1;
	}
	//从字体文件创建face，simhei.ttf是黑体
	error = FT_New_Face(pFTLib, "C:/Windows/Fonts/SIMLI.TTF", 0, &pFTFace);
	if (error) {
		std::cout << "FT_New_Face " << error << std::endl;
	}
	return 0;
}

int CloseFreeType(FT_Library& pFTLib, FT_Face& pFTFace)
{
	// free face
	FT_Done_Face(pFTFace);
	pFTFace = NULL;

	// free FreeType Lib
	FT_Done_FreeType(pFTLib);
	pFTLib = NULL;
	return 0;
}

int ParseBmpFile(const char* strFile,
	BITMAPFILEHEADER& strHead, BITMAPINFOHEADER& strInfo, IMAGEDATA*& arrayColor)
{
	FILE *fpi = fopen(strFile, "rb");
	if (fpi != NULL) {
		// 读取bmp文件的文件头和信息头   
		fread(&strHead, 1, sizeof(tagBITMAPFILEHEADER), fpi);
		if (0x4d42 != strHead.bfType) {
			std::cout << "the file is not a bmp file!" << std::endl;
			return -1;
		}
		showBmpHead(strHead); // 显示文件头   
		fread(&strInfo, 1, sizeof(tagBITMAPINFOHEADER), fpi);
		showBmpInforHead(strInfo); // 显示文件信息头 
		// 动态分配数组大小，大小为实际图片位图高*位图宽
		arrayColor = new  IMAGEDATA[strInfo.biWidth * strInfo.biHeight];
		// 读取像素信息
		fread(arrayColor, 1, strInfo.biWidth * strInfo.biHeight * sizeof(IMAGEDATA), fpi);
		fclose(fpi);
	} else {
		std::cout << "file open error!" << std::endl;
		return -1;
	}
	return 0;
}

int SaveBmpFile(const char* strFile, 
	BITMAPFILEHEADER& strHead, BITMAPINFOHEADER& strInfo, IMAGEDATA*& arrayColor)
{
	FILE *fpw;
	//保存bmp图片   
	if ((fpw = fopen(strFile, "wb")) == NULL) {
		std::cout << "create the bmp file error!" << std::endl;
		return -1;
	}
	//写头文件，14位
	fwrite(&strHead, 1, sizeof(tagBITMAPFILEHEADER), fpw);
	//写头文件，40位头文件信息
	strInfo.biHeight = -strInfo.biHeight;//正数是从数组的末尾开始扫描，负数表示从数组的开始端开始扫描
	fwrite(&strInfo, 1, sizeof(tagBITMAPINFOHEADER), fpw);
	//保存修改后的像素数据   
	fwrite(arrayColor, 1, strInfo.biWidth * strInfo.biHeight * sizeof(IMAGEDATA), fpw);

	fclose(fpw);
	return 0;
}

int GenUtf8ByChar(char* szAnsi, wchar_t*& wszString, int& wcsLen)
{
	wcsLen = ::MultiByteToWideChar(CP_ACP, NULL, szAnsi, strlen(szAnsi), NULL, 0);
	std::cout << "wcsLen=" << wcsLen << std::endl;
	//分配空间要给'\0'留个空间，MultiByteToWideChar不会给'\0'空间
	wszString = new wchar_t[wcsLen + 1];
	//转换
	::MultiByteToWideChar(CP_ACP, NULL, szAnsi, strlen(szAnsi), wszString, wcsLen);
	//最后加上'\0'
	wszString[wcsLen] = '\0';
	return 0;
}

int DrawWord(int bitmapStart, FT_Bitmap& bitmap, BITMAPINFOHEADER& strInfo, IMAGEDATA*& arrayColor)
{
	for (int i = 0; i < bitmap.rows; ++i) {
		for (int j = 0; j < bitmap.width; ++j) {
			if (bitmap.buffer[i * bitmap.width + j] != 0) {
				//像素点存在，就置为红色，其他都为默认的白色
				arrayColor[(i + 20)*strInfo.biWidth + j + bitmapStart + 20].green = 0;
				arrayColor[(i + 20)*strInfo.biWidth + j + bitmapStart + 20].blue = 0;
			}
		}
	}
	return 0;
}

int WriteWord(WORD word, int& bitmapStart, FT_Face& pFTFace,
	BITMAPINFOHEADER& strInfo, IMAGEDATA*& arrayColor)
{
	FT_Error error = 0;
	FT_Glyph glyph;
	//读取一个字体位图到face中
	FT_Load_Glyph(pFTFace, FT_Get_Char_Index(pFTFace, word), FT_LOAD_DEFAULT);
	error = FT_Get_Glyph(pFTFace->glyph, &glyph);
	if (error) {
		return -1;
	}
	//  convert glyph to bitmap with 256 gray
	FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
	//把字插入到图片中，每个字中间间隔10个像素，并且离左上角x=100,y=100偏移量
	DrawWord(bitmapStart, bitmap_glyph->bitmap, strInfo, arrayColor);
	bitmapStart += bitmap_glyph->bitmap.width + 5;
	//  free glyph
	FT_Done_Glyph(glyph);
	glyph = NULL;
	return 0;
}

int WriteWordToBmpByFace(FT_Face& pFTFace, BITMAPINFOHEADER& strInfo, IMAGEDATA*& arrayColor)
{
	WORD word;
	wchar_t *pUser = NULL;
	int pUserLen = 0;
	wchar_t user[MAX_PATH] = { 0 };
	DWORD userLen = sizeof(user);
	GenUtf8ByChar(("麦思"), pUser, pUserLen);
	if (pUser == NULL) return -1;
	// GetUserName(user, &userLen);
	//字体偏移量，用做字体显示
	int bitmap_width_sum = rand() % 2 * 200;
	//for循环实现一个字一个字插入到图片中
	for (int k = 0; k < 1; k++) {
		//复制内存块，把wszString中存储的文字一个一个取出来，复制到word中，已方便读取字体位图
		memcpy(&word, pUser + rand() % 2, 2);
		WriteWord(word, bitmap_width_sum, pFTFace, strInfo, arrayColor);
	}
	delete[] pUser;
	return 0;
}

int WriteWordToBmp(BITMAPINFOHEADER& strInfo, IMAGEDATA*& arrayColor)
{
	FT_Library pFTLib = NULL;
	FT_Face pFTFace = NULL;
	OpenFreeType(pFTLib, pFTFace);
	FT_Set_Char_Size(pFTFace, 0, 3*64, 500, 500);//设置字体大小
	FT_Vector pen;   
	FT_Matrix matrix; /* transformation matrix */
	double angle = rand();
	angle = angle / 360 * 3.1415926 * 2;
	/* 将该文字坐标转为笛卡尔坐标 */
	pen.x = 300 * 64;
	pen.y = 200 * 64; 
	matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
	matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
	matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
	matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
	FT_Set_Transform(pFTFace, &matrix, &pen);
    // FT_Set_Pixel_Sizes(pFTFace,0,16 );
	WriteWordToBmpByFace(pFTFace, strInfo, arrayColor);

	CloseFreeType(pFTLib, pFTFace);
	return 0;
}

int WordInsertToBmp(char *strFile, char *dstFile)
{
	BITMAPFILEHEADER strHead;
	BITMAPINFOHEADER strInfo;
	IMAGEDATA* arrayColor;
	
	ParseBmpFile(strFile, strHead, strInfo, arrayColor);
	WriteWordToBmp(strInfo, arrayColor);
	SaveBmpFile(dstFile, strHead, strInfo, arrayColor);
	return 0;
}
