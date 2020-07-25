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

int PrintBitMap(FT_Bitmap &bitmap)
{
	FILE* fp = fopen("bitmap.txt", "w+");
	for (int i = 0; i < bitmap.rows; ++i)
	{
		for (int j = 0; j < bitmap.width; ++j)
		{
			//  if it has gray>0 we set show it as 1, o otherwise
			fprintf(fp, " %d ", bitmap.buffer[i * bitmap.width + j] ? 1 : 0);
		}
		fprintf(fp, " \n ");
	}
	fclose(fp);
	return 0;
}

int WriteWord(WORD word, int& bitmapStart, FT_Face& pFTFace,
	BITMAPINFOHEADER& strInfo, IMAGEDATA*& arrayColor)
{
	FT_Error error = 0;
	FT_Glyph glyph;
	//读取一个字体位图到face中
	FT_UInt idx = FT_Get_Char_Index(pFTFace, word);
	error = FT_Load_Glyph(pFTFace, idx, FT_LOAD_DEFAULT);
	error = FT_Get_Glyph(pFTFace->glyph, &glyph);
	if (error) {
		return -1;
	}
	//  convert glyph to bitmap with 256 gray
	FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
	//把字插入到图片中，每个字中间间隔10个像素，并且离左上角x=100,y=100偏移量
	DrawWord(bitmapStart, bitmap_glyph->bitmap, strInfo, arrayColor);
	// PrintBitMap(bitmap_glyph->bitmap);
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

float int26p6_to_float(long v)
{
	return (float)v;
}

HDC g_hdc = NULL;

HDC GetHdc()
{
	return g_hdc;
}

void SetHdc(HDC hdc)
{
	g_hdc = hdc;
}

POINT FtVectorToPoint(FT_Vector ftv)
{
	POINT p = { ftv.x, ftv.y };
	return p;
}

int PaintBezier(HDC hdc, POINT apt[])
{
	//调用系统的绘制贝塞尔函数
	PolyBezier(hdc, apt, 4);
	MoveToEx(hdc, apt[0].x, apt[0].y, NULL);
	LineTo(hdc, apt[1].x, apt[1].y);
	MoveToEx(hdc, apt[2].x, apt[2].y, NULL);
	LineTo(hdc, apt[3].x, apt[3].y);
	return 0;
}

int PaintLine(FT_Vector start, FT_Vector end)
{
	HDC hdc = GetHdc();
	if (hdc == NULL) {
		return 0;
	}
	// QPainter painter;
	// QPen pen(RandColor());
	HPEN pen = CreatePen(PS_DASH, 0, RGB(155, 150, 0));
	float startX = int26p6_to_float(start.x);
	float startY = -int26p6_to_float(start.y);
	float fEndX = int26p6_to_float(end.x);
	float fEndY = -int26p6_to_float(end.y);

	// painter.setPen(pen);
	// painter.drawLine(startX, startY, fEndX, fEndY);

	SelectObject(GetHdc(), pen);
	MoveToEx(hdc, start.x, start.y, NULL);
	LineTo(hdc, end.x, end.y);
	return 0;
}

int PaintQuadPath(FT_Vector start, FT_Vector vx1, FT_Vector vx2)
{
	HDC hdc = GetHdc();
	if (hdc == NULL) {
		return 0;
	}
	//QPainter painter;
	//QPen pen(RandColor());
	//QPainterPath path;
	POINT apt[3];
	HPEN pen = CreatePen(PS_DASH, 0, RGB(0, 0, 255));
	float startX = int26p6_to_float(start.x);
	float startY = -int26p6_to_float(start.y);
	float x1 = int26p6_to_float(vx1.x);
	float y1 = -int26p6_to_float(vx1.y);
	float x2 = int26p6_to_float(vx2.x);
	float y2 = -int26p6_to_float(vx2.y);

	// painter.setPen(pen);
	// path.moveTo(startX, startY);
	// path.quadTo(x1, y1, x2, y2);
	// painter.drawPath(path);
	apt[0] = FtVectorToPoint(start);
	apt[1] = FtVectorToPoint(vx1);
	apt[2] = FtVectorToPoint(vx2);
	SelectObject(hdc, pen);
	PolyBezier(hdc, apt, 3);
	return 0;
}

int PaintCubicPath(FT_Vector start, FT_Vector vx1, FT_Vector vx2, FT_Vector vx3)
{
	HDC hdc = GetHdc();
	if (hdc == NULL) {
		return 0;
	}
	// QPainter painter;
	// QPen pen(RandColor());
	// QPainterPath path;
	POINT apt[4];
	HPEN pen = CreatePen(PS_DASH, 0, RGB(0, 255, 0));
	float startX = int26p6_to_float(start.x);
	float startY = -int26p6_to_float(start.y);
	float x1 = int26p6_to_float(vx1.x);
	float y1 = -int26p6_to_float(vx1.y);
	float x2 = int26p6_to_float(vx2.x);
	float y2 = -int26p6_to_float(vx2.y);
	float x3 = int26p6_to_float(vx3.x);
	float y3 = -int26p6_to_float(vx3.y);

	//painter.setPen(pen);
	//path.moveTo(startX, startY);
	//path.cubicTo(x1, y1, x2, y2, x3, y3);
	//painter.drawPath(path);
	apt[0] = FtVectorToPoint(start);
	apt[1] = FtVectorToPoint(vx1);
	apt[2] = FtVectorToPoint(vx2);
	apt[3] = FtVectorToPoint(vx3);
	SelectObject(hdc, pen);
	PolyBezier(hdc, apt, 4);
	return 0;
}

int PaintConic(FT_Vector *&point, FT_Vector* limit, FT_Vector& v_s,  FT_Vector v_e, char*& tags)
{
	FT_Vector v_control;
	v_control.x = point->x;
	v_control.y = point->y;
	while (point < limit)
	{
		FT_Vector vec;
		FT_Vector v_middle;
		point++;
		tags++;
		char tag = FT_CURVE_TAG(tags[0]);
		vec.x = point->x;
		vec.y = point->y;
		if (tag == FT_CURVE_TAG_ON)	{
			PaintQuadPath(v_s, v_control, vec);
			v_s = vec;
			return 0;
		}
		if (tag != FT_CURVE_TAG_CONIC) {
			return -1;
		}
		v_middle.x = (v_control.x + vec.x) / 2;
		v_middle.y = (v_control.y + vec.y) / 2;
		PaintQuadPath(v_s, v_control, v_middle);
		v_s = v_middle;
		v_control = vec;
	}
	PaintQuadPath(v_s, v_control, v_e);
	v_s = v_e;
	return -2;
}

int PaintCubic(FT_Vector *&point, FT_Vector* limit, FT_Vector& v_s, FT_Vector v_e, char*& tags)
{
	FT_Vector vec1, vec2;
	if (point + 1 > limit || FT_CURVE_TAG(tags[1]) != FT_CURVE_TAG_CUBIC)
	{
		return - 1;
	}
	vec1.x = point[0].x;
	vec1.y = point[0].y;
	vec2.x = point[1].x;
	vec2.y = point[1].y;
	point += 2;
	tags += 2;
	if (point <= limit)
	{
		PaintCubicPath(v_s, vec1, vec2, *point);
		v_s = *point;
		return 0;
	}
	PaintCubicPath(v_s, vec1, vec2, v_e);
	v_s = v_e;
	return -2;
}


int PaintWord(WORD word, FT_Face& face)
{
	FT_Error error = 0;
	FT_Glyph glyph;
	//读取一个字体位图到face中
	FT_UInt idx = FT_Get_Char_Index(face, word);
	error = FT_Load_Glyph(face, idx, FT_LOAD_DEFAULT);
	// error = FT_Get_Glyph(face->glyph, &glyph);
	FT_GlyphSlot pGlyphSlot = face->glyph;
	FT_Outline* outline = &pGlyphSlot->outline;
	//QPainter painter(this);
	//painter.translate(400, 400);
	FT_Vector* point;
	FT_Vector* limit;
	char* tags;
	FT_Vector  v_last;
	FT_Vector  v_control;
	FT_Vector  v_start;
	int ret;
	int first = 0;
	for (int n = 0; n < outline->n_contours; n++)
	{
		int  last = outline->contours[n];
		limit = outline->points + last;
		v_start = outline->points[first];
		v_last = outline->points[last];
		v_control = v_start;
		point = outline->points + first;
		tags = outline->tags + first;
		char tag = FT_CURVE_TAG(tags[0]);
		FT_Vector v_e = v_control;
		FT_Vector  v_s = v_control;
		while (point < limit)
		{
			point++;
			tags++;
			tag = FT_CURVE_TAG(tags[0]);
			switch (tag)
			{
				case FT_CURVE_TAG_ON:
					PaintLine(v_s, *point);
					v_s = *point;
					break;
				case FT_CURVE_TAG_CONIC:  //二次Bezier曲线
					ret = PaintConic(point, limit, v_s, v_start, tags);
					if (ret == -1) {
						return -1;
					}
					else if (ret == -2) {
						goto Close;
					}
					break;
				default:  // FT_CURVE_TAG_CUBIC 三次Bezier曲线
					ret = PaintCubic(point, limit, v_s, v_start, tags);
					if (ret == -1) {
						return - 1;
					}
					else if (ret == -2) {
						goto Close;
					}
					break;
			}
		}
	Close:
		PaintLine(v_s, v_e);
		first = last + 1;
	}
	return 0;
}