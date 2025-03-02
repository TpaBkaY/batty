/* input.h */
#ifndef _INPUT_H_
#define _INPUT_H_

#include <windows.h>

/* ��������� ������������� ���� */
typedef struct {
	/* ��������� ������� ������� */
	int K;          /* ����������� ������� � ���� */
	int L;          /* � ����� */
	int SPACE;      /* �������� */
	int P;
	/* ��������� ����������� ���� */
	int x,y;          /* ����������� ������� */
	/* ��������� ����� ������ ���� */
	int lButton;    /* �������� */
} GAMEINPUT;

/* ������������� ������ ����� ������ */
int InitInput(void);
/* ������������ ������ ����� ������ */
void ReleaseInput(void);

/* �������� ���� ������ */
int GameInput(void);
/* ��������� ���������� ����� */
int AcquireInput(BOOL flags);

/* �������� ����� */
extern GAMEINPUT input;

#endif  /* _INPUT_H_ */
