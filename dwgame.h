/* dwgame.h */
#ifndef _DRAW_GAME_H_
#define _DRAW_GAME_H_

#include "draw.h"

/* ���������� ������ ���������� */
#define SWERVES_COUNT               4
/* ������������ ���������� ������� */
#define BALLS_COUNT                 3
/* ���������� �������� ������� */
#define SLIVERS_COUNT               9
/* ���������� ������ ������ */
#define BONUS_COUNT                10
/* ���������� ����� */
#define BULLETS_COUNT               2

/* ����� �������� ������� */
#define BACKGROUND_0                0
#define BACKGROUND_1                1
#define BACKGROUND_2                2
#define BACKGROUND_3                3

#define BACKGROUND_COUNT            4

/* ��� ������ */
#define BALL_NORMAL                 0        /* ������� ����� */
#define BALL_SMASH                  1        /* ������� ����� */

/* ����� ������� ����� ���������� */
#define PICTURE_SWERVE_OFF          0        /* �������� */
#define PICTURE_SWERVE_ON           1        /* ������� */

/* ������� ������ */
#define BONUS_POINTS                0
#define BONUS_SMART_BOMB            1
#define BONUS_EXTRA_LIFE            2
#define BONUS_SLOW_BALL             3
#define BONUS_SMASH_BALL            4
#define BONUS_TRIPLE_BALL           5
#define BONUS_HAND                  6
#define BONUS_GUN                   7
#define BONUS_EXTENDED_RACKET       8
#define BONUS_ROCKET_PACK           9
#define BONUS_400_POINTS           10

/* ��� ��������� �������� */
#define SHOW_ALIEN_BIRD             1        /* ����� */
#define SHOW_ALIEN_UFO              2        /* ��� */

/* ��������� ������������ ��������� */
#define MESSAGE_ROUND               1
#define MESSAGE_PAUSE               2

/* ��� ������� */
#define TYPE_RACKET_NORMAL          0        /* ���������� ������� */
#define TYPE_RACKET_GUN             1        /* ���������� ������� */
#define TYPE_RACKET_EXTEND          2        /* ������� ������� */

#define TYPE_RACKET_N_G_1           3        /* ������� �� ���������� � ���������� */
#define TYPE_RACKET_N_G_2           4        /* normal -> gun */
#define TYPE_RACKET_N_G_3           5        /* normal -> gun */
#define TYPE_RACKET_N_G_4           6        /* normal -> gun */

#define TYPE_RACKET_N_E_1           7        /* ������� �� ���������� � ������� */
#define TYPE_RACKET_N_E_2           8        /* normal -> extend */
#define TYPE_RACKET_N_E_3           9        /* normal -> extend */
#define TYPE_RACKET_N_E_4          10        /* normal -> extend */
#define TYPE_RACKET_N_E_5          11        /* normal -> extend */
#define TYPE_RACKET_N_E_6          12        /* normal -> extend */
#define TYPE_RACKET_N_E_7          13        /* normal -> extend */

/* ��� ������ � ��������� ������ */
#define TYPE_ROCKET_0               0
#define TYPE_ROCKET_1               1

/* ��� ������� �� ����� ��������� �������� (�����) */
#define TYPE_ALIEN_0                0
#define TYPE_ALIEN_1                1
#define TYPE_ALIEN_2                2
#define TYPE_ALIEN_3                3
#define TYPE_ALIEN_4                4

/* ��� ������� �� ����� ��������� �������� (���) */
#define TYPE_UFO_0                  0
#define TYPE_UFO_1                  1
#define TYPE_UFO_2                  2
#define TYPE_UFO_3                  3
#define TYPE_UFO_4                  4
#define TYPE_UFO_5                  5

/* ����� ������� �� ����� ������� ��������� �������� */
#define TYPE_BANG_0                 0
#define TYPE_BANG_1                 1
#define TYPE_BANG_2                 2
#define TYPE_BANG_3                 3
#define TYPE_BANG_4                 4

/* ����� ������� �� ����� ����� */
#define TYPE_BULLET_0               0
#define TYPE_BULLET_1               1

/* ����� ������� �� ����� ������� ����� */
#define TYPE_BULLET_PLOP_0          2
#define TYPE_BULLET_PLOP_1          3
#define TYPE_BULLET_PLOP_2          4
#define TYPE_BULLET_PLOP_3          5

/* ����� ������� �� ����� �������� ������� */
#define TYPE_SLIVER_0               0
#define TYPE_SLIVER_1               1
#define TYPE_SLIVER_2               2
#define TYPE_SLIVER_3               3
#define TYPE_SLIVER_4               4

/* ��������� ��������� ����� */
typedef struct {
	PICTURE picture;                   /* �������� */
	PICTURE shadow;                    /* ���� �� �������� */
} FULL_PICTURE, *LPFULL_PICTURE;

/* ��������� ����������� ������������ ��������� */
typedef struct {
	int show;                          /* ���� ����������� */
	int flags;                         /* ��������� ������������ ��������� */
	int player;                        /* ����� ������ */
	int round;                         /* ����� ������ */
	int pause;                         /* ����� */
} MESSAGE, *LPMESSAGE;

/* ��������� ��������� ������� ������� */
typedef struct {
	int show;                               /* ���� ����������� */
	FULL_PICTURE picture[SLIVERS_COUNT];    /* �������� �������� */
} SLIVERS, *LPSLIVERS;

/* ��������� ����������� ������������ �������� */
typedef struct {
	int on;                            /* ���� ����������� ����� */
	int background;                    /* ��� */
	int lifes;                         /* ���������� ������ */
	struct {
		int up1,up2,hi;
	} score;                           /* �������� ���� ��������� �������� */
	UCHAR block[12][15];               /* ���� �������� */
	PICTURE pRacket;                   /* ������� */
	PICTURE pRocket;                   /* ������ � ��������� ������ */
	PICTURE pAlien;                    /* �������� �������� */
	PICTURE pBang;                     /* ����� */
	PICTURE pBonus;                    /* ���� */
	PICTURE pBullet[BULLETS_COUNT];    /* ������ � ��� ������ */
	PICTURE pSwerves[SWERVES_COUNT];   /* ����� ���������� */
	FULL_PICTURE pBomb;                /* ����� */
	FULL_PICTURE pBall[BALLS_COUNT];   /* ������ */
	SLIVERS pSlivers;                  /* ������� ������� */
	MESSAGE message;                   /* ��������� */
} GAMESCENE;

/* ��������� �������� */
extern GAMESCENE scene;

/* ��������� ������������� ������ ��������� ���� */
int InitDrawGame(void);
/* ��������������� ������ ��������� ����� */
void ReleaseDrawGame(void);

/* ���������� ������ ��������� */
int GameInitDraw(void);

/* �������� ����� */
void GameCreateScene(void);
/* ��������� ����� */
void GameReleaseScene(void);
/* ���������� ��������� ���� */
void GameDrawScene(void);
/* ���������� ������� "game over" */
void GameDrawGameOver(void);

#endif  /* _DRAW_GAME_H_ */
