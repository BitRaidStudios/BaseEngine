/*
BaseEngine

License
~~~~~~~
BaseEngine (c) 2018 BitRaid Studios
This program comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it.

Credits
~~~~~~~
Kenan Ajkunic
javidx9

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

/* Includes */
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
#include <sstream>

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

	/* Creating the buffer */
	int Window(int width, int height, int fonth, int fontw)
	{
		if (hConsole == INVALID_HANDLE_VALUE)
			return Error(L"Bad Handle");
		nScreenWidth = width;
		nScreenHeight = height;

		rectWindow = { 0, 0, 1, 1 };
		SetConsoleWindowInfo(hConsole, TRUE, &rectWindow);

		COORD coord = { (short)nScreenWidth, (short)nScreenHeight };
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

		wcscpy_s(cfi.FaceName, L"Raster Fonts");

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

	/* Drawing functions */
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

	void ClearScreen()
	{
		Fill(0, 0, ScreenWidth(), ScreenHeight(), L' ', 0);
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
	}

	void DrawString(int x, int y, wstring c, short col = 0x000F)
	{
		for (size_t i = 0; i < c.size(); i++)
		{
			bufScreen[y * nScreenWidth + x + i].Char.UnicodeChar = c[i];
			bufScreen[y * nScreenWidth + x + i].Attributes = col;
		}
	}

	void DrawStringAlpha(int x, int y, wstring c, short col = 0x000F)
	{
		for (size_t i = 0; i < c.size(); i++)
		{
			if (c[i] != L' ')
			{
				bufScreen[y * nScreenWidth + x + i].Char.UnicodeChar = c[i];
				bufScreen[y * nScreenWidth + x + i].Attributes = col;
			}
		}
	}

	void DrawLine(int x1, int y1, int x2, int y2, wchar_t c = 0x2588, short col = 0x000F)
	{
		int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
		dx = x2 - x1;
		dy = y2 - y1;
		dx1 = abs(dx);
		dy1 = abs(dy);
		px = 2 * dy1 - dx1;
		py = 2 * dx1 - dy1;
		if (dy1 <= dx1)
		{
			if (dx >= 0)
			{
				x = x1;
				y = y1;
				xe = x2;
			}
			else
			{
				x = x2;
				y = y2;
				xe = x1;
			}
			Draw(x, y, c, col);
			for (i = 0; x<xe; i++)
			{
				x = x + 1;
				if (px<0)
					px = px + 2 * dy1;
				else
				{
					if ((dx<0 && dy<0) || (dx>0 && dy>0))
						y = y + 1;
					else
						y = y - 1;
					px = px + 2 * (dy1 - dx1);
				}
				Draw(x, y, c, col);
			}
		}
		else
		{
			if (dy >= 0)
			{
				x = x1;
				y = y1;
				ye = y2;
			}
			else
			{
				x = x2;
				y = y2;
				ye = y1;
			}
			Draw(x, y, c, col);
			for (i = 0; y<ye; i++)
			{
				y = y + 1;
				if (py <= 0)
					py = py + 2 * dx1;
				else
				{
					if ((dx<0 && dy<0) || (dx>0 && dy>0))
						x = x + 1;
					else
						x = x - 1;
					py = py + 2 * (dx1 - dy1);
				}
				Draw(x, y, c, col);
			}
		}
	}

	void DrawWireFrameModel(const vector<pair<float, float>> &vecModelCoordinates, float x, float y, float r = 0.0f, float s = 1.0f, short col = FG_WHITE)
	{
		vector<pair<float, float>> vecTransformedCoordinates;
		int verts = vecModelCoordinates.size();
		vecTransformedCoordinates.resize(verts);

		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
			vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
		}

		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
		}

		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
		}

		for (int i = 0; i < verts + 1; i++)
		{
			int j = (i + 1);
			DrawLine((int)vecTransformedCoordinates[i % verts].first, (int)vecTransformedCoordinates[i % verts].second,
				(int)vecTransformedCoordinates[j % verts].first, (int)vecTransformedCoordinates[j % verts].second, PIXEL_SOLID, col);
		}
	}

	void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, COLORREF col)
	{
		DrawLine(x1, y1, x2, y2, col);
		DrawLine(x2, y2, x3, y3, col);
		DrawLine(x3, y3, x1, y1, col);
	}

	// https://www.avrfreaks.net/sites/default/files/triangles.c
	void FillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, COLORREF col)
	{
		auto SWAP = [](int &x, int &y) { int t = x; x = y; y = t; };
		auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) Draw(i, ny, col); };
		int t1x, t2x, y, minx, maxx, t1xp, t2xp;
		bool changed1 = false;
		bool changed2 = false;
		int signx1, signx2, dx1, dy1, dx2, dy2;
		int e1, e2;
		// Sort vertices
		if (y1>y2) { SWAP(y1, y2); SWAP(x1, x2); }
		if (y1>y3) { SWAP(y1, y3); SWAP(x1, x3); }
		if (y2>y3) { SWAP(y2, y3); SWAP(x2, x3); }
		t1x = t2x = x1; y = y1;   // Starting points
		dx1 = (int)(x2 - x1); if (dx1<0) { dx1 = -dx1; signx1 = -1; }
		else signx1 = 1;
		dy1 = (int)(y2 - y1);
		dx2 = (int)(x3 - x1); if (dx2<0) { dx2 = -dx2; signx2 = -1; }
		else signx2 = 1;
		dy2 = (int)(y3 - y1);
		if (dy1 > dx1) {   // swap values
			SWAP(dx1, dy1);
			changed1 = true;
		}
		if (dy2 > dx2) {   // swap values
			SWAP(dy2, dx2);
			changed2 = true;
		}
		e2 = (int)(dx2 >> 1);
		// Flat top, just process the second half
		if (y1 == y2) goto next;
		e1 = (int)(dx1 >> 1);
		for (int i = 0; i < dx1;) {
			t1xp = 0; t2xp = 0;
			if (t1x<t2x) { minx = t1x; maxx = t2x; }
			else { minx = t2x; maxx = t1x; }
			// process first line until y value is about to change
			while (i<dx1) {
				i++;
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) t1xp = signx1;//t1x += signx1;
					else          goto next1;
				}
				if (changed1) break;
				else t1x += signx1;
			}
			// Move line
		next1:
			// process second line until y value is about to change
			while (1) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) t2xp = signx2;//t2x += signx2;
					else          goto next2;
				}
				if (changed2)     break;
				else              t2x += signx2;
			}
		next2:
			if (minx>t1x) minx = t1x; if (minx>t2x) minx = t2x;
			if (maxx<t1x) maxx = t1x; if (maxx<t2x) maxx = t2x;
			drawline(minx, maxx, y);    // Draw line from min to max points found on the y
										// Now increase y
			if (!changed1) t1x += signx1;
			t1x += t1xp;
			if (!changed2) t2x += signx2;
			t2x += t2xp;
			y += 1;
			if (y == y2) break;
		}
	next:
		// Second half
		dx1 = (int)(x3 - x2); if (dx1<0) { dx1 = -dx1; signx1 = -1; }
		else signx1 = 1;
		dy1 = (int)(y3 - y2);
		t1x = x2;
		if (dy1 > dx1) {   // swap values
			SWAP(dy1, dx1);
			changed1 = true;
		}
		else changed1 = false;
		e1 = (int)(dx1 >> 1);
		for (int i = 0; i <= dx1; i++) {
			t1xp = 0; t2xp = 0;
			if (t1x<t2x) { minx = t1x; maxx = t2x; }
			else { minx = t2x; maxx = t1x; }
			// process first line until y value is about to change
			while (i<dx1) {
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) { t1xp = signx1; break; }//t1x += signx1;
					else          goto next3;
				}
				if (changed1) break;
				else   	   	  t1x += signx1;
				if (i<dx1) i++;
			}
		next3:
			// process second line until y value is about to change
			while (t2x != x3) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) t2xp = signx2;
					else          goto next4;
				}
				if (changed2)     break;
				else              t2x += signx2;
			}
		next4:
			if (minx>t1x) minx = t1x; if (minx>t2x) minx = t2x;
			if (maxx<t1x) maxx = t1x; if (maxx<t2x) maxx = t2x;
			drawline(minx, maxx, y);
			if (!changed1) t1x += signx1;
			t1x += t1xp;
			if (!changed2) t2x += signx2;
			t2x += t2xp;
			y += 1;
			if (y>y3) return;
		}
	}

	void Clip(int &x, int &y)
	{
		if (x < 0) x = 0;
		if (x >= nScreenWidth) x = nScreenWidth;
		if (y < 0) y = 0;
		if (y >= nScreenHeight) y = nScreenHeight;
	}

	void GetScreenResolution(int& h, int& v)
	{
		RECT screen;
		const HWND hScreen = GetDesktopWindow();

		GetWindowRect(hScreen, &screen);

		h = screen.right;
		v = screen.bottom;
	}

	/* Logic */
	// This will make your life easier
	int Clamp(int val, int claMin, int claMax)
	{
		if (val >= claMax)
			return val = claMax;
		else if (val <= claMin)
			return val = claMin;
	}

	int Wrap(int val, int wraMin, int wraMax)
	{
		if (val >= wraMax)
			return val = wraMin;
		else if (val <= wraMin)
			return val = wraMax;
	}

	void saveToTxt(string value)
	{

	}

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

	void Map()
	{
		wstring map;
		ifstream iMap("levels/level.txt");
		string sMap;
		for (int m = 0; m < 32; m++)
		{

		}
	}

private:
	void GameThread()
	{
		if (!OnUserCreate())
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
				WriteConsoleOutput(hConsole, bufScreen, { (short)nScreenWidth, (short)nScreenHeight }, { 0, 0 }, &rectWindow);
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
	sKeyState GetKey(int nKeyID) { return keys[nKeyID]; }
	bool IsFocused() { return bConsoleInFocus; }

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
	short keyOldState[256] = { 0 };
	short keyNewState[256] = { 0 };
};

atomic<bool> BaseEngine::bAtomActive(false);
condition_variable BaseEngine::cvGameFinished;
mutex BaseEngine::muxGame;
