/* main.c */
#include "global.h"
#include <commctrl.h>
#include <stdlib.h>
#include "game.h"
#include "draw.h"
#include "input.h"
#include "resource.h"

/* ��������� ��� ����������� ���������� */
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"comctl32.lib")

/* ��� ������ ���� */
static const TCHAR szWindowClassName[] = _T("BATTY");
static const TCHAR szWindowTitle[] = _T("Batty");

/* ���������� ���������� */
HINSTANCE ghInst;                  /* ����� ���������� */
HWND ghWndMain;                    /* ����� ���� */
int giDebug;                       /* ���� ������� */

/* ��������� ���� 'status bar' */
#define STB_PRIMARY             0
#define STB_RUN                 1

/* ����� 'status bar' */
static HWND hStatusBar;
/* ������ ������ */
static HICON hIconRun;
static HICON hIconStop;

/* ���������, ��� ���� � ������ */
static BOOL bActive;

/* ���������� ������ ������� ���������� � ��������� ���� */
static void SetStatusRun(BOOL flags)
{
	HICON hIcon;

	hIcon = flags ? hIconRun : hIconStop;
	SendMessage(hStatusBar,SB_SETICON,STB_RUN,(LPARAM)hIcon);
}

/* ������� ���� */
static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rc;

	switch (msg) {
		case WM_CREATE:
			/* ��������� ���� */
			break;

		case WM_PAINT:
			/* �������� ���� */
			hdc = BeginPaint(hWnd,&ps);
			if (!bActive) {
				GetClientRect(hWnd,&rc);
				BitBlt(hdc,0,0,rc.right,rc.bottom,ghMemDC,0,0,SRCCOPY);
			}
			EndPaint(hWnd,&ps);
			SetStatusRun(bActive);
			break;

		case WM_ACTIVATEAPP:
			/* ���� �������� ��� ������ ����� */
			bActive = (BOOL)wParam;
			/* �������� ��� ��������� ���������� ����� */
			AcquireInput(bActive);
			SetStatusRun(bActive);
			break;

		case WM_DESTROY:
			/* ��������� ������ */
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
	}

	return 0;
}

/* �������� ������ ������������ ������� */
static int VersionCheck(void)
{
	OSVERSIONINFO os;

	/* ���������� ��������� � �������� �� */
	memset(&os,0,sizeof(os));
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&os))
		return 1;

	/* ��������� ��������� �������� */
	return (os.dwPlatformId == VER_PLATFORM_WIN32_NT) ? 0 : 1;
}

/* �������������� ���� ���� � ��������� ��� */
static int InitializeApp(HINSTANCE hInst, int nCmdShow)
{
	DWORD dwWindowStyle;
	WNDCLASSEX wc;
	RECT rc;
	int nBorders[2];

	/* �������� ����� ���������� */
	ghInst = hInst;

	/* �������� ��������� ������ ���� */
	memset(&wc,0,sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON1));
	wc.hIconSm = LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(RGB(0,0,60));
	wc.lpszClassName = szWindowClassName;

	/* �������������� ����� ���� */
	if (!RegisterClassEx(&wc))
		return 1;

	/* �������� ���� */
	dwWindowStyle = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_VISIBLE;
	SetRect(&rc,0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
	AdjustWindowRect(&rc,dwWindowStyle,FALSE);
	ghWndMain = CreateWindowEx(0,szWindowClassName,szWindowTitle,dwWindowStyle,
		CW_USEDEFAULT,CW_USEDEFAULT,(rc.right - rc.left),(rc.bottom - rc.top)+20,
		NULL,NULL,hInst,NULL);
	if (!ghWndMain)
		return 1;

	/* �������� ������ */
	hIconRun = LoadImage(hInst,MAKEINTRESOURCE(IDI_ICON2),IMAGE_ICON,0,0,0);
	if (!hIconRun)
		return 1;
	hIconStop = LoadImage(hInst,MAKEINTRESOURCE(IDI_ICON3),IMAGE_ICON,0,0,0);
	if (!hIconStop)
		return 1;

	/* �������� 'status bar' */
	InitCommonControls();
	hStatusBar = CreateStatusWindow(SBT_TOOLTIPS|WS_CHILD|WS_VISIBLE,NULL,ghWndMain,0);
	if (!hStatusBar)
		return 1;
	/* �������� ������ � ���� ������� */
	nBorders[0] = SCREEN_WIDTH - 22;
	nBorders[1] = -1;
	SendMessage(hStatusBar,SB_SETPARTS,2,(LPARAM)nBorders);
	/* ��������� ����������� ���� */
	SetStatusRun(TRUE);

	/* ��������� ���� */
	ShowWindow(ghWndMain,nCmdShow);
	UpdateWindow(ghWndMain);

	return 0;
}

/* ����� ����� � ��������� */
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PTSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	/* �������� ������ ������������ ������� */
	if (VersionCheck()) {
		MessageBox(NULL,_T("Error check Windows."),
			_T("Error"),MB_OK|MB_ICONSTOP);
		return 1;
	}

	/* �������� ���� ���������� ���������� */
//	if (!_tcscmp(lpCmdLine,_T("--debug")))
		giDebug = 1;

	/* �������� � ��������� ���� */
	if (InitializeApp(hInst,nCmdShow)) {
		MessageBox(NULL,_T("Error start game."),
			_T("Error"),MB_OK|MB_ICONSTOP);
		return 1;
	}

	/* ������������� ���� */
	if (InitGame()) {
		ShutdownGame();
		ShowWindow(ghWndMain,SW_HIDE);
		MessageBox(ghWndMain,_T("Error run game."),
			_T("Error"),MB_OK|MB_ICONSTOP);
		return 1;
	}

	/* ������� ���� ��������� */
	for(;;) {
		/* ���� ���������� � ������ */
		if (bActive) {
			/* ���� ���� ���������, �� ���������� ��� */
			if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
				/* �������� �� ���������� ��������� */
				if (msg.message == WM_QUIT)
					break;
				/* �������� ��������� �� ��������� */
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				/* �������� ������� ��������� */
				continue;
			}
			/* ������� ���������� ���� */
			MainGame();
		} else {
			BOOL bRet;

			/* ������� � ���������� ��������� */
			bRet = GetMessage(&msg,NULL,0,0);
			if (!bRet || (bRet == -1))
				break;
			/* �������� ��������� �� ��������� */
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	}

	/* ��������������� ���� */
	ShutdownGame();

	return msg.wParam;
}
