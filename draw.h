/* draw.h */
#ifndef _DRAW_H_
#define _DRAW_H_

#include <windows.h>

/* ������ �������� ������� */
typedef struct {
	int show;                          /* ���� ����������� */
	int type;                          /* ��� ������� */
	int X,Y;                           /* ���������� */
} PICTURE, *LPPICTURE;

/* ����������� ��� ��������� */
extern UINT *ptImage;
/* �������� ��� ��������� � ������ */
extern HDC ghMemDC;

/* ������������� ������ ��������� ����� */
int InitDraw(void);
/* ��������������� ������ ��������� ����� */
void ReleaseDraw(void);

/* ��������� ����������� (����������� �����) */
void UpdateDraw(void);

#endif  /* _DRAW_H */
