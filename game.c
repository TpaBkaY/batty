/* game.c */
#include "global.h"
#include <mmsystem.h>
#include <stdlib.h>
#include <process.h>
#include "game.h"
#include "draw.h"
#include "dwgame.h"
#include "dwmenu.h"
#include "sound.h"
#include "input.h"
#include "lcgame.h"
#include "lcmenu.h"

/* ������� ����������� ������� "game over" (2 ���.) */
#define TIME_SHOW_GAME_OVER    2000

/* ��������� ���� */
#define GAME_INIT_DRAW            1   /* ������������� ������ ��������� �������� �������� */
#define GAME_CREATE_ROUND         2   /* �������� �������� ������ */
#define GAME_RUN                  3   /* ���� � ������� ������ */
#define GAME_EXIT                 4   /* ����� �� �������� ������ */

#define MENU_INIT_DRAW            5   /* ������������� ������ ��������� ���� ���� */
#define MENU_CREATE_SCENE         6   /* ���������� �������� ���� */
#define MENU_RUN                  7   /* ���� � ���� */
#define MENU_EXIT                 8   /* ����� �� ���� */

/* ������ �������� */
static HANDLE hEvent,hThread;
/* ���� ���������� ������ ������������� */
static DWORD bSyncQuit;

/* ��������� ���������� */
static int gameState;           /* ������ ��������� ���� */
static int level;               /* ������ ������� ���� */
static DWORD dwLastTick;        /* ������ ����� ����������� ����������� ����� */

/* ���������� ��� ������� �������� �������� �������� FPS */
static int fpsBuff[TIME_BUFF];  /* ����� ��� ������� FPS */
static int pFps;                /* ������� ��������� � ������ */
int giFps;                      /* ������ FPS */

/* ���������� ��� ������� �������� ������� ���������� ������ ����� */
static int frameBuff[TIME_BUFF];
static int pFrame;
int giFrame;

/* ���������� ��������� GAME_INIT_DRAW */
static int stateGameInitDraw(void)
{
	/*
	 * ���������� ������ ��������� ��������
	 * �������� � ���������� ���������.
	 */
	gameState = GameInitDraw() ? MENU_EXIT : GAME_CREATE_ROUND;
	/* ��������� ��������� ����� */
	level = 0;

	return 0;
}

/* ���������� ��������� GAME_CREATE_ROUND */
static int stateGameCreateRound(void)
{
	/* �������� ������ */
	GameCreateLevel(level);
	/* �������� �������� ��� ����� */
	GameCreateScene();
	/* ���������� ��������� */
	gameState = GAME_RUN;

	return 0;
}

/* ���������� ��������� GAME_RUN */
static int stateGameRun(void)
{
	DWORD dwTime;
	
	if (giDebug)
		dwTime = timeGetTime();

	/* ���������� ���� ������ */
	GameInput();

	/* ���������� ����������� �������� */
	switch (GameMoveScene()) {
		case 1:
			/* �������� ����� ������� */
			gameState = GAME_CREATE_ROUND;
			level++;
			break;

		case 2:
			/* ����� �������� */
			gameState = GAME_EXIT;
			break;
	}

	/* ��������� ����� � ������ */
	GameSound();
	/* ��������� ���� */
	GameDrawScene();
	/* ������� ���� ����������� */
	UpdateDraw();

	/* ������ ������� ���������� ������ ����� */
	if (giDebug) {
		frameBuff[pFrame] = timeGetTime() - dwTime;
		if (++pFrame >= TIME_BUFF) {
			int i,sum;
			for (i = sum = 0; i < TIME_BUFF; i++)
				sum += frameBuff[i];
			giFrame = sum / TIME_BUFF;
			pFrame = 0;
		}
	}

	return 1;
}

/* ���������� ��������� GAME_EXIT */
static int stateGameExit(void)
{
	static int count = 0;
	int ret = 1;

	/* ��������� ������� "game over" */
	if (count < (TIME_SHOW_GAME_OVER / DELAY_FPS)) {
		DWORD dwTime;

		/* ������� ������� */
		count++;
		/* ������� ����� ������ ������ ����������� */
		if (giDebug)
			dwTime = timeGetTime();

		/* ��������� ���� */
		GameDrawGameOver();
		/* ������� ���� ����������� */
		UpdateDraw();

		/* ������ ������� ���������� ������ ����� */
		if (giDebug) {
			frameBuff[pFrame] = timeGetTime() - dwTime;
			if (++pFrame >= TIME_BUFF) {
				int i,sum;
				for (i = sum = 0; i < TIME_BUFF; i++)
					sum += frameBuff[i];
				giFrame = sum / TIME_BUFF;
				pFrame = 0;
			}
		}
	} else {
		count = 0;
		/* ���������� ��������� �� ���� */
		gameState = MENU_INIT_DRAW;
		ret = 0;
	}

	return ret;
}

/* ���������� ��������� MENU_INIT_DRAW */
static int stateMenuInitDraw(void)
{
	/* ���������� ������ ��������� ���� � ���������� ��������� */
	gameState = MenuInitDraw() ? MENU_EXIT : MENU_CREATE_SCENE;

	return 0;
}

/* ���������� ��������� MENU_CREATE_SCENE */
static int stateMenuCreateScene(void)
{
	/* ���������� ��������� ����������� ���� */
	MenuCreateScene();
	/* ���������� ��������� */
	gameState = MENU_RUN;

	return 0;
}

/* ���������� ��������� MENU_RUN */
static int stateMenuRun(void)
{
	DWORD dwTime;

	/////////////////////////////////////////////////////
	gameState = GAME_INIT_DRAW;
	return 0;
	/////////////////////////////////////////////////////

	/* ������� ����� ������ ������ ����������� */
	if (giDebug)
		dwTime = timeGetTime();

	/* ���������� ���� ������ */
	GameInput();
	/* ���������� ����������� �������� */
	MenuMoveScene();
	/* ��������� ����� � ������ */
	GameSound();
	/* ��������� ���� */
	MenuDrawScene();
	/* ������� ���� ����������� */
	UpdateDraw();

	/* ������ ������� ���������� ������ ����� */
	if (giDebug) {
		frameBuff[pFrame] = timeGetTime() - dwTime;
		if (++pFrame >= TIME_BUFF) {
			int i,sum;
			for (i = sum = 0; i < TIME_BUFF; i++)
				sum += frameBuff[i];
			giFrame = sum / TIME_BUFF;
			pFrame = 0;
		}
	}

	return 1;
}

/* ���������� ��������� MENU_EXIT */
static int stateMenuExit(void)
{
	/* ������ ��������� �� ������ ���� ���������� */
	PostMessage(ghWndMain,WM_CLOSE,0,0);

	return 0;
}

/* ���������� ��������� ���� */
int procGameState(void)
{
	/* ��������� ������� ��������� ���� */
	switch (gameState) {
		/* ����������� ������ ��������� �������� �������� */
		case GAME_INIT_DRAW:
			return stateGameInitDraw();
		/* �������� ������� ����� */
		case GAME_CREATE_ROUND:
			return stateGameCreateRound();
		/* ���� ������� ���� */
		case GAME_RUN:
			return stateGameRun();
		/* ���������� �������� �������� */
		case GAME_EXIT:
			return stateGameExit();
		/* ���������� ������ ��������� ���� */
		case MENU_INIT_DRAW:
			return stateMenuInitDraw();
		/* ���������� ��������� ���� */
		case MENU_CREATE_SCENE:
			return stateMenuCreateScene();
		/* ���� �������� � ���� */
		case MENU_RUN:
			return stateMenuRun();
		/* ���������� ���� */
		case MENU_EXIT:
			return stateMenuExit();
	}

	return 0;
}

/* ���� ��������� ���� */
void MainGame(void)
{
	/* ��������� ������� ESC */
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		gameState = MENU_EXIT;

	/* ��������� FPS */
	if (giDebug) {
		DWORD dwTempTick = timeGetTime();

		/* ������� ��������� ����� */
		fpsBuff[pFps] = dwTempTick - dwLastTick;
		/* ���� ����� ����������, ����������� FPS */
		if (++pFps >= TIME_BUFF) {
			int i,sum,delay;
			/* ��������� ������� ����� */
			for (i = sum = 0; i < TIME_BUFF; i++)
				sum += fpsBuff[i];
			/* ��������� ����� �������� */
			delay = sum / TIME_BUFF;
			/* ��������� FPS */
			giFps = 1000 / delay;
			/* ������� ��������� ������ */
			pFps = 0;
		}
		/* ������� ����� ���������� ����� */
		dwLastTick = dwTempTick;
	}

	/* ���������� ��������� ���� */
	if (!procGameState())
		return;

	/* ������� �������� ��� ����������� FPS */
	WaitForSingleObject(hEvent,INFINITE);
}

/* ����� ������� ����� ��� ����������� FPS */
static DWORD WINAPI SyncThreadProc(LPVOID lpParameter)
{
	/* �������� ���������� ������ */
	while (!bSyncQuit) {
		Sleep(DELAY_FPS);
		/* ��������� ������� ��������� ������ ����� */
		SetEvent(hEvent);
	}

	return 0;
}

/* ������������� ���� */
int InitGame(void)
{
	DWORD threadId;

	/* ������������� ������ ��������� */
	if (InitDraw())
		return 1;
	/* ������������� ��������� ������ */
	if (InitSound())
		return 1;
	/* ������������� ������ ����� */
	if (InitInput())
		return 1;
	/* ������������� ������ ��������� �������� �������� */
	if (InitDrawGame())
		return 1;
	/* ������������� ������ ��������� ���� ���� */
	if (InitDrawMenu())
		return 1;

	/* ��������� ������� � ����������� � 1 ms */
	timeBeginPeriod(1);
	/* ������������� ���������� ��������� ����� */
	srand(timeGetTime());
	/* ����������������� ��������� ����� */
	dwLastTick = timeGetTime();

	/* ���� �������� */
	gameState = MENU_INIT_DRAW;

	/* �������� ������� � ����������� */
	hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	/* �������� ����� ������������� � ��������� ��� ��������� */
	hThread = (HANDLE) _beginthreadex(NULL,0,
		(unsigned int (__stdcall *)(void *))SyncThreadProc,
		NULL,0,&threadId);
	SetThreadPriority(hThread,THREAD_PRIORITY_TIME_CRITICAL);

	return 0;
}

/* ��������������� ���� */
void ShutdownGame(void)
{
	/* ��������� ���� ���������� ������ ������������� */
	bSyncQuit = 1;

	/* ��������������� ������� */
	timeEndPeriod(1);

	/* ��������������� ������ ��������� �������� ���� */
	ReleaseDrawMenu();
	/* ��������������� ������ ��������� �������� �������� */
	ReleaseDrawGame();
	/* ��������������� ������ ����� */
	ReleaseInput();
	/* ��������������� ��������� ������ */
	ReleaseSound();
	/* ��������������� ������ ��������� */
	ReleaseDraw();

	/* �������� ���������� ������ ������������� */
	WaitForSingleObject(hThread,INFINITE);

	/* ������� ��� ������ */
	CloseHandle(hEvent);
	CloseHandle(hThread);
}
