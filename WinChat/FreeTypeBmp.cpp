#include "FreeTypeBmp.h"

//��ʾλͼ�ļ�ͷ��Ϣ   
void showBmpHead(BITMAPFILEHEADER pBmpHead) {
	std::cout << "λͼ�ļ�ͷ:" << std::endl;
	std::cout << "bfType value is " << pBmpHead.bfType << std::endl;
	std::cout << "�ļ���С:" << pBmpHead.bfSize << std::endl;
	std::cout << "������_1:" << pBmpHead.bfReserved1 << std::endl;
	std::cout << "������_2:" << pBmpHead.bfReserved2 << std::endl;
	std::cout << "ʵ��λͼ���ݵ�ƫ���ֽ���:" << pBmpHead.bfOffBits << std::endl;
}

void showBmpInforHead(tagBITMAPINFOHEADER pBmpInforHead) {
	std::cout << "λͼ��Ϣͷ:" << std::endl;
	std::cout << "�ṹ��ĳ���:" << pBmpInforHead.biSize << std::endl;
	std::cout << "λͼ��:" << pBmpInforHead.biWidth << std::endl;
	std::cout << "λͼ��:" << pBmpInforHead.biHeight << std::endl;
	std::cout << "biPlanesƽ����:" << pBmpInforHead.biPlanes << std::endl;
	std::cout << "biBitCount������ɫλ��:" << pBmpInforHead.biBitCount << std::endl;
	std::cout << "ѹ����ʽ:" << pBmpInforHead.biCompression << std::endl;
	std::cout << "biSizeImageʵ��λͼ����ռ�õ��ֽ���:" << pBmpInforHead.biSizeImage << std::endl;
	std::cout << "X����ֱ���:" << pBmpInforHead.biXPelsPerMeter << std::endl;
	std::cout << "Y����ֱ���:" << pBmpInforHead.biYPelsPerMeter << std::endl;
	std::cout << "ʹ�õ���ɫ��:" << pBmpInforHead.biClrUsed << std::endl;
	std::cout << "��Ҫ��ɫ��:" << pBmpInforHead.biClrImportant << std::endl;
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
	//�������ļ�����face��simhei.ttf�Ǻ���
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
		// ��ȡbmp�ļ����ļ�ͷ����Ϣͷ   
		fread(&strHead, 1, sizeof(tagBITMAPFILEHEADER), fpi);
		if (0x4d42 != strHead.bfType) {
			std::cout << "the file is not a bmp file!" << std::endl;
			return -1;
		}
		showBmpHead(strHead); // ��ʾ�ļ�ͷ   
		fread(&strInfo, 1, sizeof(tagBITMAPINFOHEADER), fpi);
		showBmpInforHead(strInfo); // ��ʾ�ļ���Ϣͷ 
		// ��̬���������С����СΪʵ��ͼƬλͼ��*λͼ��
		arrayColor = new  IMAGEDATA[strInfo.biWidth * strInfo.biHeight];
		// ��ȡ������Ϣ
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
	//����bmpͼƬ   
	if ((fpw = fopen(strFile, "wb")) == NULL) {
		std::cout << "create the bmp file error!" << std::endl;
		return -1;
	}
	//дͷ�ļ���14λ
	fwrite(&strHead, 1, sizeof(tagBITMAPFILEHEADER), fpw);
	//дͷ�ļ���40λͷ�ļ���Ϣ
	strInfo.biHeight = -strInfo.biHeight;//�����Ǵ������ĩβ��ʼɨ�裬������ʾ������Ŀ�ʼ�˿�ʼɨ��
	fwrite(&strInfo, 1, sizeof(tagBITMAPINFOHEADER), fpw);
	//�����޸ĺ����������   
	fwrite(arrayColor, 1, strInfo.biWidth * strInfo.biHeight * sizeof(IMAGEDATA), fpw);

	fclose(fpw);
	return 0;
}

int GenUtf8ByChar(char* szAnsi, wchar_t*& wszString, int& wcsLen)
{
	wcsLen = ::MultiByteToWideChar(CP_ACP, NULL, szAnsi, strlen(szAnsi), NULL, 0);
	std::cout << "wcsLen=" << wcsLen << std::endl;
	//����ռ�Ҫ��'\0'�����ռ䣬MultiByteToWideChar�����'\0'�ռ�
	wszString = new wchar_t[wcsLen + 1];
	//ת��
	::MultiByteToWideChar(CP_ACP, NULL, szAnsi, strlen(szAnsi), wszString, wcsLen);
	//������'\0'
	wszString[wcsLen] = '\0';
	return 0;
}

int DrawWord(int bitmapStart, FT_Bitmap& bitmap, BITMAPINFOHEADER& strInfo, IMAGEDATA*& arrayColor)
{
	for (int i = 0; i < bitmap.rows; ++i) {
		for (int j = 0; j < bitmap.width; ++j) {
			if (bitmap.buffer[i * bitmap.width + j] != 0) {
				//���ص���ڣ�����Ϊ��ɫ��������ΪĬ�ϵİ�ɫ
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
	//��ȡһ������λͼ��face��
	FT_Load_Glyph(pFTFace, FT_Get_Char_Index(pFTFace, word), FT_LOAD_DEFAULT);
	error = FT_Get_Glyph(pFTFace->glyph, &glyph);
	if (error) {
		return -1;
	}
	//  convert glyph to bitmap with 256 gray
	FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
	//���ֲ��뵽ͼƬ�У�ÿ�����м���10�����أ����������Ͻ�x=100,y=100ƫ����
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
	GenUtf8ByChar(("��˼"), pUser, pUserLen);
	if (pUser == NULL) return -1;
	// GetUserName(user, &userLen);
	//����ƫ����������������ʾ
	int bitmap_width_sum = rand() % 2 * 200;
	//forѭ��ʵ��һ����һ���ֲ��뵽ͼƬ��
	for (int k = 0; k < 1; k++) {
		//�����ڴ�飬��wszString�д洢������һ��һ��ȡ���������Ƶ�word�У��ѷ����ȡ����λͼ
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
	FT_Set_Char_Size(pFTFace, 0, 3*64, 500, 500);//���������С
	FT_Vector pen;   
	FT_Matrix matrix; /* transformation matrix */
	double angle = rand();
	angle = angle / 360 * 3.1415926 * 2;
	/* ������������תΪ�ѿ������� */
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
