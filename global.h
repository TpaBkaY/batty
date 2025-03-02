/* global.h */
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/* MFC �� ������������ */
#define WIN32_LEAN_AND_MEAN

/* �������� ��������� UNICODE */
#define UNICODE                 /* Windows */

#ifdef UNICODE
#define _UNICODE                /* RTL */
#endif

/* ��������� ����� ��������� */
#include <windows.h>
#include <Tchar.h>
#include <string.h>

/*
 * ��������: ��� ����������� � ���� �������� �������������� 1:2
 */
#define SCALE                    2

/* ������ ������ ��� ������� ��������� �������� */
#define TIME_BUFF               10

/*
 * ��������: ��� ������ ���� ������
 * ��������� � ������� ������.
 */
/* ������� ������ */
#define FPS                     50
/* �������� ����� ������� */
#define DELAY_FPS      (1000 / FPS)

/*
 * ��������: ��� ������ ���� ������
 * ��������� � ������ ������� �������� ����.
 */
/* ��������� ���� ���� */
#define WIN_GAME_WIDTH         256
#define WIN_GAME_HEIGHT        192

/* ���������� �������� ���� */
#define GAME_WIDTH        (WIN_GAME_WIDTH * SCALE)
#define GAME_HEIGHT       (WIN_GAME_HEIGHT * SCALE)
/* ����� �������� ���� */
#define SHIFT_WIDTH        10
#define SHIFT_HEIGHT       10
/* ��������� ���� */
#define SCREEN_WIDTH      (GAME_WIDTH + SHIFT_WIDTH * 2 * SCALE)
#define SCREEN_HEIGHT     (GAME_HEIGHT + SHIFT_HEIGHT * 2 * SCALE)
/* ������� ��������� �������� ���� */
#define GAME_X            ((SCREEN_WIDTH - GAME_WIDTH) / 2)
#define GAME_Y            ((SCREEN_HEIGHT - GAME_HEIGHT) / 2)

/*
 * ����������� ���������� ���������� �������� ����,
 * � ���������� �� ������������ ����������� �����������.
 */
#define GetX(x)     (((x) * SCALE) + GAME_X)
#define GetY(y)     (((y) * SCALE) + GAME_Y)

/* ������������� �������� DirectX */
#define INIT_DIRECT_STRUCT(str)    { memset(&str,0,sizeof(str)); str.dwSize = sizeof(str); }

/* ���������� ���������� */
extern HINSTANCE ghInst;           /* ����� ���������� */
extern HWND ghWndMain;             /* ����� �������� ���� */
extern int giDebug;                /* ���� ������� */

#endif  /* _GLOBAL_H_ */
