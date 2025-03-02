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

/* таймаут отображения надписи "game over" (2 сек.) */
#define TIME_SHOW_GAME_OVER    2000

/* состояния игры */
#define GAME_INIT_DRAW            1   /* инициализация модуля рисования игрового процесса */
#define GAME_CREATE_ROUND         2   /* создание игрового раунда */
#define GAME_RUN                  3   /* игра в игровом раунде */
#define GAME_EXIT                 4   /* выход из игрового раунда */

#define MENU_INIT_DRAW            5   /* инициализация модуля рисования меню игры */
#define MENU_CREATE_SCENE         6   /* подготовка структур меню */
#define MENU_RUN                  7   /* игра в меню */
#define MENU_EXIT                 8   /* выход из игры */

/* хендлы объектов */
static HANDLE hEvent,hThread;
/* флаг завершения потока синхронизации */
static DWORD bSyncQuit;

/* локальные переменные */
static int gameState;           /* хранит состояние игры */
static int level;               /* хранит уровень игры */
static DWORD dwLastTick;        /* хранит время предыдущего отображение кадра */

/* переменные для расчета среднего значения текущего FPS */
static int fpsBuff[TIME_BUFF];  /* буфер для расчета FPS */
static int pFps;                /* текущее положение в буфере */
int giFps;                      /* хранит FPS */

/* переменные для расчета среднего времени построения обного кадра */
static int frameBuff[TIME_BUFF];
static int pFrame;
int giFrame;

/* обработаем состояние GAME_INIT_DRAW */
static int stateGameInitDraw(void)
{
	/*
	 * Подготовим модуль рисования игрового
	 * процесса и переключим состояние.
	 */
	gameState = GameInitDraw() ? MENU_EXIT : GAME_CREATE_ROUND;
	/* установим начальный раунд */
	level = 0;

	return 0;
}

/* обработаем состояние GAME_CREATE_ROUND */
static int stateGameCreateRound(void)
{
	/* создание уровня */
	GameCreateLevel(level);
	/* создание объектов для сцены */
	GameCreateScene();
	/* переключим состояние */
	gameState = GAME_RUN;

	return 0;
}

/* обработаем состояние GAME_RUN */
static int stateGameRun(void)
{
	DWORD dwTime;
	
	if (giDebug)
		dwTime = timeGetTime();

	/* произведем ввод данных */
	GameInput();

	/* произвести перемещение объектов */
	switch (GameMoveScene()) {
		case 1:
			/* загрузим новый уровень */
			gameState = GAME_CREATE_ROUND;
			level++;
			break;

		case 2:
			/* игрок проиграл */
			gameState = GAME_EXIT;
			break;
	}

	/* проиграть звуки и музыку */
	GameSound();
	/* отобразим кадр */
	GameDrawScene();
	/* обновим кадр изображения */
	UpdateDraw();

	/* расчет времени построения одного кадра */
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

/* обработаем состояние GAME_EXIT */
static int stateGameExit(void)
{
	static int count = 0;
	int ret = 1;

	/* отобразим надпись "game over" */
	if (count < (TIME_SHOW_GAME_OVER / DELAY_FPS)) {
		DWORD dwTime;

		/* обновим счетчик */
		count++;
		/* получим время начала вывода изображения */
		if (giDebug)
			dwTime = timeGetTime();

		/* отобразим кадр */
		GameDrawGameOver();
		/* обновим кадр изображения */
		UpdateDraw();

		/* расчет времени построения одного кадра */
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
		/* переключим состояние на меню */
		gameState = MENU_INIT_DRAW;
		ret = 0;
	}

	return ret;
}

/* обработаем состояние MENU_INIT_DRAW */
static int stateMenuInitDraw(void)
{
	/* подготовим модуль рисования меню и переключим состояние */
	gameState = MenuInitDraw() ? MENU_EXIT : MENU_CREATE_SCENE;

	return 0;
}

/* обработаем состояние MENU_CREATE_SCENE */
static int stateMenuCreateScene(void)
{
	/* подготовим структуры описывающие меню */
	MenuCreateScene();
	/* переключим состояние */
	gameState = MENU_RUN;

	return 0;
}

/* обработаем состояние MENU_RUN */
static int stateMenuRun(void)
{
	DWORD dwTime;

	/////////////////////////////////////////////////////
	gameState = GAME_INIT_DRAW;
	return 0;
	/////////////////////////////////////////////////////

	/* получим время начала вывода изображения */
	if (giDebug)
		dwTime = timeGetTime();

	/* произведем ввод данных */
	GameInput();
	/* произвести перемещение объектов */
	MenuMoveScene();
	/* проиграть звуки и музыку */
	GameSound();
	/* отобразим кадр */
	MenuDrawScene();
	/* обновим кадр изображения */
	UpdateDraw();

	/* расчет времени построения одного кадра */
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

/* обработаем состояние MENU_EXIT */
static int stateMenuExit(void)
{
	/* пошлем сообщение об выходе окну приложения */
	PostMessage(ghWndMain,WM_CLOSE,0,0);

	return 0;
}

/* обработать состояние игры */
int procGameState(void)
{
	/* проверяет текущее состояние игры */
	switch (gameState) {
		/* подготовить модуль рисования игрового процесса */
		case GAME_INIT_DRAW:
			return stateGameInitDraw();
		/* создадим игровой раунд */
		case GAME_CREATE_ROUND:
			return stateGameCreateRound();
		/* идет процесс игры */
		case GAME_RUN:
			return stateGameRun();
		/* завершение игрового процесса */
		case GAME_EXIT:
			return stateGameExit();
		/* подготовка модуля рисования меню */
		case MENU_INIT_DRAW:
			return stateMenuInitDraw();
		/* подготовим структуры меню */
		case MENU_CREATE_SCENE:
			return stateMenuCreateScene();
		/* игра работает в меню */
		case MENU_RUN:
			return stateMenuRun();
		/* завершение игры */
		case MENU_EXIT:
			return stateMenuExit();
	}

	return 0;
}

/* цикл обработки игры */
void MainGame(void)
{
	/* проверить нажатие ESC */
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		gameState = MENU_EXIT;

	/* расчитаем FPS */
	if (giDebug) {
		DWORD dwTempTick = timeGetTime();

		/* получим прошедшее время */
		fpsBuff[pFps] = dwTempTick - dwLastTick;
		/* если буфер заполнился, расчитываем FPS */
		if (++pFps >= TIME_BUFF) {
			int i,sum,delay;
			/* расчитаем среднее время */
			for (i = sum = 0; i < TIME_BUFF; i++)
				sum += fpsBuff[i];
			/* расчитаем время задержки */
			delay = sum / TIME_BUFF;
			/* расчитаем FPS */
			giFps = 1000 / delay;
			/* сбросим указатель буфера */
			pFps = 0;
		}
		/* обновим время последнего кадра */
		dwLastTick = dwTempTick;
	}

	/* обработаем состояние игры */
	if (!procGameState())
		return;

	/* сделаем задержку для обеспечения FPS */
	WaitForSingleObject(hEvent,INFINITE);
}

/* поток отсчета меток для обеспечения FPS */
static DWORD WINAPI SyncThreadProc(LPVOID lpParameter)
{
	/* проверка завершения работы */
	while (!bSyncQuit) {
		Sleep(DELAY_FPS);
		/* установим событие генерации нового кадра */
		SetEvent(hEvent);
	}

	return 0;
}

/* инициализация игры */
int InitGame(void)
{
	DWORD threadId;

	/* инициализация модуля рисования */
	if (InitDraw())
		return 1;
	/* инициализация звукового модуля */
	if (InitSound())
		return 1;
	/* инициализация модуля ввода */
	if (InitInput())
		return 1;
	/* инициализация модуля рисования игрового процесса */
	if (InitDrawGame())
		return 1;
	/* инициализация модуля рисования меню игры */
	if (InitDrawMenu())
		return 1;

	/* настройка таймера с разрешением в 1 ms */
	timeBeginPeriod(1);
	/* инициализация генератора случайных чисел */
	srand(timeGetTime());
	/* проинициализируем начальное время */
	dwLastTick = timeGetTime();

	/* игра стартует */
	gameState = MENU_INIT_DRAW;

	/* создадим событие с автосбросом */
	hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	/* создадим поток синхронизации и установим его приоритет */
	hThread = (HANDLE) _beginthreadex(NULL,0,
		(unsigned int (__stdcall *)(void *))SyncThreadProc,
		NULL,0,&threadId);
	SetThreadPriority(hThread,THREAD_PRIORITY_TIME_CRITICAL);

	return 0;
}

/* деинициализация игры */
void ShutdownGame(void)
{
	/* установим флаг завершения потока синхронизации */
	bSyncQuit = 1;

	/* деинициализация таймера */
	timeEndPeriod(1);

	/* деинициализация модуля рисования игрового меню */
	ReleaseDrawMenu();
	/* деинициализация модуля рисования игрового процесса */
	ReleaseDrawGame();
	/* деинициализация модуля ввода */
	ReleaseInput();
	/* деинициализация звукового модуля */
	ReleaseSound();
	/* деинициализация модуля рисования */
	ReleaseDraw();

	/* подождем завершения потока синхронизации */
	WaitForSingleObject(hThread,INFINITE);

	/* закроем все хендлы */
	CloseHandle(hEvent);
	CloseHandle(hThread);
}
