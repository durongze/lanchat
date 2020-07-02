#ifndef __WAVE_API__
#define __WAVE_API__

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>

#define ESTIMATION  (44100 * 16 * 2 / 8)
#define BUFFER_SIZE (ESTIMATION * 5)	// 录制声音长度
#define FRAGMENT_SIZE 1024				// 缓存区大小
#define FRAGMENT_NUM 4					// 缓存区个数

class WaveInApi
{
	HWAVEIN hWaveIn;
	WAVEHDR wh[FRAGMENT_NUM];
public:
	WaveInApi();
	~WaveInApi();
	int Init();
	int Uninit();
	int Record();
};

class WaveOutApi
{
	HWAVEOUT hWaveOut;
	WAVEHDR wavhdr;
public:
	WaveOutApi();
	~WaveOutApi();
	int Init();
	int Uninit();

	int Play();
};

#endif

