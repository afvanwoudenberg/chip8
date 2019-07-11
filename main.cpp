// main.cpp
// Aswin van Woudenberg

#include <iostream>
#include <string>
#include <algorithm>
#include <windows.h>
#include <mmsystem.h>

#include "resource.h"
#include "memory.h"
#include "cpu.h"
#include "display.h"
#include "keyboard.h"

#define MAX_LOADSTRING 100

#define ID_MENU_OPEN 9001
#define ID_MENU_RESET 9002
#define ID_MENU_SOUND 9003

using namespace std;

HINSTANCE hInstance;
HWND hMainWnd;
LPCTSTR clsName = "Chip8App";
HICON hIcon;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szOpen[MAX_LOADSTRING];
TCHAR szReset[MAX_LOADSTRING];
TCHAR szSound[MAX_LOADSTRING];
TCHAR szFileError[MAX_LOADSTRING];
TCHAR szFileNotFound[MAX_LOADSTRING];

bool isRunning;
bool hasProgram;
bool soundOn;

HBRUSH hPixelOnBrush;
HBRUSH hPixelOffBrush;

Memory *memory;
Cpu *cpu;
Display *display;
Keyboard *keyboard;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void updateDisplay(Display *d);
void startBeep();
void handleMessages();
bool selectAndOpenFile();
void setWindowTitle(const char *path);

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASSEX wndClsEx;
	DWORD tickTrigger = 0;
	DWORD tickDelay = 17;
	DWORD tickCount = 0;
	
	// Save hInstance
	hInstance = hInst;

	// Load resources
	LoadString(hInstance, IDS_APPNAME, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDS_OPEN, szOpen, MAX_LOADSTRING);
	LoadString(hInstance, IDS_RESET, szReset, MAX_LOADSTRING);
	LoadString(hInstance, IDS_SOUND, szSound, MAX_LOADSTRING);
	LoadString(hInstance, IDS_FILE_ERROR, szFileError, MAX_LOADSTRING);
	LoadString(hInstance, IDS_FILE_NOT_FOUND, szFileNotFound, MAX_LOADSTRING);
	hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));

	// Create the application window
	wndClsEx.cbSize = sizeof(WNDCLASSEX);
	wndClsEx.style = CS_HREDRAW | CS_VREDRAW;
	wndClsEx.lpfnWndProc = WndProc;
	wndClsEx.cbClsExtra = 0;
	wndClsEx.cbWndExtra = 0;
	wndClsEx.hIcon = hIcon;
	wndClsEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClsEx.hbrBackground = NULL;
	wndClsEx.lpszMenuName = NULL;
	wndClsEx.lpszClassName = clsName;
	wndClsEx.hInstance = hInstance;
	wndClsEx.hIconSm = hIcon;

	// Register the application
	RegisterClassEx(&wndClsEx);

	// Calc window dimensions
	int width = 512 + GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
	int height = 256 + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

	// Create the window object
	hMainWnd = CreateWindow(clsName,
		szTitle,
		WS_VISIBLE | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		NULL,
		NULL,
		hInstance,
		NULL);
	
	// Find out if the window was created
	if( !hMainWnd ) // If the window was not created,
		return 0; // stop the application

	// Initialize Chip-8
	memory = new Memory();
	display = new Display();
	keyboard = new Keyboard();
	cpu = new Cpu();
	cpu->setDisplay(display);
	cpu->setMemory(memory);
	cpu->setKeyboard(keyboard);
	display->setUpdateProc(updateDisplay);
	cpu->setStartBeepProc(startBeep);
	cpu->setKeyboardProc(handleMessages);
	isRunning = false;
	hasProgram = false;

	// Display the window to the user
	ShowWindow(hMainWnd, SW_SHOWNORMAL);
	UpdateWindow(hMainWnd);

	// Open file from commandline
	if (strlen(lpCmdLine) > 0)
	{
		// Read the file into memory
		if (memory->loadProgram(lpCmdLine)) {
			setWindowTitle(lpCmdLine);
			isRunning = true;
			hasProgram = true;
		} else {
			MessageBox(hMainWnd, szFileNotFound, szTitle, MB_ICONWARNING);
		}
	} else {
		selectAndOpenFile();
	}

	// Decode and treat the messages
	// as long as the application is running
	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else if (isRunning)
		{
			tickCount = GetTickCount();
			if (tickCount > tickTrigger)
			{
				tickTrigger = tickCount + tickDelay;
				isRunning = cpu->interpret(10);
			}
		}
	}
	
	// Terminate Chip-8
	delete cpu;
	delete display;
	delete memory;
	delete keyboard;

	return msg.wParam;
}

void setWindowTitle(const char *path) 
{
	string fullPath(path);
	string filename;
	string title;
	string windowTitle(szTitle);
	replace(fullPath.begin(), fullPath.end(), '\\', '/');
	string::size_type lastSlashPos=fullPath.find_last_of('/');
	if (lastSlashPos==std::string::npos) {
		filename = fullPath;
	} else {
		filename = fullPath.substr(lastSlashPos + 1, fullPath.size() - lastSlashPos - 1);
	}
	string::size_type lastDotPos=filename.find_last_of('.');
	if (lastDotPos==std::string::npos) {
		title = filename;
	} else {
		title = filename.substr(0,lastDotPos);
	}
	windowTitle.append(" | ").append(title);
	::SetWindowText(hMainWnd, windowTitle.c_str());
}

void startBeep() 
{
	if (soundOn) 
		PlaySound((LPCSTR)IDW_BEEP, hInstance, SND_ASYNC | SND_RESOURCE);
}

void updateDisplay(Display *d)
{
	InvalidateRect(hMainWnd, 0, false);
}

void handleMessages()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) && msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

bool selectAndOpenFile() 
{
	bool result;
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.lpstrFilter = "Chip-8 Files (*.ch8)\0*.ch8\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = "ch8";

	result = false;
	if(GetOpenFileName(&ofn))
	{
		// Read the file into memory
		if (memory->loadProgram(ofn.lpstrFile)) {
			setWindowTitle(ofn.lpstrFile);
			isRunning = true;
			hasProgram = true;
			result = true;
			cpu->reset();
		} else {
			MessageBox(hMainWnd, szFileError, szTitle, MB_ICONWARNING);
		}
	}
	return result;
}

byte mapWindowsToChip8Key(LPARAM lParam)
{
	switch ((lParam >> 16) & 255) 
	{
	case 0x02: { return 0x1; }
	case 0x03: { return 0x2; }
	case 0x04: { return 0x3; }
	case 0x05: { return 0xC; }
	case 0x10: { return 0x4; }
	case 0x11: { return 0x5; }
	case 0x12: { return 0x6; }
	case 0x13: { return 0xD; }
	case 0x1E: { return 0x7; }
	case 0x1F: { return 0x8; }
	case 0x20: { return 0x9; }
	case 0x21: { return 0xE; }
	case 0x2C: { return 0xA; }
	case 0x2D: { return 0x0; }
	case 0x2E: { return 0xB; }
	case 0x2F: { return 0xF; }
	default: { return 0x10; }
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC          hdcMem;
	HBITMAP      hbmMem;
	HANDLE       hOld;

	PAINTSTRUCT  ps;
	HDC          hdc;

    switch(msg)
    {
	case WM_CREATE:
		{
			HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
			// Open menu
			AppendMenu(hSysMenu, MF_STRING, ID_MENU_OPEN, szOpen);
			// Reset menu
			AppendMenu(hSysMenu, MF_STRING, ID_MENU_RESET, szReset);
			// Sound menu
			AppendMenu(hSysMenu, MF_STRING, ID_MENU_SOUND, szSound);
			CheckMenuItem(hSysMenu, ID_MENU_SOUND, MF_CHECKED);
			soundOn = true;

			hPixelOnBrush = CreateSolidBrush(RGB(255,255,255));
			hPixelOffBrush = CreateSolidBrush(RGB(0, 0, 0));
			
			break;
		}
	case WM_ACTIVATE:
		{
			if (wParam != WA_INACTIVE && !::IsIconic(hWnd))
			{ if (hasProgram) isRunning = true; } 
			else
			{ isRunning = false; }
			break;
		}
    case WM_DESTROY:
		{
			DeleteObject(hPixelOnBrush);
			DeleteObject(hPixelOffBrush);
			
			PostQuitMessage(WM_QUIT);
			break;
		}
	case WM_PAINT:
		{
			// Get DC for window
			hdc = BeginPaint(hWnd, &ps);

			// Create an off-screen DC for double-buffering
			hdcMem = CreateCompatibleDC(hdc);
			hbmMem = CreateCompatibleBitmap(hdc, 512, 256);
			hOld   = SelectObject(hdcMem, hbmMem);

			// Draw into hdcMem
			int width = display->getWidth();
			int height = display->getHeight();
			int pixelSize = 512 / display->getWidth();
			
			for (int x=0; x<width; x++)
				for (int y=0; y<height; y++)
				{
					RECT rc;
					rc.left = x * pixelSize;
					rc.top = y * pixelSize;
					rc.right = (x + 1) * pixelSize;
					rc.bottom = (y + 1) * pixelSize;

					if (display->getPixel(x, y)) {
						FillRect(hdcMem, &rc, hPixelOnBrush);
					} else {
						FillRect(hdcMem, &rc, hPixelOffBrush);
					}
				}

			// Transfer the off-screen DC to the screen
			BitBlt(hdc, 0, 0, 512, 256, hdcMem, 0, 0, SRCCOPY);

			// Free-up the off-screen DC
			SelectObject(hdcMem, hOld);
			DeleteObject(hbmMem);
			DeleteDC    (hdcMem);

			EndPaint(hWnd, &ps);
			break;
		}
	case WM_KEYDOWN:
		{
			byte key = mapWindowsToChip8Key(lParam);
			if (key <= 0xF) 
				keyboard->pressKey(key);
			else
				return DefWindowProc(hWnd, msg, wParam, lParam);
			break;
		}
	case WM_KEYUP:
		{
			byte key = mapWindowsToChip8Key(lParam);
			if (key <= 0xF) 
				keyboard->releaseKey(key);
			else
				return DefWindowProc(hWnd, msg, wParam, lParam);
			break;
		}
	case WM_SYSCOMMAND:
		{
			switch(wParam)
			{
			case ID_MENU_OPEN:
				{
					selectAndOpenFile();
					break;
				}
			case ID_MENU_RESET:
				{
					cpu->reset();
					display->clear();
					break;
				}
			case ID_MENU_SOUND:
				{
					HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
					if (soundOn)
					{
						CheckMenuItem(hSysMenu, ID_MENU_SOUND, MF_UNCHECKED);
						soundOn = false;
					} else
					{
						CheckMenuItem(hSysMenu, ID_MENU_SOUND, MF_CHECKED);
						soundOn = true;
					}
					break;
				}
			default:
				{
					return DefWindowProc(hWnd, msg, wParam, lParam);
				}
			}
			break;
		}
    default:
		{
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
    }
    return 0;
}
