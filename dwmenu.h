/* dwmenu.h */
#ifndef _DRAW_MENU_H_
#define _DRAW_MENU_H_

#include "draw.h"

/* ��������� ������������ ���� */
typedef struct {
	PICTURE pCursor;
} MENUSCENE;

/* ��������� ����������� ���� */
extern MENUSCENE menu;

/* ��������� ������������� ������ ��������� �������� ���� */
int InitDrawMenu(void);
/* ��������������� ������ ��������� �������� ���� */
void ReleaseDrawMenu(void);

/* ���������� ������ ��������� ���� */
int MenuInitDraw(void);

/* ���������� ��������� ���� ���� */
int MenuDrawScene(void);

#endif  /* _DRAW_MENU_H_ */
