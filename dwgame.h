/* dwgame.h */
#ifndef _DRAW_GAME_H_
#define _DRAW_GAME_H_

#include "draw.h"

/* количество кругов откланения */
#define SWERVES_COUNT               4
/* максимальное количество мячиков */
#define BALLS_COUNT                 3
/* количество осколков ракетки */
#define SLIVERS_COUNT               9
/* количество разных призов */
#define BONUS_COUNT                10
/* количество пулек */
#define BULLETS_COUNT               2

/* номер фонового рисунка */
#define BACKGROUND_0                0
#define BACKGROUND_1                1
#define BACKGROUND_2                2
#define BACKGROUND_3                3

#define BACKGROUND_COUNT            4

/* тип мячика */
#define BALL_NORMAL                 0        /* обычный мячик */
#define BALL_SMASH                  1        /* большой мячик */

/* номер рисунка круга откланения */
#define PICTURE_SWERVE_OFF          0        /* выключен */
#define PICTURE_SWERVE_ON           1        /* включен */

/* рисунки призов */
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

/* вид летающего существа */
#define SHOW_ALIEN_BIRD             1        /* птица */
#define SHOW_ALIEN_UFO              2        /* НЛО */

/* указывает отображаемое сообщение */
#define MESSAGE_ROUND               1
#define MESSAGE_PAUSE               2

/* тип ракетки */
#define TYPE_RACKET_NORMAL          0        /* нормальная ракетка */
#define TYPE_RACKET_GUN             1        /* стреляющая ракетка */
#define TYPE_RACKET_EXTEND          2        /* большая ракетка */

#define TYPE_RACKET_N_G_1           3        /* переход от нормальной к стреляющей */
#define TYPE_RACKET_N_G_2           4        /* normal -> gun */
#define TYPE_RACKET_N_G_3           5        /* normal -> gun */
#define TYPE_RACKET_N_G_4           6        /* normal -> gun */

#define TYPE_RACKET_N_E_1           7        /* переход от нормальной к большой */
#define TYPE_RACKET_N_E_2           8        /* normal -> extend */
#define TYPE_RACKET_N_E_3           9        /* normal -> extend */
#define TYPE_RACKET_N_E_4          10        /* normal -> extend */
#define TYPE_RACKET_N_E_5          11        /* normal -> extend */
#define TYPE_RACKET_N_E_6          12        /* normal -> extend */
#define TYPE_RACKET_N_E_7          13        /* normal -> extend */

/* тип ракеты в следующую стадию */
#define TYPE_ROCKET_0               0
#define TYPE_ROCKET_1               1

/* тип рисунка из серии летающего существа (птица) */
#define TYPE_ALIEN_0                0
#define TYPE_ALIEN_1                1
#define TYPE_ALIEN_2                2
#define TYPE_ALIEN_3                3
#define TYPE_ALIEN_4                4

/* тип рисунка из серии летающего существа (НЛО) */
#define TYPE_UFO_0                  0
#define TYPE_UFO_1                  1
#define TYPE_UFO_2                  2
#define TYPE_UFO_3                  3
#define TYPE_UFO_4                  4
#define TYPE_UFO_5                  5

/* номер рисунка из серии взрывов летающего существа */
#define TYPE_BANG_0                 0
#define TYPE_BANG_1                 1
#define TYPE_BANG_2                 2
#define TYPE_BANG_3                 3
#define TYPE_BANG_4                 4

/* номер рисунка из серии пулек */
#define TYPE_BULLET_0               0
#define TYPE_BULLET_1               1

/* номер рисунка из серии хлопков пулек */
#define TYPE_BULLET_PLOP_0          2
#define TYPE_BULLET_PLOP_1          3
#define TYPE_BULLET_PLOP_2          4
#define TYPE_BULLET_PLOP_3          5

/* номер рисунка из серии осколков ракетки */
#define TYPE_SLIVER_0               0
#define TYPE_SLIVER_1               1
#define TYPE_SLIVER_2               2
#define TYPE_SLIVER_3               3
#define TYPE_SLIVER_4               4

/* структура описывает мячик */
typedef struct {
	PICTURE picture;                   /* картинка */
	PICTURE shadow;                    /* тень от кортинки */
} FULL_PICTURE, *LPFULL_PICTURE;

/* структура описывающая отображаемое сообщение */
typedef struct {
	int show;                          /* флаг отоброжения */
	int flags;                         /* указывает отображаемое сообщение */
	int player;                        /* номер игрока */
	int round;                         /* номер раунда */
	int pause;                         /* пауза */
} MESSAGE, *LPMESSAGE;

/* структура описывает осколки ракетки */
typedef struct {
	int show;                               /* флаг отображения */
	FULL_PICTURE picture[SLIVERS_COUNT];    /* картинки осколков */
} SLIVERS, *LPSLIVERS;

/* структура описывающая изображаемую картинку */
typedef struct {
	int on;                            /* флаг отображения сцены */
	int background;                    /* фон */
	int lifes;                         /* количество жизней */
	struct {
		int up1,up2,hi;
	} score;                           /* призовые очки набранные игроками */
	UCHAR block[12][15];               /* поле кирпичей */
	PICTURE pRacket;                   /* ракетка */
	PICTURE pRocket;                   /* ракета в следующую стадию */
	PICTURE pAlien;                    /* летающее существо */
	PICTURE pBang;                     /* взрыв */
	PICTURE pBonus;                    /* приз */
	PICTURE pBullet[BULLETS_COUNT];    /* пульки и его хлопки */
	PICTURE pSwerves[SWERVES_COUNT];   /* круги откланения */
	FULL_PICTURE pBomb;                /* бомба */
	FULL_PICTURE pBall[BALLS_COUNT];   /* мячики */
	SLIVERS pSlivers;                  /* осколки ракетки */
	MESSAGE message;                   /* сообщение */
} GAMESCENE;

/* описывает картинку */
extern GAMESCENE scene;

/* начальная инициализация модуля рисования игры */
int InitDrawGame(void);
/* деинициализация модуля рисования кадра */
void ReleaseDrawGame(void);

/* подготовка модуля рисования */
int GameInitDraw(void);

/* создание сцены */
void GameCreateScene(void);
/* разрушить сцену */
void GameReleaseScene(void);
/* нарисовать очередной кадр */
void GameDrawScene(void);
/* отобразить надпись "game over" */
void GameDrawGameOver(void);

#endif  /* _DRAW_GAME_H_ */
