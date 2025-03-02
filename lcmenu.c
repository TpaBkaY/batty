/* lcmenu.c */
#include "global.h"
#include <math.h>
#include "lcmenu.h"
#include "dwmenu.h"
#include "input.h"

/* логические координаты игового поля */
#define LOGIC_WIDTH            (WIN_GAME_WIDTH - 2 - 2)
#define LOGIC_HEIGHT           (WIN_GAME_HEIGHT - 2 - 2)
#define LOGIC_X                 2
#define LOGIC_Y                 2

/* координата курсора в начале игры */
#define CURSOR_X               (LOGIC_X + (LOGIC_WIDTH / 2))
#define CURSOR_Y               (LOGIC_Y + (LOGIC_HEIGHT / 2))
/* множитель для тонкой настройки перемещения курсора */
#define CURSOR_FACTOR               0.5

/* описывает курсор */
typedef struct {
	int on;                          /* курсор отображается */
	int x,y;                         /* координаты курсора */
} CURSOR;

/* курсор */
static CURSOR cursor;

/* возвращает округленное целочисленное значение */
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

/* обрабатывает курсор */
static void MoveCursorMenu(void)
{
	/* проверим необходимость обработки */
	if (!cursor.on)
		return;

	/* проверим ось X мыши */
	if (input.x) {
		/* переместим указатель */
		cursor.x += ftol(input.x * CURSOR_FACTOR);
		if (cursor.x < LOGIC_X)
			cursor.x = LOGIC_X;
		else if (cursor.x > (LOGIC_X + LOGIC_WIDTH - 1))
			cursor.x = LOGIC_X + LOGIC_WIDTH - 1;
	}
	/* проверим ось Y мыши */
	if (input.y) {
		/* переместим указатель */
		cursor.y += ftol(input.y * CURSOR_FACTOR);
		if (cursor.y < LOGIC_Y)
			cursor.y = LOGIC_Y;
		else if (cursor.y > (LOGIC_Y + LOGIC_HEIGHT - 1))
			cursor.y = LOGIC_Y + LOGIC_HEIGHT - 1;
	}

	/* сформируем изображение */
	menu.pCursor.X = cursor.x;
	menu.pCursor.Y = cursor.y;
}

/* выполнить перемещение объектов меню */
int MenuMoveScene(void)
{
	/* обработаем курсор */
	MoveCursorMenu();


	return 0;
}

/* подготовка всех объектов меню */
void MenuCreateScene(void)
{
	/* очистим меню */
	memset(&menu,0,sizeof(menu));

	/* очистим курсор */
	memset(&cursor,0,sizeof(cursor));

	/* отобразим и разместим курсор */
	cursor.on = menu.pCursor.show = 1;
	cursor.x = menu.pCursor.X = CURSOR_X;
	cursor.y = menu.pCursor.Y = CURSOR_Y;
}
