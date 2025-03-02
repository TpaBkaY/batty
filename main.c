/* main.c */
#include "global.h"
#include <commctrl.h>
#include <stdlib.h>
#include "game.h"
#include "draw.h"
#include "input.h"
#include "resource.h"

/* подключим все необходимые библиотеки */
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"comctl32.lib")

/* имя класса окна */
static const TCHAR szWindowClassName[] = _T("BATTY");
static const TCHAR szWindowTitle[] = _T("Batty");

/* глобальные переменные */
HINSTANCE ghInst;                  /* хендл приложения */
HWND ghWndMain;                    /* хендл окна */
int giDebug;                       /* флаг отладки */

/* указатели окна 'status bar' */
#define STB_PRIMARY             0
#define STB_RUN                 1

/* хендл 'status bar' */
static HWND hStatusBar;
/* хендлы иконок */
static HICON hIconRun;
static HICON hIconStop;

/* указывает, что окно в фокусе */
static BOOL bActive;

/* отображает статус захвата управления в стутусном окне */
static void SetStatusRun(BOOL flags)
{
	HICON hIcon;

	hIcon = flags ? hIconRun : hIconStop;
	SendMessage(hStatusBar,SB_SETICON,STB_RUN,(LPARAM)hIcon);
}

/* функция окна */
static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rc;

	switch (msg) {
		case WM_CREATE:
			/* создается окно */
			break;

		case WM_PAINT:
			/* отресуем окно */
			hdc = BeginPaint(hWnd,&ps);
			if (!bActive) {
				GetClientRect(hWnd,&rc);
				BitBlt(hdc,0,0,rc.right,rc.bottom,ghMemDC,0,0,SRCCOPY);
			}
			EndPaint(hWnd,&ps);
			SetStatusRun(bActive);
			break;

		case WM_ACTIVATEAPP:
			/* окно получает или теряет фокус */
			bActive = (BOOL)wParam;
			/* захватим или освободим устройства ввода */
			AcquireInput(bActive);
			SetStatusRun(bActive);
			break;

		case WM_DESTROY:
			/* завершаем работу */
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
	}

	return 0;
}

/* проверка версии операционной системы */
static int VersionCheck(void)
{
	OSVERSIONINFO os;

	/* подготовим структуру и заполним ее */
	memset(&os,0,sizeof(os));
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&os))
		return 1;

	/* возвратим результат проверки */
	return (os.dwPlatformId == VER_PLATFORM_WIN32_NT) ? 0 : 1;
}

/* зарегестрируем клас окна и отобразим его */
static int InitializeApp(HINSTANCE hInst, int nCmdShow)
{
	DWORD dwWindowStyle;
	WNDCLASSEX wc;
	RECT rc;
	int nBorders[2];

	/* запомним хендл приложения */
	ghInst = hInst;

	/* заполним структуру класса окна */
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

	/* зарегистрируем класс окна */
	if (!RegisterClassEx(&wc))
		return 1;

	/* создадим окно */
	dwWindowStyle = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_VISIBLE;
	SetRect(&rc,0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
	AdjustWindowRect(&rc,dwWindowStyle,FALSE);
	ghWndMain = CreateWindowEx(0,szWindowClassName,szWindowTitle,dwWindowStyle,
		CW_USEDEFAULT,CW_USEDEFAULT,(rc.right - rc.left),(rc.bottom - rc.top)+20,
		NULL,NULL,hInst,NULL);
	if (!ghWndMain)
		return 1;

	/* загрузим иконки */
	hIconRun = LoadImage(hInst,MAKEINTRESOURCE(IDI_ICON2),IMAGE_ICON,0,0,0);
	if (!hIconRun)
		return 1;
	hIconStop = LoadImage(hInst,MAKEINTRESOURCE(IDI_ICON3),IMAGE_ICON,0,0,0);
	if (!hIconStop)
		return 1;

	/* создадим 'status bar' */
	InitCommonControls();
	hStatusBar = CreateStatusWindow(SBT_TOOLTIPS|WS_CHILD|WS_VISIBLE,NULL,ghWndMain,0);
	if (!hStatusBar)
		return 1;
	/* создадим отделы в окне статуса */
	nBorders[0] = SCREEN_WIDTH - 22;
	nBorders[1] = -1;
	SendMessage(hStatusBar,SB_SETPARTS,2,(LPARAM)nBorders);
	/* отобразим захваченный ввод */
	SetStatusRun(TRUE);

	/* отобразим окно */
	ShowWindow(ghWndMain,nCmdShow);
	UpdateWindow(ghWndMain);

	return 0;
}

/* точка входа в программу */
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PTSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	/* проверим версию операциооной системы */
	if (VersionCheck()) {
		MessageBox(NULL,_T("Error check Windows."),
			_T("Error"),MB_OK|MB_ICONSTOP);
		return 1;
	}

	/* проверим флаг отладочной информации */
//	if (!_tcscmp(lpCmdLine,_T("--debug")))
		giDebug = 1;

	/* создадим и отобразим окно */
	if (InitializeApp(hInst,nCmdShow)) {
		MessageBox(NULL,_T("Error start game."),
			_T("Error"),MB_OK|MB_ICONSTOP);
		return 1;
	}

	/* инициализация игры */
	if (InitGame()) {
		ShutdownGame();
		ShowWindow(ghWndMain,SW_HIDE);
		MessageBox(ghWndMain,_T("Error run game."),
			_T("Error"),MB_OK|MB_ICONSTOP);
		return 1;
	}

	/* главный цикл сообщений */
	for(;;) {
		/* окно находиться в фокусе */
		if (bActive) {
			/* если есть сообщение, то обработаем его */
			if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
				/* проверим на завершение программы */
				if (msg.message == WM_QUIT)
					break;
				/* отправим сообщение на обработку */
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				/* повторим выборку сообщения */
				continue;
			}
			/* вызвать обработчик игры */
			MainGame();
		} else {
			BOOL bRet;

			/* получим и обработаем сообщение */
			bRet = GetMessage(&msg,NULL,0,0);
			if (!bRet || (bRet == -1))
				break;
			/* отправим сообщение на обработку */
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	}

	/* деинициализация игры */
	ShutdownGame();

	return msg.wParam;
}
