/* game.h */
#ifndef _GAME_H_
#define _GAME_H_

/* ������������� ���� */
int InitGame(void);
/* �������� ���� ���� */
void MainGame(void);
/* �������� ���� */
void ShutdownGame(void);

/* ������� ���������� ������ � ������� */
extern int giFps;
/* ������� ����� ���������� ������ ����� */
extern int giFrame;

#endif  /* _GAME_H_ */
