/* global.h */
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/* MFC не используется */
#define WIN32_LEAN_AND_MEAN

/* включаем поддержку UNICODE */
#define UNICODE                 /* Windows */

#ifdef UNICODE
#define _UNICODE                /* RTL */
#endif

/* подключим общие заголовки */
#include <windows.h>
#include <Tchar.h>
#include <string.h>

/*
 * ВНИМАНИЕ: все изображения в игре проходят маштабирование 1:2
 */
#define SCALE                    2

/* разбер буфера для расчета временных отрезков */
#define TIME_BUFF               10

/*
 * ВНИМАНИЕ: вся логика игры жестко
 * привязана к частоте кадров.
 */
/* частота кадров */
#define FPS                     50
/* задержка между кадрами */
#define DELAY_FPS      (1000 / FPS)

/*
 * ВНИМАНИЕ: вся логика игры жестко
 * привязана к даному размеру игрового окна.
 */
/* параметры окна игры */
#define WIN_GAME_WIDTH         256
#define WIN_GAME_HEIGHT        192

/* координаты игрового окна */
#define GAME_WIDTH        (WIN_GAME_WIDTH * SCALE)
#define GAME_HEIGHT       (WIN_GAME_HEIGHT * SCALE)
/* сдвиг игрового окна */
#define SHIFT_WIDTH        10
#define SHIFT_HEIGHT       10
/* параметры окна */
#define SCREEN_WIDTH      (GAME_WIDTH + SHIFT_WIDTH * 2 * SCALE)
#define SCREEN_HEIGHT     (GAME_HEIGHT + SHIFT_HEIGHT * 2 * SCALE)
/* позиция отрисовки игрового окна */
#define GAME_X            ((SCREEN_WIDTH - GAME_WIDTH) / 2)
#define GAME_Y            ((SCREEN_HEIGHT - GAME_HEIGHT) / 2)

/*
 * Преобразуют логические координаты игрового окна,
 * в координату на отображаемой поверхности поверхности.
 */
#define GetX(x)     (((x) * SCALE) + GAME_X)
#define GetY(y)     (((y) * SCALE) + GAME_Y)

/* инициализация структур DirectX */
#define INIT_DIRECT_STRUCT(str)    { memset(&str,0,sizeof(str)); str.dwSize = sizeof(str); }

/* глобальные переменные */
extern HINSTANCE ghInst;           /* хендл приложения */
extern HWND ghWndMain;             /* хендл главного окна */
extern int giDebug;                /* флаг отладки */

#endif  /* _GLOBAL_H_ */
