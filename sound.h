/* sound.h */
#ifndef _SOUND_H_
#define _SOUND_H_

/* ��������� ����� */
typedef struct {
	int brik;           /* �������� � ������ */
	int stone;          /* �������� � ������� ������ */
	int wall;           /* ��������� � ������� ��� ����� */
	int bang;           /* �������� � �������� �������� */
	int swerve;         /* ������������ ����� ���������� */
	int destroy;        /* ����� ������� */
	int transform;      /* �������������� ������� */
	int bonus;          /* ������� ����� */
	int rocket;         /* ������� ������ */
} GAMEPLAY;

/* ������������� ������ */
int InitSound(void);
/* ��������������� ������ */
void ReleaseSound(void);

/* ������������� ����� */
int GameSound(void);

extern GAMEPLAY play;

#endif  /* _SOUND_H_ */
