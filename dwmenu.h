/* dwmenu.h */
#ifndef _DRAW_MENU_H_
#define _DRAW_MENU_H_

#include "draw.h"

/* описывает отображаемое меню */
typedef struct {
	PICTURE pCursor;
} MENUSCENE;

/* структура описывающая меню */
extern MENUSCENE menu;

/* начальная инициализация модуля рисования игрового меню */
int InitDrawMenu(void);
/* деинициализация модуля рисования игрового меню */
void ReleaseDrawMenu(void);

/* подготовка модуля рисования меню */
int MenuInitDraw(void);

/* нарисовать очередной кадр меню */
int MenuDrawScene(void);

#endif  /* _DRAW_MENU_H_ */
