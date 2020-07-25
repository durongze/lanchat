#include "stdafx.h"
#include "RgbYuv.h"
#include <stdio.h>
//读BMP
void ReadBmp(unsigned char *RGB, FILE *fp)
{
	int i, j;
	int pix = 3;
	int width = pix * WIDTH;
	int height = HEIGHT;
	unsigned char temp;

	fseek(fp, 54, SEEK_SET);
	fread(RGB + width * height, 1, width * height, fp); // 读取
	for (i = height - 1, j = 0; i >= 0; i--, j++) // 调整顺序
	{
		memcpy(RGB + j * width, RGB + width * height + i * width, width);
	}

	// 顺序调整
	for (i = 0; (unsigned int)i < width * height; i += pix)
	{
		temp = RGB[i];
		RGB[i] = RGB[i + 2];
		RGB[i + 2] = temp;
	}
	return;
}

//将RGB数据流转换为yuv数据流
void Convert(unsigned char *RGB, unsigned char *YUV)
{
	//变量声明
	unsigned int i, x, y, j;
	unsigned char *Y = NULL;
	unsigned char *U = NULL;
	unsigned char *V = NULL;
	int width = WIDTH;
	int height = HEIGHT;
	Y = YUV;
	U = YUV + width * height;
	V = U + ((width * height) >> 2);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++)	{
			j = y * width + x;
			i = j * 3;
			Y[j] = (unsigned char)(DY(RGB[i], RGB[i + 1], RGB[i + 2]));

			if (x % 2 == 1 && y % 2 == 1) {
				j = (width >> 1) * (y >> 1) + (x >> 1);
				//上面i仍有效
				U[j] = (unsigned char)
					((DU(RGB[i], RGB[i + 1], RGB[i + 2]) +
						DU(RGB[i - 3], RGB[i - 2], RGB[i - 1]) +
						DU(RGB[i - width * 3], RGB[i + 1 - width * 3], RGB[i + 2 - width * 3]) +
						DU(RGB[i - 3 - width * 3], RGB[i - 2 - width * 3], RGB[i - 1 - width * 3])) / 4);

				V[j] = (unsigned char)
					((DV(RGB[i], RGB[i + 1], RGB[i + 2]) +
						DV(RGB[i - 3], RGB[i - 2], RGB[i - 1]) +
						DV(RGB[i - width * 3], RGB[i + 1 - width * 3], RGB[i + 2 - width * 3]) +
						DV(RGB[i - 3 - width * 3], RGB[i - 2 - width * 3], RGB[i - 1 - width * 3])) / 4);
			}
		}
	}
	return;
}

int RgbYuvMain()
{
	int w = 1920;
	int h = 1080;
	int i = 1;
	char file[255];
	FILE *fp;
	FILE *fp2;
	unsigned char *YUV = NULL;
	unsigned char *RGB = NULL;
	unsigned char *H264 = NULL;
	long imgSize = w*h;
	long sizeh264buf = 0;
	long counth264buf = 0;

	RGB = (unsigned char*)malloc(imgSize * 6);
	YUV = (unsigned char*)malloc(imgSize + (imgSize >> 1));
	H264 = (unsigned char*)malloc(imgSize * 6);
	sprintf(file, "720bmp.bmp", i);//读取文件
	if ((fp = fopen(file, "rb")) == NULL) {
		return 0;
	}

	ReadBmp(RGB, fp);//将fp文件中数据读入RGB中
	Convert(RGB, YUV);//将RGB数据转换为YUV编码数据
	return 0;
}