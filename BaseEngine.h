/*
	BaseEngine

	License
	~~~~~~~
	BaseEngine (c) 2018 BitRaid Studios
	This program comes with ABSOLUTELY NO WARRANTY.
	This is free software, and you are welcome to redistribute it.

	Credits
	~~~~~~~
	javidx9 - The 'base' (get it) code of the engine

	Author
	~~~~~~
	Twitter: @BitRaid_Studios
	Website: bitraidstudios.tk
	GitHub: BitRaidStudios
*/

#ifndef UNICODE
#error Please enable UNICODE for your compiler!
#define UNICODE
#define _UNICODE
#endif

#pragma once

#include <windows.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <fstream>
#include <string>

using namespace std;

enum COLORS
{
	FG_BLACK = 0x0000,
	FG_DARK_BLUE = 0x0001,
	FG_DARK_GREEN = 0x0002,
	FG_DARK_CYAN = 0x0003,
	FG_DARK_RED = 0x0004,
	FG_DARK_MAGENTA = 0x0005,
	FG_DARK_YELLOW = 0x0006,
	FG_GREY = 0x0007,
	FG_DARK_GREY = 0x0008,
	FG_BLUE = 0x0009,
	FG_GREEN = 0x000A,
	FG_CYAN = 0x000B,
	FG_RED = 0x000C,
	FG_MAGENTA = 0x000D,
	FG_YELLOW = 0x000E,
	FG_WHITE = 0x000F,
	FG_BROWN = 0xA52A,
	BG_BLACK = 0x0000,
	BG_DARK_BLUE = 0x0010,
	BG_DARK_GREEN = 0x0020,
	BG_DARK_CYAN = 0x0030,
	BG_DARK_RED = 0x0040,
	BG_DARK_MAGENTA = 0x0050,
	BG_DARK_YELLOW = 0x0060,
	BG_GREY = 0x0070,
	BG_DARK_GREY = 0x0080,
	BG_BLUE = 0x0090,
	BG_GREEN = 0x00A0,
	BG_CYAN = 0x00B0,
	BG_RED = 0x00C0,
	BG_MAGENTA = 0x00D0,
	BG_YELLOW = 0x00E0,
	BG_WHITE = 0x00F0,
};

enum PIXEL_TYPE
{
	PIXEL_SOLID = 0x2588,
	PIXEL_THREEQUARTERS = 0x2593,
	PIXEL_HALF = 0x2592,
	PIXEL_QUARTER = 0x2591,
};

class BaseEngine
{
public:
	BaseEngine()
	{
		nScreenWidth = 160;
		nScreenHeight = 100;

		memset(keyNewState, 0, 256 * sizeof(short));
		memset(keyOldState, 0, 256 * sizeof(short));
		memset(keys, 0, 256 * sizeof(sKeyState));

		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);

		sAppName = L"untitled";
	}

	int Window(int width, int height, int fonth, int fontw)
	{
		if (hConsole == INVALID_HANDLE_VALUE)
			return Error(L"Bad Handle");
		nScreenWidth = width;
		nScreenHeight = height;
		
		rectWindow = {0, 0, 1, 1};
		SetConsoleWindowInfo(hConsole, TRUE, &rectWindow);
		
		COORD coord = {(short)nScreenWidth, (short)nScreenHeight};
		if (!SetConsoleScreenBufferSize(hConsole, coord))
			Error(L"SetConsoleScreenBufferSize");
		
		if (!SetConsoleActiveScreenBuffer(hConsole))
			return Error(L"SetConsoleActiveScreenBuffer");

		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize = sizeof(cfi);
		cfi.nFont = 0;
		cfi.dwFontSize.X = fontw;
		cfi.dwFontSize.Y = fonth;
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;
		
		wcscpy_s(cfi.FaceName, L"Lucida Console");

		if (!SetCurrentConsoleFontEx(hConsole, false, &cfi))
			return Error(L"SetCurrentConsoleFontEx");

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
			return Error(L"GetConsoleScreenBufferInfo");
		if (nScreenHeight > csbi.dwMaximumWindowSize.Y)
			return Error(L"Screen Height / Font Height Too Big");
		if (nScreenWidth > csbi.dwMaximumWindowSize.X)
			return Error(L"Screen Width / Font Width Too Big");

		rectWindow = { 0, 0, (short)nScreenWidth - 1, (short)nScreenHeight - 1 };
		if (!SetConsoleWindowInfo(hConsole, TRUE, &rectWindow))
			return Error(L"SetConsoleWindowInfo");

		bufScreen = new CHAR_INFO[nScreenWidth * nScreenHeight];
		memset(bufScreen, 0, sizeof(CHAR_INFO) * nScreenWidth * nScreenHeight);

		SetConsoleCtrlHandler((PHANDLER_ROUTINE)CloseHandler, TRUE);
		return 1;
	}

	virtual void Draw(int x, int y, wchar_t c = 0x2588, short col = 0x000F)
	{
		if (x >= 0 && x < nScreenWidth && y >= 0 && y < nScreenHeight)
		{
			bufScreen[y * nScreenWidth + x].Char.UnicodeChar = c;
			bufScreen[y * nScreenWidth + x].Attributes = col;
		}
	}

	void Fill(int x1, int y1, int x2, int y2, wchar_t c = 0x2588, short col = 0x000F)
	{
		Clip(x1, y1);
		Clip(x2, y2);
		for (int x = x1; x < x2; x++)
			for (int y = y1; y < y2; y++)
				Draw(x, y, c, col);
	}

	void Clip(int &x, int &y)
	{
		if (x < 0) x = 0;
		if (x >= nScreenWidth) x = nScreenWidth;
		if (y < 0) y = 0;
		if (y >= nScreenHeight) y = nScreenHeight;
	}

	void DrawCircle(int xc, int yc, int r, wchar_t c = 0x2588, short col = 0x000F)
	{
		int x = 0;
		int y = r;
		int p = 3 - 2 * r;
		if (!r) return;

		while (y >= x)
		{
			Draw(xc - x, yc - y, c, col);
			Draw(xc - y, yc - x, c, col);
			Draw(xc + y, yc - x, c, col);
			Draw(xc + x, yc - y, c, col);
			Draw(xc - x, yc + y, c, col);
			Draw(xc - y, yc + x, c, col);
			Draw(xc + y, yc + x, c, col);
			Draw(xc + x, yc + y, c, col);
			if (p < 0) p += 4 * x++ + 6;
			else p += 4 * (x++ - y--) + 10;
		}
	}

	void FillCircle(int xc, int yc, int r, wchar_t c = 0x2588, short col = 0x000F)
	{
		int x = 0;
		int y = r;
		int p = 3 - 2 * r;
		if (!r) return;

		auto drawline = [&](int sx, int ex, int ny)
		{
			for (int i = sx; i <= ex; i++)
				Draw(i, ny, c, col);
		};

		while (y >= x)
		{
			drawline(xc - x, xc + x, yc - y);
			drawline(xc - y, xc + y, yc - x);
			drawline(xc - x, xc + x, yc + y);
			drawline(xc - y, xc + y, yc + x);
			if (p < 0) p += 4 * x++ + 6;
			else p += 4 * (x++ - y--) + 10;
		}
	};

	~BaseEngine()
	{
		SetConsoleActiveScreenBuffer(hOriginalConsole);
		delete[] bufScreen;
	}

public:
	void Start()
	{
		bAtomActive = true;

		thread t = thread(&BaseEngine::GameThread, this);

		t.join();
	}

	int ScreenWidth()
	{
		return nScreenWidth;
	}

	int ScreenHeight()
	{
		return nScreenHeight;
	}

	/*void Map()
	{
		ifstream iMap("levels/level.txt");
		string sMap;
		while (getline(iMap, sMap))
		{
			map += string(sMap);
			map.push_back(L'\n');
		}
	}*/

private:
	void GameThread()
	{
		if(!OnUserCreate())
			bAtomActive = false;

		auto tp1 = chrono::system_clock::now();
		auto tp2 = chrono::system_clock::now();

		while (bAtomActive)
		{
			while (bAtomActive)
			{
				tp2 = chrono::system_clock::now();
				chrono::duration<float> elapsedTime = tp2 - tp1;
				tp1 = tp2;
				float fElapsedTime = elapsedTime.count();

				for (int i = 0; i < 256; i++)
				{
					keyNewState[i] = GetAsyncKeyState(i);

					keys[i].bPressed = false;
					keys[i].bReleased = false;

					if (keyNewState[i] != keyOldState[i])
					{
						if (keyNewState[i] & 0x8000)
						{
							keys[i].bPressed = !keys[i].bHeld;
							keys[i].bHeld = true;
						}
						else
						{
							keys[i].bReleased = true;
							keys[i].bHeld = false;
						}
					}
					keyOldState[i] = keyNewState[i];
				}
				
				if (!OnUserUpdate(fElapsedTime))
					bAtomActive = false;

				wchar_t s[256];
				swprintf_s(s, 256, L"BaseEngine - %s - FPS: %3.2f ", sAppName.c_str(), 1.0f / fElapsedTime);
				SetConsoleTitle(s);
				WriteConsoleOutput(hConsole, bufScreen, {(short)nScreenWidth, (short)nScreenHeight}, {0, 0}, &rectWindow);
			}

			if (OnUserDestroy())
			{
				delete[] bufScreen;
				SetConsoleActiveScreenBuffer(hOriginalConsole);
				cvGameFinished.notify_one();
			}
			else
			{
				bAtomActive = true;
			}
		}
	}

public:
	virtual bool OnUserCreate() = 0;
	virtual bool OnUserUpdate(float fElapsedTime) = 0;

	virtual bool OnUserDestroy()
	{
		return true;
	}

protected:
	struct sKeyState
	{
		bool bPressed;
		bool bReleased;
		bool bHeld;
	} keys[256];

public:
	sKeyState GetKey(int nKeyID) {return keys[nKeyID];}
	bool IsFocused() {return bConsoleInFocus;}

protected:
	int Error(const wchar_t *msg)
	{
		wchar_t buf[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
		SetConsoleActiveScreenBuffer(hOriginalConsole);
		wprintf(L"ERROR: %s\n\t%s\n", msg, buf);
		return 0;
	}

	static BOOL CloseHandler(DWORD evt)
	{
		if (evt == CTRL_CLOSE_EVENT)
		{
			bAtomActive = false;

			unique_lock<mutex> ul(muxGame);
			cvGameFinished.wait(ul);
		}
		return true;
	}
	
protected:
	int nScreenWidth;
	int nScreenHeight;
	HANDLE hConsole;
	HANDLE hConsoleIn;
	HANDLE hOriginalConsole;
	wstring sAppName;
	SMALL_RECT rectWindow;
	CHAR_INFO *bufScreen;
	static atomic<bool> bAtomActive;
	static mutex muxGame;
	static condition_variable cvGameFinished;
	bool bConsoleInFocus = true;
	wstring map;
	short keyOldState[256] = {0};
	short keyNewState[256] = {0};
};

atomic<bool> BaseEngine::bAtomActive(false);
condition_variable BaseEngine::cvGameFinished;
mutex BaseEngine::muxGame;
