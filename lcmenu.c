/* lcmenu.c */
#include "global.h"
#include <math.h>
#include "lcmenu.h"
#include "dwmenu.h"
#include "input.h"

/* ���������� ���������� ������� ���� */
#define LOGIC_WIDTH            (WIN_GAME_WIDTH - 2 - 2)
#define LOGIC_HEIGHT           (WIN_GAME_HEIGHT - 2 - 2)
#define LOGIC_X                 2
#define LOGIC_Y                 2

/* ���������� ������� � ������ ���� */
#define CURSOR_X               (LOGIC_X + (LOGIC_WIDTH / 2))
#define CURSOR_Y               (LOGIC_Y + (LOGIC_HEIGHT / 2))
/* ��������� ��� ������ ��������� ����������� ������� */
#define CURSOR_FACTOR               0.5

/* ��������� ������ */
typedef struct {
	int on;                          /* ������ ������������ */
	int x,y;                         /* ���������� ������� */
} CURSOR;

/* ������ */
static CURSOR cursor;

/* ���������� ����������� ������������� �������� */
static int ftol(double a)
{
	double integer;
	int val;

	if (modf(a,&integer) <= 0.5)
		val = (int)integer;
	else
		val = (int)integer + 1;

	return val;
}

/* ������������ ������ */
static void MoveCursorMenu(void)
{
	/* �������� ������������� ��������� */
	if (!cursor.on)
		return;

	/* �������� ��� X ���� */
	if (input.x) {
		/* ���������� ��������� */
		cursor.x += ftol(input.x * CURSOR_FACTOR);
		if (cursor.x < LOGIC_X)
			cursor.x = LOGIC_X;
		else if (cursor.x > (LOGIC_X + LOGIC_WIDTH - 1))
			cursor.x = LOGIC_X + LOGIC_WIDTH - 1;
	}
	/* �������� ��� Y ���� */
	if (input.y) {
		/* ���������� ��������� */
		cursor.y += ftol(input.y * CURSOR_FACTOR);
		if (cursor.y < LOGIC_Y)
			cursor.y = LOGIC_Y;
		else if (cursor.y > (LOGIC_Y + LOGIC_HEIGHT - 1))
			cursor.y = LOGIC_Y + LOGIC_HEIGHT - 1;
	}

	/* ���������� ����������� */
	menu.pCursor.X = cursor.x;
	menu.pCursor.Y = cursor.y;
}

/* ��������� ����������� �������� ���� */
int MenuMoveScene(void)
{
	/* ���������� ������ */
	MoveCursorMenu();


	return 0;
}

/* ���������� ���� �������� ���� */
void MenuCreateScene(void)
{
	/* ������� ���� */
	memset(&menu,0,sizeof(menu));

	/* ������� ������ */
	memset(&cursor,0,sizeof(cursor));

	/* ��������� � ��������� ������ */
	cursor.on = menu.pCursor.show = 1;
	cursor.x = menu.pCursor.X = CURSOR_X;
	cursor.y = menu.pCursor.Y = CURSOR_Y;
}
