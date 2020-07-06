#include "stdafx.h"
#include "WaveApi.h"

#pragma comment(lib, "winmm.lib")

static unsigned char buffer[BUFFER_SIZE] = { 0 };
static int buf_count = 0;
static WAVEFORMATEX wavform = { WAVE_FORMAT_PCM, 2, 44100, ESTIMATION, 4, 16, 0 };

// 录音回调函数
void CALLBACK waveInProc(HWAVEIN hwi,
	UINT uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2)
{
	LPWAVEHDR pwh = (LPWAVEHDR)dwParam1;

	if ((WIM_DATA == uMsg) && (buf_count<BUFFER_SIZE))
	{
		int temp = BUFFER_SIZE - buf_count;
		temp = (temp>pwh->dwBytesRecorded) ? pwh->dwBytesRecorded : temp;
		memcpy(buffer + buf_count, pwh->lpData, temp);
		buf_count += temp;

		waveInAddBuffer(hwi, pwh, sizeof(WAVEHDR));
	}
}

// 放音回调函数
void CALLBACK waveOutProc(HWAVEOUT hwo,
	UINT uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2)
{
	if (WOM_DONE == uMsg)
	{
		buf_count = BUFFER_SIZE;
	}
}

WaveInApi::WaveInApi()
{
}


WaveInApi::~WaveInApi()
{
}

int WaveInApi::Init()
{
	// Device
	WAVEINCAPS wic;
	int nReturn = waveInGetNumDevs();
	for (int i = 0; i < nReturn; i++)
	{
		memset(&wic, 0, sizeof(wic));
		waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS));
		printf("#%d\t设备名：%s\n", i, wic.szPname);
	}

	waveInOpen(&hWaveIn, WAVE_MAPPER, &wavform, (DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION);
	waveInGetDevCaps((UINT_PTR)hWaveIn, &wic, sizeof(WAVEINCAPS));
	for (int i = 0; i < FRAGMENT_NUM; i++)
	{
		wh[i].lpData = new char[FRAGMENT_SIZE];
		wh[i].dwBufferLength = FRAGMENT_SIZE;
		wh[i].dwBytesRecorded = 0;
		wh[i].dwUser = NULL;
		wh[i].dwFlags = 0;
		wh[i].dwLoops = 1;
		wh[i].lpNext = NULL;
		wh[i].reserved = 0;
		waveInPrepareHeader(hWaveIn, &wh[i], sizeof(WAVEHDR));
		waveInAddBuffer(hWaveIn, &wh[i], sizeof(WAVEHDR));
	}
	waveInStart(hWaveIn);
	return 0;
}

int WaveInApi::Uninit()
{
	waveInStop(hWaveIn);
	waveInReset(hWaveIn);
	for (int i = 0; i < FRAGMENT_NUM; i++)
	{
		waveInUnprepareHeader(hWaveIn, &wh[i], sizeof(WAVEHDR));
		delete wh[i].lpData;
	}
	waveInClose(hWaveIn);
	return 0;
}
// 入口
int WaveInApi::Record()
{
	buf_count = 0;
	while (buf_count < BUFFER_SIZE)
	{
		Sleep(1);
	}
	return 0;
}

WaveOutApi::WaveOutApi()
{
}


WaveOutApi::~WaveOutApi()
{
}

int WaveOutApi::Init()
{
	// Device
	WAVEOUTCAPS woc;
	UINT nReturn = waveOutGetNumDevs();
	for (int i = 0; i<nReturn; i++)
	{
		memset(&woc, 0, sizeof(woc));
		waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS));
		printf("#%d\t设备名：%s\n", i, woc.szPname);
	}

	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wavform, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
	waveOutGetDevCaps((UINT_PTR)hWaveOut, &woc, sizeof(WAVEOUTCAPS));
	wavhdr.lpData = (LPSTR)buffer;
	wavhdr.dwBufferLength = BUFFER_SIZE;
	wavhdr.dwFlags = 0;
	wavhdr.dwLoops = 0;
	waveOutPrepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));

	return 0;
}

int WaveOutApi::Uninit()
{
	// clean
	waveOutReset(hWaveOut);
	waveOutUnprepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
	return 0;
}



int WaveOutApi::Play()
{
	buf_count = 0;
	waveOutWrite(hWaveOut, &wavhdr, sizeof(WAVEHDR));
	while (buf_count < BUFFER_SIZE)
	{
		Sleep(1);
	}

	return 0;
}

