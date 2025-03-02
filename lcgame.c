/*
 * lggame.c
 *
 * ВНИМАНИЕ: все координаты фигур в логике расчитываются с
 * привязкой к логическому окну с размерами (256,192) с
 * началной точкой в (0,0). А потом, в модуле 'draw.h'
 * они преобразоваваются в экранные координаты, простым
 * добавлением смещения к логическому окну, для того что бы
 * оно было посередине экранна. (см 'global.h')
 */
#include "global.h"
#include <stdlib.h>
#include <math.h>
#include "lcgame.h"
#include "dwgame.h"
#include "input.h"
#include "sound.h"

/* количество жизней в начале игры */
#define LIFE_COUNT             2

/* логические координаты игового поля */
#define LOGIC_WIDTH            (WIN_GAME_WIDTH - 8 - 8)
#define LOGIC_HEIGHT           (WIN_GAME_HEIGHT - 8)
#define LOGIC_X                8
#define LOGIC_Y                8

/* pi */
#define PI                3.14159265358979

/* координата ракетки в начале игры */
#define RACKET_X              (LOGIC_X+108)
#define RACKET_Y              (LOGIC_Y+165)
/* множитель для тонкой настройки перемещения ракетки */
#define RACKET_FACTOR               0.3

/* скорости мячика */
#define START_SPEED_BALL            2.8
#define MIN_SPEED_BALL              2.0
#define MAX_SPEED_BALL              5.6
#define ADD_SPEED_BALL              0.8

/*
 * Время через которое мячик изменит свое напровление,
 * если он не столкнется с ракеткой или не разобъет
 * кирпич или не попадет в летающее существо (15 сек).
 */
#define TIME_NEW_COURSE_BALL       (15 * FPS)
/* время через которое увеличивается скорость мячика (15 сек) */
#define TIME_ADD_SPEED_BALL        (15 * FPS)
/* время через которое мячик отлипнет от руки (4 сек) */
#define TIME_ENDING_HAND            (4 * FPS)
/* время через которое мячик обратно станет обычным (6 сек) */
#define TIME_ENDING_SMASH_BALL      (6 * FPS)

/* скорость разлетания осколков ракетки */
#define SPEED_SLIVER               1.65

/* радиус круга отклонения */
#define SWERVE_RADIUS                11

/* ускорение падающих фигур по оси 'y' */
#define ACCELERATION_Y            0.029
/* максимальная скорость падающей фигуры */
#define MAX_SPEED_Y                 1.9

/*
 * Минимальное растояние между пулькой и ракеткой
 * при которой можно создавать новую пульку (в точках).
 */
#define MIN_SPACE_BULLET             44
/* скорость движения пульки */
#define SPEED_BULLET                  6

/* описывает круг откланения */
typedef struct {
	int on;                 /* круг включен */
	int count;              /* счетчик до перегрузки состояния */
	int x,y;                /* центральная точка круга */
} SWERVE, *LPSWERVE;

/* описывает блок с которым соприкасается мячик */
typedef struct {
	int x,y;                    /* его положение в масиве */
	int offset_x, offset_y;     /* насколько глубоко вошел мячик */
} BOUND, *LPBOUND;

/* описывает шарик */
typedef struct {
	int on;                /* мячик отображается */
	int count;             /* счетчик изменения направления */
	int angle;             /* угол направления движения (в градусах) */
	/*
	 * Для повышения точности все расчеты выполняются
	 * с 'double' с последующим их приведением к 'int'.
	 */
	int ix,iy;             /* целочисленные координаты */
	double v;              /* скорость шарика */
	double vx,vy;          /* проекция скорости на координаты 'x' и 'y' */
	double x,y;            /* координаты */
	/* взаимодействие мячика с кругом откланения */
	struct {
		int lock;          /* круг захватил мячик */
		int angle;         /* угол к которому идет откланение */
		int num;           /* номер круга откланения захватившего шарик */
	} swerve;
	/* взаимодейстия мячика с призом 'BONUS_HAND' */
	struct {
		int count;         /* окончание магнита */
		int offset;        /* в примагниченном состоянии смещение мячика */
	} hand;
} BALL, *LPBALL;

/* описывает один кадр анимации */
typedef struct {
	int number;                     /* номер кадра */
	int count;                      /* число повторений кадра */
} FRAME;

/* описывает анимацию */
typedef struct {
	int point;                   /* текуший изображаемый кадр */
	int count;                   /* счетчик повторений кадра */
	const FRAME *frames;         /* указатель на буфер с кадрами */
} ANIMATION;

/* описывает ракетку */
typedef struct {
	int on;                          /* ракетка отображается */
	int x,y;                         /* координаты ракетки */
	const FRAME *frames;             /* указатель на следующий буфер с кадрами */
	ANIMATION animation;             /* анимации изменения ракетки */
} RACKET;

/* описывает летающее существо */
typedef struct {
	int on;                         /* 'alien' отображается */
	int angle;                      /* угол направления движения (в градусах) */
	int ix,iy;                      /* целочисленные координаты */
	double v;                       /* скорость 'alien' */
	double vx,vy;                   /* проекция скорости на 'x' и на 'y' */
	double x,y;                     /* координаты */
	ANIMATION animation;            /* анимации летающего существа */
	/* структура конечного автомата ИИ */
	struct {
		int state;                  /* состояние конечного автомата */
		int count;                  /* счетчик до перегрузки состояния */
	} automat;
} ALIEN;

/* описывает взрыв летающего существа */
typedef struct {
	int on;                         /* взрыв отображается */
	int point;                      /* текущий изображаемый кадр */
	int count;                      /* счетчик повтореных кадров */
} BANG;

/* описывает бомбу */
typedef struct {
	int on;                         /* бомба отображается */
	int ix,iy;                      /* целоцисленные координаты */
	int count;                      /* число кадров до создания бомбы */
	double vy;                      /* скорость бомбы (движется в плоскости 'y') */
	double y;                       /* координата 'y' */
} BOMB;

/* управляющие флаги призов */
#define F_POINTS                   (1<<0)
#define F_SMART_BOMB               (1<<1)
#define F_EXTRA_LIFE               (1<<2)
#define F_SLOW_BALL                (1<<3)
#define F_SMASH_BALL               (1<<4)
#define F_TRIPLE_BALL              (1<<5)
#define F_HAND                     (1<<6)
#define F_GUN                      (1<<7)
#define F_EXTENDED_RACKET          (1<<8)
#define F_ROCKET_PACK              (1<<9)
/* флаг старта игрового раунда */
#define F_START_RAUND              (1<<31)

/* проверка флага */
#define CHECK_FLAG(val,flags)       (val & flags)

/* описывает приз */
typedef struct {
	int on;                         /* приз отображается */
	int ix,iy;                      /* целочисленные координаты */
	unsigned int flags;             /* управляющие флаги */
	double vx,vy;                   /* скорости по соответствующим плоскостям */
	double x,y;                     /* координаты */
} BONUS;

/* описывает хлопок пульки */
typedef struct {
	int point;                      /* текущий изображаемый кадр */
	int count;                      /* счетчик повтореных кадров */
} BULLET_PLOP, *LPBULLET_PLOP;

/* описывает игровую анимацию */
typedef struct {
	int state;                      /* текущее состояние анимации */
	int count;                      /* счетчик состояния */
	int (*func)(void);              /* указатель на функцию обработчик анимации */
} ANIMA;

/* описывает осколки ракетки */
typedef struct {
	double vx,vy;                   /* проекции скоростей на 'x' и 'y' */
	double x,y;                     /* координаты */
} SLIVER, *LPSLIVER;

/* описывает ракету в следующую стадию */
typedef struct {
	int ix,iy;                      /* целочисленные координаты */
	double y;                       /* координата 'y' */
	double vy;                      /* скорость по 'y' */
} ROCKET;

/* призовые очки набранные игроком */
typedef struct {
	int up1,up2,hi;
	int addLifes;
} SCORE;

/* таблица кадров для птицы */
static const FRAME lpAlienBird[] = {
	{0,3},{1,3},{2,3},{3,3},
	{4,3},{3,3},{2,3},{1,3},
	{-1,0}   /* пограничный элемент */
};

/* таблица кадров для НЛО */
static const FRAME lpAlienUfo[] = {
	{0,3},{1,3},{2,3},{3,3},{4,3},
	{5,3},{4,3},{3,3},{2,3},{1,3},
	{-1,0}   /* пограничный элемент */
};

/* таблица кадров для взрыва */
static const FRAME lpBang[] = {
	{0,2},{1,2},{2,2},{3,2},{4,2},
	{3,2},{2,2},{1,2},{0,2},
	{-1,0}   /* пограничный элемент */
};

/* таблица кадров перехода ракетки из обычной в стреляющую */
static const FRAME lpNormalToGunRacket[] = {
	{0,1},{3,8},{4,8},{5,8},{6,8},{1,1},
	{-1,0}   /* пограничный элемент */
};

/* таблица кадров перехада ракетки из стреляющей в обычную */
static const FRAME lpGunToNormalRacket[] = {
	{1,1},{6,8},{5,8},{4,8},{3,8},{0,1},
	{-1,0}   /* пограничный элемент */
};

/* таблица кадров перехода ракетки из обычной в большую */
static const FRAME lpNormalToExtendedRacket[] = {
	{0,1},{7,2},{8,2},{9,2},{10,2},
	{11,2},{12,2},{13,2},{2,1},
	{-1,0}   /* пограничный элемент */
};

/* таблица кадров перехода ракетки из большой в обычною */
static const FRAME lpExtendedToNormalRacket[] = {
	{2,1},{13,2},{12,2},{11,2},{10,2},
	{9,2},{8,2},{7,2},{0,1},
	{-1,0}   /* пограничный элемент */
};

/* таблица кадров хлопка пульки */
static const FRAME lpBulletPlop[] = {
	{2,2},{3,2},{4,2},{5,2},
	{-1,0}   /* пограничный элемент */
};

/*
 * Таблица кадров анимации осколков
 * 0 - осколки первого типа
 * 1 - осколки второго типа
 * 2 - осколки третьего типа
 * 3 - осколки четвертого типа
 * 4 - осколки пятого типа
 */
static const FRAME lpFlySliverAnima[] = {
	{0,1},{0,35},{1,14},{2,7},{3,3},{4,2},{5,55},
	{-1,0}   /* пограничный элемент */
};

/*
 * Анимация старта игры
 * 0 - черный экран
 * 1 - игровое поле и сообщение о раунде
 * 2 - игровое поле, анимация двойных кубиков и сообщение о раунде
 */
static const FRAME lpStartRoundAnima[] = {
	{0,5},{1,60},{2,14},
	{-1,0}   /* пограничный элемент */
};

/*
 * Анимация окончания раунда, выполняет
 * функцию задержки времени.
 */
static const FRAME lpEndRoundAnima[] = {
	{0,40},
	{-1,0}   /* пограничный элемент */
};

/*
 * Анимация завершения паузы
 * 0 - отображается цифра 3
 * 1 - отображается цифра 2
 * 2 - отображается цифра 1
 */
static const FRAME lpPauseEndAnima[] = {
	{0,25},{1,25},{2,25},
	{-1,0}   /* пограничный элемент */
};

/*
 * Анимация бонуса полета на ракете. Здесь счетчик не
 * используется.
 * 0 - полет на ракете
 * 1 - подсчет бонусных очков
 */
static const FRAME lpFlyRocketAnima[] = {
	{0,0},{1,0},
	{-1,0}   /* пограничный элемент */
};

/*
 * 1 - желтый кирпич
 * 2 - розовый кирпич
 * 3 - красный кирпич
 * 4 - зеленый кирпич
 * 5 - синий кирпич
 * 6 - черный кирпич
 *
 * Добавочные значения к блокам:
 * 0x40 - блок разбивается после двух попаданий
 * 0x80 - блок не разбивается
 */
/* раунд 1 (15,12) */
static const UCHAR round0[] = {
	0,0,0,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0,0,0,    /* 1 */
	0,0,2,5,5,5,5,5,5,5,5,5,2,0,0,                               /* 2 */
	0,2,2,2,5,5,5,5,5,5,5,2,2,2,0,                               /* 3 */
	2,2,0x43,2,2,4,4,4,4,4,2,2,0x43,2,2,                         /* 4 */
	5,2,2,2,1,1,1,1,1,1,1,2,2,2,5,                               /* 5 */
	5,5,2,1,1,1,1,1,1,1,1,1,2,5,5,                               /* 6 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                               /* 7 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                               /* 8 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                               /* 9 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                               /* 10 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                               /* 11 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                                /* 12 */
};

/* раунд 2 (15,12) */
static const UCHAR round1[] = {
	0,0,0x46,0x46,0x46,0x46,0x46,0x46,0x46,0x46,0x46,0x46,0x46,0,0,   /* 1 */
	0,0x46,0x46,1,2,2,0,0,0,2,2,1,0x46,0x46,0,                        /* 2 */
	0x46,0x46,1,1,2,2,0,0,0,2,2,1,1,0x46,0x46,                        /* 3 */
	0x46,5,1,1,2,2,0,0,0,2,2,1,1,5,0x46,                              /* 4 */
	5,5,1,1,2,2,0,0,0,2,2,1,1,5,5,                                    /* 5 */
	5,5,1,0x46,0x46,0x46,0x46,0x46,0x46,0x46,0x46,0x46,1,5,5,         /* 6 */
	5,5,0x46,0x46,0,0,0,0,0,0,0,0x46,0x46,5,5,                        /* 7 */
	5,0x46,0x46,0,0,0,0,0,0,0,0,0,0x46,0x46,5,                        /* 8 */
	0x46,0x46,0,0,0,0,0,0,0,0,0,0,0,0x46,0x46,                        /* 9 */
	0x46,0,0,0,0,0,0,0,0,0,0,0,0,0,0x46,                              /* 10 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                    /* 11 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                                     /* 12 */
};

/* раунд 3 (15,12) */
static const UCHAR round2[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                       /* 1 */
	0x46,0x46,0x46,0x46,0x46,0x83,0,0,0,0x83,0x46,0x46,0x46,0x46,0x46,   /* 2 */
	0,0,0,0,0,0x83,0,0,0,0x83,0,0,0,0,0,                                 /* 3 */
	0,0,0,0,0,0x83,0,0,0,0x83,0,0,0,0,0,                                 /* 4 */
	0,0x83,0x83,0x83,0x83,0x83,0,2,0,0x83,0x83,0x83,0x83,0x83,0,         /* 5 */
	0,0x83,1,0,0,0,2,2,2,0,0,0,1,0x83,0,                                 /* 6 */
	0,0x83,1,0,0,0,2,2,2,0,0,0,1,0x83,0,                                 /* 7 */
	0,0x83,0x83,0x83,0x83,0x83,0,2,0,0x83,0x83,0x83,0x83,0x83,0,         /* 8 */
	0,0,0,0,5,0x83,0,0,0,0x83,5,0,0,0,0,                                 /* 9 */
	0,0,0,0,5,0x83,0,0,0,0x83,5,0,0,0,0,                                 /* 10 */
	0x83,0x83,0x83,0x83,0x83,0x83,0,0,0,0x83,0x83,0x83,0x83,0x83,0x83,   /* 11 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                                        /* 12 */
};

/* раунд 4 (15,12) */
static const UCHAR round3[] = {
	0,0,4,4,2,2,0,0,0,2,2,4,4,0,0,              /* 1 */
	0,4,4,2,2,0x46,3,0,3,0x46,2,2,4,4,0,        /* 2 */
	0,4,2,2,0x46,3,3,0,3,3,0x46,2,2,4,0,        /* 3 */
	0,2,2,0x46,3,3,1,0,1,3,3,0x46,2,2,0,        /* 4 */
	0,2,0x46,3,3,1,5,0,5,1,3,3,0x46,2,0,        /* 5 */
	0,0x46,3,0,0,5,5,0,5,5,0,0,3,0x46,0,        /* 6 */
	0,3,3,0,0,5,0x46,0,0x46,5,0,0,3,3,0,        /* 7 */
	0,3,1,5,5,0x46,2,0,2,0x46,5,5,1,3,0,        /* 8 */
	0,1,5,5,0x46,2,2,0,2,2,0x46,5,5,1,0,        /* 9 */
	0,5,5,0x46,2,2,4,0,4,2,2,0x46,5,5,0,        /* 10 */
	0,5,0x46,2,2,4,4,0,4,4,2,2,0x46,5,0,        /* 11 */
	0,0,2,2,4,4,0,0,0,4,4,2,2,0,0               /* 12 */
};

/* раунд 5 (15,12) */
static const UCHAR round4[] = {
	0x82,0x82,0x82,0x82,0,0,0x82,0x82,0x82,0,0,0x82,0x82,0x82,0x82,              /* 1 */
	0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,  /* 2 */
	0,0,0,0,0x82,0,0,0,0,0,0x82,0,0,0,0x82,                                      /* 3 */
	0,0,0,0,0x82,0,0,0,0,0,0x82,0,0,0,0x82,                                      /* 4 */
	0,0x82,3,0,0x82,0,5,0x82,4,0,0x82,0,0x82,0,0x82,                             /* 5 */
	0,0x82,0,0,0x82,0,0,0x82,0,0,0x82,1,0x82,1,0x82,                             /* 6 */
	0,0x82,0,0,0x82,0,0,0x82,0,0,0x82,0,0x82,0,0x82,                             /* 7 */
	0,0x82,0,3,0x82,5,0,0x82,0,4,0x82,0,0x82,0,0x82,                             /* 8 */
	0,0x82,0,0,0,0,0,0x82,0,0,0,0,0x82,0,0x82,                                   /* 9 */
	0,0x82,0,0,0,0,0,0x82,0,0,0,0,0x82,1,0x82,                                   /* 10 */
	0,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,     /* 11 */
	0,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82      /* 12 */
};

/* раунд 6 (15,12) */
static const UCHAR round5[] = {
	0,0,0x43,0x43,0,0,0,0,0,0,0,0x43,0x43,0,0,                                /* 1 */
	0,0x43,0x43,0x43,0x43,0x43,0,0,0,0x43,0x43,0x43,0x43,0x43,0,              /* 2 */
	0,0x43,0x46,0x46,0x43,0x43,0x43,0,0x43,0x43,0x43,0x46,0x46,0x43,0,        /* 3 */
	0,0x43,0x46,1,0x46,0x43,0x43,0,0x43,0x43,0x46,1,0x46,0x43,0,              /* 4 */
	0,0x43,0x43,0x46,0x46,0x43,0x43,0x43,0x43,0x43,0x46,0x46,0x43,0x43,0,     /* 5 */
	0,0,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0,0,           /* 6 */
	0,0,0,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0,0,0,                 /* 7 */
	0,0,0,0,0x43,0x43,1,0x43,1,0x43,0x43,0,0,0,0,                             /* 8 */
	0,0,0x43,0x43,0x43,0x43,1,0,1,0x43,0x43,0x43,0x43,0,0,                    /* 9 */
	0,0x43,0x43,0x43,0x43,0,1,0,1,0,0x43,0x43,0x43,0x43,0,                    /* 10 */
	0x43,0x43,0x43,0,0,0,0,0,0,0,0,0,0x43,0x43,0x43,                          /* 11 */
	0x43,0x43,0,0,0,0,0,0,0,0,0,0,0,0x43,0x43                                 /* 12 */
};

/* раунд 7 (15,12) */
static const UCHAR round6[] = {
	0,0x86,0,0,0,0,0,0,0,0,0,0,0,0,0,                                         /* 1 */
	0,0x86,0,0,0,0,0,0,0,0,0,0,0,0,0,                                         /* 2 */
	0,0x86,0,0,0,0,0,3,3,3,0,0,0,0,0,                                         /* 3 */
	0,0x86,0,0,0,3,3,3,1,3,3,3,0,0,0,                                         /* 4 */
	0,0x86,0,0,3,3,1,1,0x45,1,1,3,3,0,0,                                      /* 5 */
	0,0x86,0,3,3,1,1,0x45,0x45,0x45,1,1,3,3,0,                                /* 6 */
	0,0x86,0,0,3,3,1,1,0x45,1,1,3,3,0,0,                                      /* 7 */
	0,0x86,0,0,0,3,3,3,1,3,3,3,0,0,0,                                         /* 8 */
	0,0x86,0,0,0,0,0,3,3,3,0,0,0,0,0,                                         /* 9 */
	0,0x86,0,0,0,0,0,0,0,0,0,0,0,0,0,                                         /* 10 */
	0,0x86,0,0,0,0,0,0,0,0,0,0,0,0,0,                                         /* 11 */
	0,0x86,0x86,0x86,0x86,0x86,0x86,0x86,0x86,0x86,0x86,0x86,0x86,0x86,0x86   /* 12 */
};

/* раунд 8 (15,12) */
static const UCHAR round7[] = {
	0,0x86,0,0x86,0,0,0,0,0,0,0,0x86,0,0x86,0,           /* 1 */
	0,0x86,1,0x86,0,0,0,0,0,0,0,0x86,1,0x86,0,           /* 2 */
	0,0x86,4,0x86,0,0,0,0,0,0,0,0x86,4,0x86,0,           /* 3 */
	0,0x86,0x86,0x86,0,0,0,0,0,0,0,0x86,0x86,0x86,0,     /* 4 */
	0,0,0,0,0,0,0x42,0x42,0x42,0,0,0,0,0,0,              /* 5 */
	0,0,0,0,0x42,0x42,4,4,4,0x42,0x42,0,0,0,0,           /* 6 */
	0,0,0x42,0x42,4,4,1,1,1,4,4,0x42,0x42,0,0,           /* 7 */
	0,0,0x42,0x42,4,4,1,1,1,4,4,0x42,0x42,0,0,           /* 8 */
	0,0,0,0,0x42,0x42,4,4,4,0x42,0x42,0,0,0,0,           /* 9 */
	0x45,0,0,0,0,0,0x42,0x42,0x42,0,0,0,0,0,0x45,        /* 10 */
	0,0x45,0,0,0,0,0,0,0,0,0,0,0,0x45,0,                 /* 11 */
	0,0,0x45,0,0,0,0,0,0,0,0,0,0x45,0,0                  /* 12 */
};

/* раунд 9 (15,12) */
static const UCHAR round8[] = {
	0,0,0x83,0,0,0,0,0,0,0,0,0,0x83,0,0,                    /* 1 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                          /* 2 */
	0x83,0,0x46,0,0x83,0,0,0,0,0,0x83,0,0x46,0,0x83,        /* 3 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                          /* 4 */
	0,0,0x83,0,0,0,0,0,0,0,0,0,0x83,0,0,                    /* 5 */
	0,0,0,0,0,0,0,0x85,0,0,0,0,0,0,0,                       /* 6 */
	0,0,0,0,0,0,0,0x42,0,0,0,0,0,0,0,                       /* 7 */
	0,0x85,0,0,0,0x85,0x42,0x46,0x42,0x85,0,0,0,0x85,0,     /* 8 */
	0,0,0,0,0,0,0,0x42,0,0,0,0,0,0,0,                       /* 9 */
	0,0,0,0x85,0,0,0,0x85,0,0,0,0x85,0,0,0,                 /* 10 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                          /* 11 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                           /* 12 */
};

/* раунд 10 (15,12) */
static const UCHAR round9[] = {
	0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,                                          /* 1 */
	0,0,1,4,1,0,0,0,0,0,1,4,1,0,0,                                          /* 2 */
	0,1,4,0x45,4,1,0,0,0,1,4,0x45,4,1,0,                                    /* 3 */
	1,4,0x45,0x42,0x45,4,1,0,1,4,0x45,0x42,0x45,4,1,                        /* 4 */
	0,1,4,0x45,4,1,0,0,0,1,4,0x45,4,1,0,                                    /* 5 */
	0,0,1,4,1,0,0,0,0,0,1,4,1,0,0,                                          /* 6 */
	0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,                                          /* 7 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                          /* 8 */
	0x86,0x86,0,0x86,0x86,0x86,0x86,0x86,0x86,0x86,0x86,0x86,0,0x86,0x86,   /* 9 */
	0,0,0,0x86,0,0,0,0,0,0,0,0x86,0,0,0,                                    /* 10 */
	0,0,0,0x86,0,0,0,0,0,0,0,0x86,0,0,0,                                    /* 11 */
	0,0x86,0x86,0x86,0,0,0,0,0,0,0,0x86,0x86,0x86,0                         /* 12 */
};

/* раунд 11 (15,12) */
static const UCHAR round10[] = {
	0,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0,   /* 1 */
	0,0x43,0,0,0,0,0,0,0,0,0,0,0,0x43,0,                                    /* 2 */
	0,0x43,0,0,0,0,0,0,0,0,0,0,0,0x43,0,                                    /* 3 */
	0,0x43,0,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0,0x43,0,         /* 4 */
	0,0x43,0,0x45,0,0,0,0,0,0,0,0x45,0,0x43,0,                              /* 5 */
	0,0x43,0,0x45,0,0x42,0x42,0x42,0x42,0x42,0,0x45,0,0x43,0,               /* 6 */
	0,0x43,0,0x45,0,0x42,0x42,0x42,0x42,0x42,0,0x45,0,0x43,0,               /* 7 */
	0,0x43,0,0x45,0,0,0,0,0,0,0,0x45,0,0x43,0,                              /* 8 */
	0,0x43,0,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0,0x43,0,         /* 9 */
	0,0x43,0,0,0,0,0,0,0,0,0,0,0,0x43,0,                                    /* 10 */
	0,0x43,0,0,0,0,0,0,0,0,0,0,0,0x43,0,                                    /* 11 */
	0,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0    /* 12 */
};

/* раунд 12 (15,12) */
static const UCHAR round11[] = {
	0,0,0x86,0x45,0x43,4,0,0,0,4,0x43,0x45,0x86,0,0,        /* 1 */
	0,0,0x86,1,0x45,0x43,4,0,4,0x43,0x45,1,0x86,0,0,        /* 2 */
	0,0,0,0x86,1,0x45,0x43,4,0x43,0x45,1,0x86,0,0,0,        /* 3 */
	0,0,0,0,0x86,1,0x45,0x43,0x45,1,0x86,0,0,0,0,           /* 4 */
	0,0,0,0,0,0x86,1,0x45,1,0x86,0,0,0,0,0,                 /* 5 */
	0,0,0,0,0,0x86,1,0x45,1,0x86,0,0,0,0,0,                 /* 6 */
	0,0,0,0,0,0x86,1,0x45,1,0x86,0,0,0,0,0,                 /* 7 */
	0,0,0,0,0,0x86,1,0x45,1,0x86,0,0,0,0,0,                 /* 8 */
	0,0,0,0,0,0x86,1,0x45,1,0x86,0,0,0,0,0,                 /* 9 */
	0,0,0,0,0,0x86,1,0x45,1,0x86,0,0,0,0,0,                 /* 10 */
	0,0,0,0,0,0x86,1,1,1,0x86,0,0,0,0,0,                    /* 11 */
	0,0,0,0,0,0x86,0x86,0x86,0x86,0x86,0,0,0,0,0            /* 12 */
};

/* раунд 13 (15,12) */
static const UCHAR round12[] = {
	0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,                                    /* 1 */
	0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,                                    /* 2 */
	0,0,0,0,0,0,4,4,4,0,0,0,0,0,0,                                    /* 3 */
	0,0,0,5,5,5,5,5,5,5,5,5,0,0,0,                                    /* 4 */
	0,0,5,5,5,5,5,5,5,5,5,5,5,0,0,                                    /* 5 */
	0,0x46,0x46,2,0x46,0x46,0x46,2,0x46,0x46,0x46,2,0x46,0x46,0,      /* 6 */
	0,0x46,0x46,2,0x46,0x46,0x46,2,0x46,0x46,0x46,2,0x46,0x46,0,      /* 7 */
	0,0,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0,0,   /* 8 */
	0,0,0,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0,0,0,         /* 9 */
	0,0,0,0,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0,0,0,0,               /* 10 */
	0,0,0,0,0,0x43,0x43,0x43,0x43,0x43,0,0,0,0,0,                     /* 11 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                                     /* 12 */
};

/* раунд 14 (15,12) */
static const UCHAR round13[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                    /* 1 */
	0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,                    /* 2 */
	0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,                    /* 3 */
	0,1,1,0x43,1,1,1,0,0,0,0,0,0,0,0,                 /* 4 */
	1,1,1,0x43,1,1,0,0,0,0,0,0,0,0,0,                 /* 5 */
	1,1,1,1,1,0,0,0x45,0,0x45,0,0x45,0,0x45,0,        /* 6 */
	1,1,1,1,0,0,0,0x45,0,0x45,0,0x45,0,0x45,0,        /* 7 */
	1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,                    /* 8 */
	1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,                    /* 9 */
	0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,                    /* 10 */
	0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,                    /* 11 */
	0,0,1,1,1,0,0,0,0,0,0,0,0,0,0                     /* 12 */
};

/* раунд 15 (15,12) */
static const UCHAR round14[] = {
	0x45,0x45,0x45,0,0x43,0,0,0x42,0,0x43,0x43,0x43,0,0x45,0x45,        /* 1 */
	0x45,0x45,0x45,0,0x43,0,0,0x42,0,0x43,0x43,0x43,0,0x45,0x45,        /* 2 */
	0x45,0,0,0,0x43,0,0,0x42,0,0,0x43,0,0,0x45,0,                       /* 3 */
	0x45,0,0,0,0x43,0,0,0x42,0,0,0x43,0,0,0x45,0,                       /* 4 */
	0x45,0,0,0,0x43,0,0,0x42,0,0,0x43,0,0,0x45,0,                       /* 5 */
	0x45,0x45,0x45,0,0x43,0,0,0x42,0,0,0x43,0,0,0x45,0x45,              /* 6 */
	0x45,0x45,0x45,0,0x43,0,0,0x42,0,0,0x43,0,0,0x45,0x45,              /* 7 */
	0x45,0,0,0,0x43,0,0,0x42,0,0,0x43,0,0,0x45,0,                       /* 8 */
	0x45,0,0,0,0x43,0,0,0x42,0,0,0x43,0,0,0x45,0,                       /* 9 */
	0x45,0,0,0,0x43,0,0,0x42,0,0,0x43,0,0,0x45,0,                       /* 10 */
	0x45,0x45,0x45,0,0x43,0x43,0,0x42,0,0,0x43,0,0,0x45,0x45,           /* 11 */
	0x45,0x45,0x45,0,0x43,0x43,0,0x42,0,0,0x43,0,0,0x45,0x45            /* 12 */
};

/* описывает создаваемый уровень */
typedef struct {
	/*
	 * Количество оставшихся блоков при которых
	 * появляется летающее существо.
	 *
	 * 0  - никогда не появляется
	 */
	int AlienBlock;
	/* описывает первый круг откланения */
	int swer0,swer0X,swer0Y;
	/* описывает второй круг откланения */
	int swer1,swer1X,swer1Y;
	/* описывает третий круг откланения */
	int swer2,swer2X,swer2Y;
	/* описывает четвертый круг откланения */
	int swer3,swer3X,swer3Y;
	/* указатель на создаваемое поле */
	const UCHAR *round;
} LEVEL;

/* количество уровней */
#define LEVEL_COUNT                 15

/* описывает создаваемые уровни */
static const LEVEL levels[LEVEL_COUNT] = {
	{43, 0,0,0, 0,0,0, 0,0,0, 0,0,0, round0},
	{43, 1,116,44, 0,0,0, 0,0,0, 0,0,0, round1},
	{43, 0,0,0, 0,0,0, 0,0,0, 0,0,0, round2},
	{43, 1,116,124, 0,0,0, 0,0,0, 0,0,0, round3},
	{0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, round4},
	{43, 1,72,115, 1,116,16, 1,160,115, 0,0,0, round5},
	{43, 1,48,92, 1,216,92, 0,0,0, 0,0,0, round6},
	{43, 1,76,116, 1,116,24, 1,156,116, 0,0,0, round7},
	{43, 1,64,60, 1,84,108, 1,148,108, 1,168,60, round8},
	{43, 1,116,68, 0,0,0, 0,0,0, 0,0,0, round9},
	{43, 1,92,132, 1,140,132, 0,0,0, 0,0,0, round10},
	{43, 1,32,68, 1,116,8, 1,200,68, 0,0,0, round11},
	{43, 1,16,32, 1,24,108, 1,208,108, 1,216,32, round12},
	{43, 1,140,36, 1,140,100, 1,196,36, 1,196,100, round13},
	{43, 1,76,130, 1,156,130, 0,0,0, 0,0,0, round14},
};

/* хранит указатель на текущий раунд */
static const UCHAR *raund;
/* хранит номер уровня */
static int nLevel;
/*
 * Поле с кирпичами размерностью (15,16)
 * 
 * Т.к. раунды храняться в массиве размерностью (15,12),
 * поэтому в массиве 'block' первые 3 и последня 1 строчки
 * пустые.
 */
static UCHAR block[16][15];
/* хранит описание событий связаных с блоками */
static int EventBlock[16][15];
/* круги откланения */
static SWERVE swerve[SWERVES_COUNT];
/* мячики */
static BALL ball[BALLS_COUNT];
/* ракетка */
static RACKET racket;
/* летающее существо */
static ALIEN alien;
/* взрыв */
static BANG bang;
/* бомба */
static BOMB bomb;
/* приз */
static BONUS bonus;
/* игровая анимация */
static ANIMA anima;
/* ракета в следующий раунд */
static ROCKET rocket;
/* осколки ракеток */
static SLIVER sliver[SLIVERS_COUNT];
/* анимация хлопка пульки */
static BULLET_PLOP plop[BULLETS_COUNT];
/* призовые очки набранные игроком */
static SCORE score;
/* количество оставшихся жизней */
static int lifes;

/* время через которое увеличится скорость мячика */
static int timeAddSpeedBall;
/* время через которое закончится действие приза 'BONUS_SMASH_BALL' */
static int timeEndingSmashBall;

/* описывает паузу */
static struct {
	int deny;               /* флаг запрета доступа к паузе */
	int pause;              /* флаг паузы */
} pause;

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

/* возвращает случайное число из заданного диапозона */
static int random(int range)
{
	return (range * rand() / (RAND_MAX + 1));
}

/* производит копирование блоков для отображения */
static void CopyBlock(void)
{
	int i,j;
	UCHAR *to,*from;
	UCHAR symbol;
	int *event;

	/* переберем все блоки */
	to = &scene.block[0][0];
	from = &block[3][0];
	event = &EventBlock[3][0];
	for (i = 0; i < 12; i++) {
		/* пробежим по строке */
		for (j = 0; j < 15; j++) {
			int evn = event[j];
			symbol = from[j] & 0x0f;
			/* обработаем событие для данного блока */
			if (evn) {
				/* моргнем блоком */
				if (evn <= 2) {
					to[j] = 0x10 | symbol;
					event[j]++;
				} else if (evn <= 4) {
					to[j] = 0x20 | symbol;
					event[j]++;
				} else if (evn <= 6) {
					to[j] = 0x30 | symbol;
					event[j]++;
				} else if (evn <= 8) {
					to[j] = 0x40 | symbol;
					event[j]++;
				} else if (evn <= 10) {
					to[j] = 0x50 | symbol;
					event[j]++;
				} else if (evn <= 14) {
					to[j] = 0x60 | symbol;
					event[j]++;
				} else {
					/* сбросим событие */
					to[j] = symbol;
					event[j] = 0;
				}
			} else {
				/* нет событий, копируем блок */
				to[j] = symbol;
			}
		}
		/* перемещаем указатели */
		to += 15;
		from += 15;
		event += 15;
	}
}

/* возвращает ширину бонуса */
static int GetWidthBonus(void)
{
	int width;

	switch (scene.pBonus.type) {
		case BONUS_POINTS:          width = 23; break;
		case BONUS_SMART_BOMB:      width = 24; break;
		case BONUS_EXTRA_LIFE:      width = 19; break;
		case BONUS_SLOW_BALL:       width = 22; break;
		case BONUS_SMASH_BALL:      width = 23; break;
		case BONUS_TRIPLE_BALL:     width = 17; break;
		case BONUS_HAND:            width = 22; break;
		case BONUS_GUN:             width = 20; break;
		case BONUS_EXTENDED_RACKET: width = 22; break;
		case BONUS_ROCKET_PACK:     width = 21; break;
		case BONUS_400_POINTS:      width = 22; break;
		default: width = 0;
	}

	return width;
}

/* возвращает высоту бонуса */
static int GetHeightBonus(void)
{
	int height;

	switch (scene.pBonus.type) {
		case BONUS_POINTS:          height = 14; break;
		case BONUS_SMART_BOMB:      height = 15; break;
		case BONUS_EXTRA_LIFE:      height = 13; break;
		case BONUS_SLOW_BALL:       height = 15; break;
		case BONUS_SMASH_BALL:      height = 15; break;
		case BONUS_TRIPLE_BALL:     height = 15; break;
		case BONUS_HAND:            height = 12; break;
		case BONUS_GUN:             height = 9;  break;
		case BONUS_EXTENDED_RACKET: height = 11; break;
		case BONUS_ROCKET_PACK:     height = 12; break;
		case BONUS_400_POINTS:      height = 10; break;
		default: height = 0;
	}

	return height;
}

/*
 * Возвращает случайное значение типа бонуса.
 *
 * Основные требования к функции:
 * 1 - Призы не должны повторяться.
 * 2 - Если приз сейчас действует то он не должен появляться.
 * 3 - Бонус BONUS_EXTRA_LIFE может появиться только один раз за раунд.
 * 3 - Вероятность появления призов:
 *     a - BONUS_POINTS, BONUS_SMART_BOMB, BONUS_SLOW_BALL  = 12%
 *     b - BONUS_TRIPLE_BALL, BONUS_SMASH_BALL              = 11%
 *     c - BONUS_HAND, BONUS_GUN, BONUS_EXTENDED_RACKET     = 10%
 *     d - BONUS_EXTRA_LIFE                                 =  7%
 *     e - BONUS_ROCKET_PACK                                =  5%
 *                                                          -----
 *                                                           100%
 */
static int GetRandomBonus(void)
{
	char bBonus[100];
	int count;

	/* заполняем массив призами */
	count = 0;
	if (!CHECK_FLAG(bonus.flags,F_ROCKET_PACK)) {
		memset(bBonus+count,BONUS_ROCKET_PACK,5);
		count += 5;
	}
	if (!CHECK_FLAG(bonus.flags,F_EXTRA_LIFE)) {
		memset(bBonus+count,BONUS_EXTRA_LIFE,7);
		count += 7;
	}
	if (!CHECK_FLAG(bonus.flags,F_HAND)) {
		memset(bBonus+count,BONUS_HAND,10);
		count += 10;
	}
	if (!CHECK_FLAG(bonus.flags,F_GUN)) {
		memset(bBonus+count,BONUS_GUN,10);
		count += 10;
	}
	if (!CHECK_FLAG(bonus.flags,F_EXTENDED_RACKET)) {
		memset(bBonus+count,BONUS_EXTENDED_RACKET,10);
		count += 10;
	}
	if (!CHECK_FLAG(bonus.flags,F_TRIPLE_BALL)) {
		memset(bBonus+count,BONUS_TRIPLE_BALL,11);
		count += 11;
	}
	if (!CHECK_FLAG(bonus.flags,F_SMASH_BALL)) {
		memset(bBonus+count,BONUS_SMASH_BALL,11);
		count += 11;
	}
	if (!CHECK_FLAG(bonus.flags,F_POINTS)) {
		memset(bBonus+count,BONUS_POINTS,12);
		count += 12;
	}
	if (!CHECK_FLAG(bonus.flags,F_SMART_BOMB)) {
		memset(bBonus+count,BONUS_SMART_BOMB,12);
		count += 12;
	}
	if (!CHECK_FLAG(bonus.flags,F_SLOW_BALL)) {
		memset(bBonus+count,BONUS_SLOW_BALL,12);
		count += 12;
	}

	return bBonus[random(count)];
}

/* обрабатывает возможность создания бонуса */
static void CreateBonus(int x, int y)
{
	/* если приз отображается то уходим */
	if (bonus.on)
		return;

	/* вероятность появления приза равна 14.28% */
	if (random(7))
		return;

	/* создадим приз */
	scene.pBonus.type = GetRandomBonus();
	/* расчитаем его параметры */
	bonus.on = 1;
	bonus.ix = x * 16 + LOGIC_X - (GetWidthBonus() - 16) / 2;
	bonus.iy = y * 8 + LOGIC_Y;
	bonus.x = bonus.ix;
	bonus.y = bonus.iy;
	bonus.vy = 0.0;
	/* отобразим приз */
	scene.pBonus.show = 1;
	scene.pBonus.X = bonus.ix;
	scene.pBonus.Y = bonus.iy;
}

/* увеличивает призовые очки */
static void AddScore(int sum)
{
	score.up1 += sum;
	if (score.up1 > score.hi)
		score.hi = score.up1;
	if (score.up1 >= score.addLifes) {
		lifes++;
		score.addLifes += 30000;
	}
}

/*
 * Расчитывает призовые очки в зависимости от положения блока.
 * Самая верхняя строчка стоит 120, строчка ниже 110, строчка еще
 * ниже 100 и т.д.
 */
static void BlockScore(int x, int y)
{
	const UCHAR *block;
	int point;

	/* расчитаем сумму призовых очков */
	y -= 3;
	point = 10 * (12 - y);
	/* если блок разбивается со второго раза удвоим очки */
	block = raund + y * 15 + x;
	if (*block & 0x40)
		point *= 2;

	AddScore(point);
}

/* обрабатываем столкновение с блоком */
static int MarkBlock(int x, int y)
{
	UCHAR *point;
	int attr;

	/* проверяем блок */
	point = &block[y][x];
	if (*point == 0)
		return 0;

	/* выделим атрибуты блока */
	attr = *point & 0xf0;
	/* блок не разбивается */
	if (attr == 0x80) {
		EventBlock[y][x] = 1;
		play.stone = 1;
		return 0;
	}

	/* блок разбивается со второго удара */
	if (attr == 0x40) {
		/* проверим действие бонуса 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL)) {
			*point = 0;
			EventBlock[y][x] = 0;
			BlockScore(x,y);
			CreateBonus(x,y);
			play.brik = 1;
		} else {
			/* блок разбивается со второго удара */
			EventBlock[y][x] = 1;
			*point &= 0x0f;
			play.stone = 1;
		}
	} else {
		/* обычный блок */
		*point = 0;
		EventBlock[y][x] = 0;
		BlockScore(x,y);
		CreateBonus(x,y);
		play.brik = 1;
	}

	return 1;
}

/* расчитывает скорость по осям 'x' и 'y' для шарика */
static void SetSpeedBall(LPBALL lpBall)
{
	double rad;

	/* переведем углы в радианы */
	rad = lpBall->angle * PI / 180.0;
	/* расчитаем скорость по 'x' и 'y' */
	lpBall->vx = lpBall->v * cos(rad);
	lpBall->vy = lpBall->v * sin(rad);
}

/* если точка принадлежит блоку, то возвращается его положение */
static int GetBlock(LPBOUND bound, int px, int py)
{
	int x,y;

	/* сформируем указатель и проверим его */
	x = px - LOGIC_X;
	y = py - LOGIC_Y;
	if ((x < 0) || (y < 0))
		return 0;
	x /= 16;
	y /= 8;
	if ((x >= 15) || (y >= 16))
		return 0;

	/* проверим, что блок существует */
	if (!block[y][x])
		return 0;

	/* сформируем структуру описывающую положение блока */
	bound->x = x;
	bound->y = y;
	bound->offset_x = px - (x * 16 + LOGIC_X);
	bound->offset_y = py - (y * 8 + LOGIC_Y);

	return 1;
}

/* возвращает ширину ракетки */
static int GetWidthRacket(void)
{
	int width;

	/* взависимости от типа ракетки возвратим ее ширину */
	switch (scene.pRacket.type) {
		case TYPE_RACKET_N_G_1:
		case TYPE_RACKET_N_G_2:
			width = 26;
			break;
		case TYPE_RACKET_N_E_1:
			width = 30;
			break;
		case TYPE_RACKET_N_E_2:
			width = 32;
			break;
		case TYPE_RACKET_N_E_3:
			width = 34;
			break;
		case TYPE_RACKET_N_E_4:
			width = 36;
			break;
		case TYPE_RACKET_N_E_5:
			width = 38;
			break;
		case TYPE_RACKET_N_E_6:
			width = 40;
			break;
		case TYPE_RACKET_N_E_7:
			width = 42;
			break;
		case TYPE_RACKET_EXTEND:
			width = 44;
			break;
		case TYPE_RACKET_NORMAL:
		case TYPE_RACKET_GUN:
		case TYPE_RACKET_N_G_3:
		case TYPE_RACKET_N_G_4:
		default:
			width = 28;
			break;
	}

	return width;
}

/* возвращает высоту ракетки */
static int GetHeightRacket(void)
{
	/* у всех ракеток высота равна 10 */
	return 10;
}

/* возвращает ширину мячика */
static int GetWidthBall(void)
{
	/* возвратим размер по умолчанию */
	return 7;
}

/* установить в начальную позицию ракетку */
static void SetRacket(void)
{
	/* сбросим структуру рвкетки */
	memset(&racket,0,sizeof(racket));
	memset(&scene.pRacket,0,sizeof(scene.pRacket));

	/* установим тип ракетки и ее положение */
	racket.on = 1;
	racket.x = RACKET_X;
	racket.y = RACKET_Y;
	/* отобразим ракетку */
	scene.pRacket.show = 1;
	scene.pRacket.type = TYPE_RACKET_NORMAL;
	scene.pRacket.X = racket.x;
	scene.pRacket.Y = racket.y;
}

/* установить в начальную позицию мячики */
static void SetBall(void)
{
	/* сбросим все изображения и описания мячиков */
	memset(scene.pBall,0,sizeof(scene.pBall));
	memset(ball,0,sizeof(ball));

	/* установим тип и положение мячика */
	scene.pBall[0].picture.show = 1;
	scene.pBall[0].picture.type = BALL_NORMAL;
	scene.pBall[0].picture.X = racket.x + 12;
	scene.pBall[0].picture.Y = racket.y - 6;
	/* установим тень от мячика */
	scene.pBall[0].shadow.show = 1;
	scene.pBall[0].shadow.type = BALL_NORMAL;
	scene.pBall[0].shadow.X = scene.pBall[0].picture.X + GetWidthBall();
	scene.pBall[0].shadow.Y = scene.pBall[0].picture.Y + GetWidthBall();

	/* расчитаем начальные скорости мячика */
	ball[0].on = 1;
	ball[0].count = TIME_NEW_COURSE_BALL;
	ball[0].angle = 293;
	ball[0].v = START_SPEED_BALL;
	SetSpeedBall(&ball[0]);
	ball[0].x = scene.pBall[0].picture.X;
	ball[0].y = scene.pBall[0].picture.Y;

	/* установим счетчик увеличения скорости */
	timeAddSpeedBall = TIME_ADD_SPEED_BALL;
}

/* установить в начальную позицию круги откланения */
static void SetSwerve(void)
{
	int i;
	const LEVEL *lpLevel;

	/* сбросим все изображения и описания кругов */
	memset(scene.pSwerves,0,sizeof(scene.pSwerves));
	memset(swerve,0,sizeof(swerve));

	/* определим загружаемый уровень */
	lpLevel = &levels[nLevel%LEVEL_COUNT];

	/* загрузим круги откланения */
	for (i = 0; i < SWERVES_COUNT; i++) {
		LPSWERVE lpSwerve;
		LPPICTURE lpPicture;
		int flags,X,Y;

		/* расчитаем флаг отображения, и координаты */
		switch (i) {
			case 0:
				flags = lpLevel->swer0;
				X = lpLevel->swer0X;
				Y = lpLevel->swer0Y;
				break;

			case 1:
				flags = lpLevel->swer1;
				X = lpLevel->swer1X;
				Y = lpLevel->swer1Y;
				break;

			case 2:
				flags = lpLevel->swer2;
				X = lpLevel->swer2X;
				Y = lpLevel->swer2Y;
				break;

			case 3:
				flags = lpLevel->swer3;
				X = lpLevel->swer3X;
				Y = lpLevel->swer3Y;
				break;

			default:
				continue;
		}

		/* загрузим указатели */
		lpSwerve = &swerve[i];
		lpPicture = &scene.pSwerves[i];

		/* загружаем значения для круга */
		if (flags) {
			/* круг отображается */
			lpPicture->show = 1;
			/* установим состояние */
			if (random(2)) {
				lpPicture->type = PICTURE_SWERVE_ON;
				lpSwerve->on = 1;
			} else {
				lpPicture->type = PICTURE_SWERVE_OFF;
				lpSwerve->on = 0;
			}
			/* установим координаты */
			lpPicture->X = X;
			lpPicture->Y = Y;
			/* запомним центральную точку круга откланения */
			lpSwerve->x = X + 12;
			lpSwerve->y = Y + 12;
			lpSwerve->count = 30 + random(25);
		}
	}
}

/* удаляет летающее существо, и формирует анимацию взрыва */
static void CreateBang(void)
{
	/* удалим летающее существо */
	alien.on = 0;
	scene.pAlien.show = 0;
	/* сформируем анимацию взрыва */
	bang.on = 1;
	bang.count = 0;
	bang.point = 0;
	scene.pBang.show = 1;
	scene.pBang.type = lpBang[bang.point].number;
	scene.pBang.X = alien.ix + 4;
	scene.pBang.Y = alien.iy - 3;
}

/* удаляет все объекты */
static void CleanObject(void)
{
	int i;

	/* удалим мячики */
	for (i = 0; i < BALLS_COUNT; i++) {
		LPFULL_PICTURE lpPicture = &scene.pBall[i];
		lpPicture->picture.show = 0;
		lpPicture->shadow.show = 0;
		ball[i].on = 0;
	}
	/* удалим пульки */
	for (i = 0; i < BULLETS_COUNT; i++)
		scene.pBullet[i].show = 0;
	/* удалим ракетку */
	racket.on = 0;
	scene.pRacket.show = 0;
	/* удалим летающее существо */
	alien.on = 0;
	scene.pAlien.show = 0;
	/* удалим взрыв */
	bang.on = 0;
	scene.pBang.show = 0;
	/* удалим бонус */
	bonus.on = 0;
	scene.pBonus.show = 0;
	/* удалим бомбу */
	bomb.on = 0;
	scene.pBomb.picture.show = 0;
	scene.pBomb.shadow.show = 0;
}

/* анимация окончания раунда */
static int EndRoundAnima(void)
{
	/* увеличим счетчик состояния */
	if (++anima.count < lpEndRoundAnima[anima.state].count)
		/* заставим возвратить нуль (идет игра) */
		return 2;

	/* удаляем функцию анимации */
	anima.func = NULL;
	/* возвратим, что раунд пройден */
	return 3;
}

/* запускает анимацию окончания раунда */
static void SetEndRoundAnima(void)
{
	/* подготовим структуру анимации */
	anima.func = EndRoundAnima;
	anima.count = 0;
	anima.state = 0;
	/* запретим паузу */
	pause.deny = 1;
	/* удалим все объекты */
	CleanObject();
}

/* анимации старта раунда */
static int StartRoundAnima(void)
{
	int state;

	/* увеличим счетчик состояния */
	if (++anima.count < lpStartRoundAnima[anima.state].count)
		/* передаем управление на копирование блоков */
		return 1;

	/* сбросим счетчик */
	anima.count = 0;

	/* переключаем состояние анимации */
	state = lpStartRoundAnima[++anima.state].number;
	if (state == 1) {
		/* отобразим сцену */
		scene.on = 1;
		/* отобразим круги откланения */
		SetSwerve();
		/* отобразим сообщение */
		scene.message.show = 1;
		scene.message.flags = MESSAGE_ROUND;
		scene.message.player = 1;
		scene.message.round = nLevel + 1;
	} else if (state == 2) {
		UCHAR (*b)[15];
		int (*e)[15];
		int i,j;
		
		/*
		 * Запускаем анимацию блоков (подмигивание блоков
		 * которые не разбиваются или разбиваются со второго раза),
		 * установим событие блока.
		 */
		b = &block[3];
		e = &EventBlock[3];
		for (i = 0; i < 12; i++) {
			for (j = 0; j < 15; j++) {
				if ((*b)[j] & (0x80 | 0x40)) {
					(*e)[j] = 1;
					/* запустим звук не разбиваемого блока */
					play.stone = 1;
				}
			}
			b++; e++;
		}
	} else {
		/* анимация завершена */
		anima.func = NULL;
		/* убераем сообщение */
		scene.message.show = 0;
		/* разрешаем паузу */
		pause.deny = 0;
		/* размещаем ракетку */
		SetRacket();
		/* размещаем мячики */
		SetBall();
		/* вначале раунда мячик примагничен к ракетке */
		bonus.flags = F_START_RAUND | F_HAND;
		/* время окончания прилипания к ракетке */
		ball[0].hand.count = TIME_ENDING_HAND;
		ball[0].hand.offset = 12;
	}

	/* передаем управление на копирование блоков */
	return 1;
}

/* запускает анимацию запуска раунда */
static void SetStartRoundAnima(void)
{
	/* подготовим структуру анимации */
	anima.func = StartRoundAnima;
	anima.count = 0;
	anima.state = 0;
	/* запретим отображение сцены */
	scene.on = 0;
	/* запретим паузу */
	pause.deny = 1;
	/* удалим все объекты */
	CleanObject();
	/* сброс всех событий блоков */
	memset(EventBlock,0,sizeof(EventBlock));
	/* сбросим все призы */
	bonus.flags = 0;
}

/* выполняем анимацию разлета осколков */
static int MoveSliver(void)
{
	int i,offset;

	/* проверим завершение анимации */
	if (++anima.count >= lpFlySliverAnima[anima.state].count) {
		int state;

		/* сбросим счетчик анимации */
		anima.count = 0;
		/* переключим анимацию на следующий кадр */
		state = lpFlySliverAnima[++anima.state].number;
		/* проверим завершение анимации */
		if (state == 5) {
			/* больше не отоброжаем осколки */
			scene.pSlivers.show = 0;
			for (i = 0; i < SLIVERS_COUNT; i++) {
				LPFULL_PICTURE lpPicture = &scene.pSlivers.picture[i];
				lpPicture->picture.show = 0;
				lpPicture->shadow.show = 0;
			}
			/*
			 * Здесь завершение раунда не происходит т.к. на пограничном
			 * элементе производиться вычитание жизни и запуск анимации
			 * окончания раунда.
			 */
		} else if (state < 0) {
			/* удалим свою функцию анимации */
			anima.func = NULL;
			/*
			 * Если жизней не осталось то возвратим, что игрок
			 * проиграл иначе запустим анимацию начала раунда.
			 */
			if (--lifes < 0)
				/* заставим возвратим двойку  (игрок проиграл) */
				return 4;
			else
				SetStartRoundAnima();
			/* заставим возвратить нуль (идет игра) */
			return 2;
		} else {
			/* загрузим следующий тип осколка */
			for (i = 0; i < SLIVERS_COUNT; i++) {
				LPFULL_PICTURE lpPicture = &scene.pSlivers.picture[i];
				lpPicture->picture.type = state;
				lpPicture->shadow.type = state;
			}
		}
	}

	/* осколки больше не отображаются */
	if (lpFlySliverAnima[anima.state].number == 5)
		/* передаем управление на копирование блоков */
		return 1;

	/* расчитаем смещение тени осколка */
	switch (lpFlySliverAnima[anima.state].number) {
		case 0:
			offset = 6;
			break;
		case 1:
		case 2:
			offset = 5;
			break;
		case 3:
			offset = 4;
			break;
		default:
			offset = 3;
	}

	/* переместим осколки */
	for (i = 0; i < SLIVERS_COUNT; i++) {
		LPFULL_PICTURE lpPicture = &scene.pSlivers.picture[i];
		LPSLIVER lpSliver = &sliver[i];
		int x,y;
		/* расчитаем новую позицию */
		lpSliver->x += lpSliver->vx;
		lpSliver->y += lpSliver->vy;
		/*
		 * Обработаем столкновение со стенками.
		 * Столкновение с верхней стенкой не проверяется, т.к.
		 * осколки до верхней стенки не долетают. Если будет увеличина
		 * скорость движения осколков, то надо будет добавить проверку
		 * столкновения с верхней стенкой (координата 'y').
		 */
		if (lpSliver->x < LOGIC_X) {
			lpSliver->vx = -lpSliver->vx;
			lpSliver->x = LOGIC_X;
		} else if (lpSliver->x > LOGIC_X + LOGIC_WIDTH - 7) {
			lpSliver->vx = -lpSliver->vx;
			lpSliver->x = LOGIC_X + LOGIC_WIDTH - 7;
		}
		/* обновим отображаемые координаты */
		x = ftol(lpSliver->x);
		y = ftol(lpSliver->y);
		lpPicture->picture.X = x;
		lpPicture->picture.Y = y;
		lpPicture->shadow.X = x + offset;
		lpPicture->shadow.Y = y + offset;
	}

	/* передаем управление на копирование блоков */
	return 1;
}

/* функция анимации разлета осколков ракетки */
static int FlySliverAnima(void)
{
	/* поверим, отображаются осколки или нет */
	if (!anima.state) {
		/* разместим осколки на поле */
		int i,type,racketWidth;
		double x,y,x0,y0,angle;

		/* увеличим счетчик состояния анимации */
		anima.state++;
		/* загрузим тип осколков */
		type = lpFlySliverAnima[anima.state].number;
		/*
		 * Осколки на поле размещаются полукругом,
		 * для этого используется формула элипсиса.
		 * Исползуем следующие параметры a = 15, b = -5.
		 */
		/* расчитаем начальные координаты x,y */
		racketWidth = GetWidthRacket();
		if (racketWidth < 36)
			x = racket.x - ((36 - racketWidth) / 2);
		else if (racketWidth > 36)
			x = racket.x + ((racketWidth - 36) / 2);
		else
			x = racket.x;
		y = RACKET_Y;
		/* расчитаем начальные координаты x0,y0 */
		x0 = x + 15.0;
		y0 = y + 5.0;
		/* начальный угол */
		angle = 180.0;
		/* расчитаем положение осколков */
		for (i = 0; i < SLIVERS_COUNT; i++) {
			LPFULL_PICTURE lpPicture;
			LPSLIVER lpSliver;
			double rad;
			int lx,ly;
			/* увеличиваем координату x */
			x += 3.0;
			/* находим координату 'y' по формуле элипсиса */
			y = -5.0 * sqrt(fabs(1.0 - pow((x - x0) / 15.0,2.0))) + y0;
			/* получаем целочисленные координаты */
			lx = ftol(x);
			/* скоректируем координату x */
			if (lx < LOGIC_X)
				lx = LOGIC_X;
			else if (lx > LOGIC_X + LOGIC_WIDTH - 7)
				lx = LOGIC_X + LOGIC_WIDTH - 7;
			ly = ftol(y);
			/* запомним координаты для логики */
			lpSliver = &sliver[i];
			lpSliver->x = lx;
			lpSliver->y = ly;
			/* расчитываем скорости (vx,vy) для логики */
			rad = angle * PI / 180.0;
			lpSliver->vx = SPEED_SLIVER * cos(rad);
			lpSliver->vy = SPEED_SLIVER * sin(rad);
			/* увеличим угол */
			angle += 22.5;
			/* запомним координаты для отображения осколков */
			lpPicture = &scene.pSlivers.picture[i];
			lpPicture->picture.show = 1;
			lpPicture->picture.type = type;
			lpPicture->picture.X = lx;
			lpPicture->picture.Y = ly;
			/* запомним координаты для отображения тени от осколков */
			lpPicture->shadow.show = 1;
			lpPicture->shadow.type = type;
			lpPicture->shadow.X = lx + 6;
			lpPicture->shadow.Y = ly + 6;
		}
		/* осколки отображаются */
		scene.pSlivers.show = 1;
	} else {
		/* перемещаем осколки */
		return MoveSliver();
	}

	/* передаем управление на копирование блоков */
	return 1;
}

/* запустим анимацию разлета осколков ракетки */
static void SetFlySliverAnima(void)
{
	/* подготовим анимацию разлета осколков ракетки */
	anima.func = FlySliverAnima;
	anima.count = 0;
	anima.state = 0;
	/* запретим паузу */
	pause.deny = 1;
	/* удалим все объекты */
	CleanObject();
	/* озвучим событие */
	play.destroy = 1;
}

/* анимация окончания паузы */
static int PauseEndAnima(void)
{
	int state;

	/* увеличим счетчик состояния */
	if (++anima.count < lpPauseEndAnima[anima.state].count)
		/* заставим возвратить нуль (идет игра) */
		return 2;

	/* сбросим счетчик */
	anima.count = 0;

	/* переключаем состояние анимации */
	state = lpPauseEndAnima[++anima.state].number;
	/* уменьшим цифру завершения паузы */
	if (state == 1) {
		scene.message.pause = 2;
	} else if (state == 2) {
		scene.message.pause = 1;
	} else {
		/* анимация завершилась */
		anima.func = NULL;
		/* удалим сообщение */
		scene.message.show = 0;
		/* разрешаем паузу */
		pause.deny = 0;
	}

	/* заставим возвратить нуль (идет игра) */
	return 2;
}

/* запускает анимацию окончания паузы */
static void SetPauseEndAnima(void)
{
	/* подготовим структуру анимации */
	anima.func = PauseEndAnima;
	anima.count = 0;
	anima.state = 0;
	/* запретим паузу */
	pause.deny = 1;
	/* отобразим сообщение паузы */
	scene.message.show = 1;
	scene.message.flags = MESSAGE_PAUSE;
	scene.message.pause = 3;
}

/* функция анимиции полета на ракете в следующую стадию */
static int FlyRocketAnima(void)
{
	static int count = 0;
	UCHAR *point;
	int state;
	div_t n;

	/* определим состояние анимации */
	state = lpFlyRocketAnima[anima.state].number;

	/*
	 * Определим что делать, перемещать ракету
	 * или подсчитывать бунусные очки.
	 */
	if (!state) {
		/* переместим ракету */
		rocket.y += rocket.vy;
		rocket.iy = ftol(rocket.y);
		/* добавим ускарение */
		rocket.vy -= ACCELERATION_Y;

		/* обновим данные для отображения */
		scene.pRocket.Y = rocket.iy;
		if (scene.pRocket.type == TYPE_ROCKET_0)
			scene.pRocket.type = TYPE_ROCKET_1;
		else
			scene.pRocket.type = TYPE_ROCKET_0;

		/*
		 * Проверим завершение полета ракеты.
		 * 120 - это то количество кадров, за которое ракета
		 * уйдет за пределы игрового поля.
		 */
		if (++count >= 120) {
			anima.state++;
			count = 0;
		}
	} else if (state == 1) {
		/* расчитаем проверяемый кирпич в массиве кирпичей */
		n = div(count,15);
		n.quot += 3;
		point = &block[n.quot][n.rem];
		/* если блок есть и он разбивается, то добавим призовые очки */
		if (*point && !(*point & 0x80))
			BlockScore(n.rem,n.quot);

		/* проверим следующий блок */
		count++;
		n = div(count,15);
		n.quot += 3;
		point = &block[n.quot][n.rem];
		if (*point && !(*point & 0x80))
			BlockScore(n.rem,n.quot);

		/* размер массива с блоками имеет размер (15,12) */
		if (++count >= 15 * 12) {
			anima.state++;
			count = 0;
		}
	} else {
		/* удалим функцию анимации */
		anima.func = NULL;
		/* запустим анимацию окончания раунда */
		SetEndRoundAnima();
	}

	/* передаем управление на копирование блоков */
	return 1;
}

/* запуск анимации полета ракеты в следующую стадию */
static void SetFlyRocketAnima(void)
{
	/* подготавливаем анимацию */
	anima.func = FlyRocketAnima;
	anima.count = 0;
	anima.state = 0;
	/* запретим паузу */
	pause.deny = 1;
	/* подготовим структуры ракеты */
	rocket.ix = scene.pRacket.X;
	rocket.iy = scene.pRacket.Y;
	rocket.y = rocket.iy;
	rocket.vy = 0.0;
	/* отобразим ракету в следующий раунд */
	scene.pRocket.show = 1;
	scene.pRocket.type = TYPE_ROCKET_0;
	scene.pRocket.X = scene.pRacket.X;
	scene.pRocket.Y = scene.pRacket.Y;
	/* удалим все объекты */
	CleanObject();
}

/* проверить управляющие клавишы */
static void CheckControlKey(void)
{
	/* управление паузой */
	if (input.P) {
		if (!pause.deny) {
			if (!pause.pause) {
				scene.message.show = 1;
				scene.message.flags = MESSAGE_PAUSE;
				scene.message.pause = 0;
				pause.pause = 1;
			} else {
				/* сбросим паузу */
				pause.pause = 0;
				/* запустим анимацию окончания паузы */
				SetPauseEndAnima();
			}
		}
	}
}

/* загрузить следующий кадр анимации ракетки */
static void NextFrameRacket(void)
{
	int point,curentWidth,oldWidth;

	/* увеличим счетчик показа кадров */
	point = racket.animation.point;
	if (++racket.animation.count < racket.animation.frames[point].count)
		return;

	/* перегружаем кадр */
	point++;
	/* проверим завершение анимации */
	if (racket.animation.frames[point].number < 0) {
		/*
		 * Если установлен дополнительный буфер для отображения,
		 * то загрузим его. Это нужно когда например ракетка большая и
		 * пойман приз "стреляющая ракетка" то в анимацию загружается
		 * кадры "большая ракетка" -> "обычная ракетка" а в резервный
		 * буфер загружаются кадры "обычная ракетка" -> "стреляющая ракетка".
		 */
		if (racket.frames) {
			racket.animation.frames = racket.frames;
			racket.animation.point = 0;
			racket.animation.count = 0;
			/* сбросим резервный буфер кадров */
			racket.frames = NULL;
		} else
			racket.animation.frames = NULL;

		return;
	}

	/* получим текущую ширину ракетки */
	oldWidth = GetWidthRacket();
	/* загрузим новый тип ракетки */
	scene.pRacket.type = racket.animation.frames[point].number;
	/* получим новую ширину ракетки */
	curentWidth = GetWidthRacket();
	/* скоректируем положение ракетки */
	racket.x = racket.x + oldWidth / 2 - curentWidth / 2;
	if (racket.x < LOGIC_X)
		racket.x = LOGIC_X;
	else if (racket.x > (LOGIC_X + LOGIC_WIDTH - curentWidth))
		racket.x = LOGIC_X + LOGIC_WIDTH - curentWidth;

	/* перегрузим управляющие переменные */
	racket.animation.point = point;
	racket.animation.count = 0;
}

/* создать пульку */
static int CreateBullet(LPPICTURE lpBullet)
{
	int i,flags;

	/* если эта пулька уже отображается то уходим */
	if (lpBullet->show)
		return 0;

	/* определим близкую пульку */
	flags = 0;
	for (i = 0; i < BULLETS_COUNT; i++) {
		LPPICTURE lpTemp = &scene.pBullet[i];
		if (lpTemp != lpBullet) {
			if (lpTemp->show) {
				if ((RACKET_Y - lpTemp->Y) <= MIN_SPACE_BULLET) {
					flags = 1;
					break;
				}
			}
		}
	}

	/* нельзя создавать пульку */
	if (flags)
		return 0;

	/* создаем пульку */
	lpBullet->show = 1;
	lpBullet->type = TYPE_BULLET_0;
	lpBullet->X = racket.x + GetWidthRacket() / 2 - 2;
	lpBullet->Y = RACKET_Y - 8;

	return 1;
}

/* переместим ракетку */
static void MoveRacket(void)
{
	int width;

	/* проверим, отображается ракетка или нет */
	if (!racket.on)
		return;

	/* если есть анимация то выполняем ее */
	if (racket.animation.frames != NULL)
		NextFrameRacket();

	/* получим ширину ракетки */
	width = GetWidthRacket();

	/* проверим управление с клавиатуры */
	if (input.K) {
		/* перемещаем ракетку в левую сторону */
		racket.x -= 4;
		if (racket.x < LOGIC_X)
			racket.x = LOGIC_X;
	}
	if (input.L) {
		/* перемещаем ракетку в правую сторону */
		racket.x += 4;
		if (racket.x > (LOGIC_X + LOGIC_WIDTH - width))
			racket.x = LOGIC_X + LOGIC_WIDTH - width;
	}

	/* проверим управление с мыши */
	if (input.x) {
		/* переместим ракетку */
		racket.x += ftol(input.x * RACKET_FACTOR);
		if (racket.x < LOGIC_X)
			racket.x = LOGIC_X;
		else if (racket.x > (LOGIC_X + LOGIC_WIDTH - width))
			racket.x = LOGIC_X + LOGIC_WIDTH - width;
	}

	/* действует приз 'BONUS_GUN' */
	if (CHECK_FLAG(bonus.flags,F_GUN)) {
		/* проверим что ракетка не изменяет свой вид */
		if (racket.animation.frames == NULL) {
			/* проверим управляющие клавиши */
			if (input.lButton || input.SPACE) {
				int i;
				/* создадим пульку */
				for (i = 0; i < BULLETS_COUNT; i++) {
					if (CreateBullet(&scene.pBullet[i]))
						break;
				}
			}
		}
	}

	/* сформируем изображение */
	scene.pRacket.X = racket.x;
}

/* захват мячика кругом отклонения */
static int LockSwerve(LPSWERVE lpSwerve, LPBALL lpBall)
{
	int rBall,xBall,yBall;
	int flags = 0;

	/* расчитаем контрольные точки мячика */
	rBall = GetWidthBall() / 2;
	xBall = lpBall->ix + rBall;
	yBall = lpBall->iy + rBall;
	/* в зависимости от угла движения примим решение */
	switch (lpBall->angle) {
		/* для вертикальных углов движения */
		case 293:
			/* проверим, что мячик пересек контрольную линию */
			if (yBall <= lpSwerve->y + 1) {
				/* установим флаг обработки */
				flags = 1;
				/*
				 * В зависимости от места пересечения
				 * установим результирующий угол.
				 */
				if (xBall <= lpSwerve->x - 9) {
					lpBall->swerve.angle = 315;
					lpBall->angle = 304;
				} else {
					lpBall->swerve.angle = 247;
					lpBall->angle = 270;
				}
			}
			break;

		case 247:
			if (yBall <= lpSwerve->y + 1) {
				flags = 1;
				if (xBall >= lpSwerve->x + 9) {
					lpBall->swerve.angle = 225;
					lpBall->angle = 236;
				} else {
					lpBall->swerve.angle = 293;
					lpBall->angle = 270;
				}
			}
			break;

		case 113:
			if (yBall >= lpSwerve->y - 1) {
				flags = 1;
				if (xBall >= lpSwerve->x + 9) {
					lpBall->swerve.angle = 135;
					lpBall->angle = 124;
				} else {
					lpBall->swerve.angle = 67;
					lpBall->angle = 90;
				}
			}
			break;

		case 67:
			if (yBall >= lpSwerve->y - 1) {
				flags = 1;
				if (xBall <= lpSwerve->x - 9) {
					lpBall->swerve.angle = 45;
					lpBall->angle = 56;
				} else {
					lpBall->swerve.angle = 113;
					lpBall->angle = 90;
				}
			}
			break;

		/* для вертикальных углов движения */
		case 203:
			/* проверим, что мячик пересек контрольную линию */
			if (xBall <= lpSwerve->x + 1) {
				/* установим флаг обработки */
				flags = 1;
				/*
				 * В зависимости от места пересечения
				 * установим результирующий угол.
				 */
				if (yBall >= lpSwerve->y + 9) {
					lpBall->swerve.angle = 225;
					lpBall->angle = 214;
				} else if (yBall >= lpSwerve->y - 1) {
					lpBall->swerve.angle = 247;
					lpBall->angle = 225;
				} else {
					lpBall->swerve.angle = 157;
					lpBall->angle = 180;
				}
			}
			break;

		case 157:
			if (xBall <= lpSwerve->x + 1) {
				flags = 1;
				if (yBall <= lpSwerve->y - 9) {
					lpBall->swerve.angle = 135;
					lpBall->angle = 146;
				} else if (yBall <= lpSwerve->y + 1) {
					lpBall->swerve.angle = 113;
					lpBall->angle = 135;
				} else {
					lpBall->swerve.angle = 203;
					lpBall->angle = 180;
				}
			}
			break;

		case 337:
			if (xBall >= lpSwerve->x - 1) {
				flags = 1;
				if (yBall >= lpSwerve->y + 9) {
					lpBall->swerve.angle = 315;
					lpBall->angle = 326;
				} else if (yBall >= lpSwerve->y - 1) {
					lpBall->swerve.angle = 293;
					lpBall->angle = 315;
				} else {
					lpBall->swerve.angle = 23;
					lpBall->angle = 0;
				}
			}
			break;

		case 23:
			if (xBall >= lpSwerve->x - 1) {
				flags = 1;
				if (yBall <= lpSwerve->y - 9) {
					lpBall->swerve.angle = 45;
					lpBall->angle = 34;
				} else if (yBall <= lpSwerve->y + 1) {
					lpBall->swerve.angle = 67;
					lpBall->angle = 45;
				} else {
					lpBall->swerve.angle = 337;
					lpBall->angle = 0;
				}
			}
			break;

		/* для острых углов движения */
		case 315:
			/* проверим, что мячик вощел в зону принятия решения */
			if ((xBall >= lpSwerve->x - 7) && (yBall <= lpSwerve->y + 7)) {
				/* установим флаг обработки */
				flags = 1;
				/*
				 * В зависимости от места пересечения
				 * установим результирующий угол.
				 */
				if (yBall <= lpSwerve->y - 1) {
					lpBall->swerve.angle = 337;
					lpBall->angle = 326;
				} else {
					lpBall->swerve.angle = 293;
					lpBall->angle = 304;
				}
			}
			break;

		case 225:
			if ((xBall <= lpSwerve->x + 7) && (yBall <= lpSwerve->y + 7)) {
				flags = 1;
				if (yBall <= lpSwerve->y - 1) {
					lpBall->swerve.angle = 203;
					lpBall->angle = 214;
				} else {
					lpBall->swerve.angle = 247;
					lpBall->angle = 236;
				}
			}
			break;

		case 45:
			if ((xBall >= lpSwerve->x - 7) && (yBall >= lpSwerve->y - 7)) {
				flags = 1;
				if (yBall >= lpSwerve->y + 1) {
					lpBall->swerve.angle = 23;
					lpBall->angle = 34;
				} else {
					lpBall->swerve.angle = 67;
					lpBall->angle = 56;
				}
			}
			break;

		case 135:
			if ((xBall <= lpSwerve->x + 7) && (yBall >= lpSwerve->y - 7)) {
				flags = 1;
				if (yBall >= lpSwerve->y + 1) {
					lpBall->swerve.angle = 157;
					lpBall->angle = 146;
				} else {
					lpBall->swerve.angle = 113;
					lpBall->angle = 124;
				}
			}
			break;
	}

	/* если мячик был обработан */
	if (flags) {
		/* установить флаг захвата и указатель круга */
		lpBall->swerve.lock = 1;
		/* обновить скорость */
		SetSpeedBall(lpBall);
	}

	return flags;
}

/* освобождение мячика кругом отклонения */
static void UnlockSwerve(LPBALL lpBall)
{
	/* освобождаем мячик */
	if (lpBall->swerve.lock) {
		lpBall->swerve.lock = 0;
		/* устанавливаем угол к которому стремился мячик */
		lpBall->angle = lpBall->swerve.angle;
		/* обновляем скорости */
		SetSpeedBall(lpBall);
	}
}

/* проверить пересечение с "кругом отклонения" */
static int CheckCollideSwerve(LPBALL lpBall)
{
	int i,rSum,rBall,flags = 0;

	/* расчитаем квадрат суммы радиусов "круга откланения" и мячика */
	rBall = GetWidthBall() / 2;
	rSum = SWERVE_RADIUS + rBall;
	rSum *= rSum;

	/* проверим пересечение с кругом отклонения */
	for (i = 0; i < SWERVES_COUNT; i++) {
		LPSWERVE lpSwerve = &swerve[i];
		/* смотрим если круг включен */
		if (lpSwerve->on) {
			int dx = (lpBall->ix + rBall) - lpSwerve->x;
			int dy = (lpBall->iy + rBall) - lpSwerve->y;
			/* проверим, что мячик находится в зоне круга */
			if ((dx * dx + dy * dy) <= rSum) {
				/* если мячик небыл захвачен, то захватываем его */
				if (!lpBall->swerve.lock) {
					/* если мячик был обработан то запоминаем это */
					if (LockSwerve(lpSwerve,lpBall)) {
						lpBall->swerve.num = i;
						flags = 1;
					}
				}
			} else {
				/* если мячик был захвачен этим кругом, то освободим его */
				if (lpBall->swerve.lock && (lpBall->swerve.num == i)) {
					UnlockSwerve(lpBall);
					flags = 1;
				}
			}
		} else {
			/* если мячик был захвачен, освободим его */
			if (lpBall->swerve.lock && (lpBall->swerve.num == i)) {
				UnlockSwerve(lpBall);
				/* мячик был обработан */
				flags = 1;
			}
		}
	}

	return flags;
}

/* корректировка координаты 'X' мячика */
static void UpdateBallX(LPBALL lpBall, int displace)
{
	lpBall->x -= (lpBall->x - displace) * 2;
	lpBall->ix = ftol(lpBall->x);
}

/* корректировка координаты 'Y' мячика */
static void UpdateBallY(LPBALL lpBall, int displace)
{
	lpBall->y -= (lpBall->y - displace) * 2;
	lpBall->iy = ftol(lpBall->y);
}

/* проверяет столкновение со стенкой */
static int CheckCollideBorder(LPBALL lpBall)
{
	int wBall,flags = 0;

	/* получим ширину мячика */
	wBall = GetWidthBall();
	/* проверим столкновение с боковыми стенками, плоскость 'y' */
	if ((lpBall->ix <= LOGIC_X) &&
		(lpBall->angle > 90) && (lpBall->angle < 270)) {
		flags = 1;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* расчитаем изменение угла в плоскости 'y' */
		lpBall->angle = 180 - lpBall->angle;
		if (lpBall->angle < 0) lpBall->angle += 360;
		/* расчитаем новые скорости */
		SetSpeedBall(lpBall);
		/* скоректируем координату 'x' */
		UpdateBallX(lpBall,LOGIC_X);
		/* озвучим событие */
		play.wall = 1;
	} else if ((lpBall->ix >= (LOGIC_X + LOGIC_WIDTH - wBall)) &&
		((lpBall->angle > 270) || (lpBall->angle < 90))) {
		flags = 1;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* расчитаем изменение угла в плоскости 'y' */
		lpBall->angle = 180 - lpBall->angle;
		if (lpBall->angle < 0) lpBall->angle += 360;
		/* расчитаем новые скорости */
		SetSpeedBall(lpBall);
		/* скоректируем координату 'x' */
		UpdateBallX(lpBall,LOGIC_X+LOGIC_WIDTH-wBall);
		/* озвучим событие */
		play.wall = 1;
	}

	/* проверим столкновение с верхней стенкой, плоскость 'x' */
	if ((lpBall->iy <= LOGIC_Y) && (lpBall->angle > 180)) {
		flags = 1;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* расчитаем изменение угла в плоскости 'x' */
		lpBall->angle = 360 - lpBall->angle;
		/* расчитаем новые скорости */
		SetSpeedBall(lpBall);
		/* скоректируем координату 'y' */
		UpdateBallY(lpBall,LOGIC_Y);
		/* озвучим событие */
		play.wall = 1;
	} else if (lpBall->iy > (LOGIC_Y + LOGIC_HEIGHT)) {
		/* шарик улетел за нижнюю линию */
		int i, count;
		/* сбросим данный шарик */
		lpBall->on = 0;
		/* подсчитаем количество оставшихся шариков */
		for (i = count = 0; i < BALLS_COUNT; i++) {
			if (ball[i].on)
				count++;
		}
		/* проверим количество оставшихся мячиков */
		if (count == 0)
			/* нет шариков, разрушаем ракетку */
			SetFlySliverAnima();
		else if (count == 1)
			/* один шарик, сбросим приз "три шарика" */
			bonus.flags &= ~F_TRIPLE_BALL;
	}

	return flags;
}

/* проверяет столкновение с ракеткой */
static int CheckCollideRacket(LPBALL lpBall)
{
	int b1,b2,r1,r2,r3,r4,r5,r6;
	int wRacket,wBall,temp;

	/* проверим, что проекция скорости по 'y' положительна */
	if (lpBall->angle > 180)
		return 0;
	/* получим ширину мячика */
	wBall = GetWidthBall();
	/* проверим, что мячик находится на уровне ракетки */
	if ((lpBall->iy < (RACKET_Y - wBall)) || (lpBall->iy > RACKET_Y))
		return 0;
	/* получим ширину ракетки */
	wRacket = GetWidthRacket();
	/* проверим, что мячик пересекается с ракеткой */
	if (((lpBall->ix + wBall) <= racket.x) ||
		(lpBall->ix >= (racket.x + wRacket)))
		return 0;

	/* расчитаем координаты контрольных точек ракетки */
	temp = wRacket / 2;
	r3 = temp - 1;
	r4 = wRacket - (r3 + 1);
	r2 = temp / 2;
	r5 = wRacket - (r2 + 1);
	r1 = r2 / 2 + 1;
	r6 = wRacket - (r1 + 1);
	/* добавим смещение ракетки */
	r1 += racket.x;
	r2 += racket.x;
	r3 += racket.x;
	r4 += racket.x;
	r5 += racket.x;
	r6 += racket.x;

	/* установим координаты точек шарика */
	b1 = lpBall->ix + 2;
	b2 = lpBall->ix + 4;

	/*
	 * Проверим что не действует приз 'BONUS_HAND' или
	 * ракетка изменяет свой вид.
	 */
	if (!CHECK_FLAG(bonus.flags,F_HAND) ||
		(racket.animation.frames != NULL)) {
		/* Выясним направление скорости на плоскости 'x'.*/
		if (lpBall->angle < 90) {
			/* Проекция скорости по 'x' положительная. */
			/*
			 * Расчитаем угол отражения взависимости от 
			 * места соприкосновения с ракеткой.
			 */
			if (b2 <= r1) {
				if (lpBall->angle == 67) lpBall->angle = 225;
				else lpBall->angle = 203;
			} else if ((b2 <= r2) && (lpBall->angle != 67)) {
				lpBall->angle = 225;
			} else if (b2 <= r3) {
				lpBall->angle = 247;
			} else if (b1 < r6) {
				lpBall->angle = 360 - lpBall->angle;
			} else {
				if (lpBall->angle == 67) lpBall->angle = 315;
				else lpBall->angle = 337;
			}
		} else {
			/* Проекция скорости по 'x' отрицательная. */
			/*
			 * Расчитаем угол отражения взависимости от 
			 * места соприкосновения с ракеткой.
			 */
			if (b1 >= r6) {
				if (lpBall->angle == 113) lpBall->angle = 315;
				else lpBall->angle = 337;
			} else if ((b1 >= r5) && (lpBall->angle != 113)) {
				lpBall->angle = 315;
			} else if (b1 >= r4) {
				lpBall->angle = 293;
			} else if (b2 > r1) {
				lpBall->angle = 360 - lpBall->angle;
			} else {
				if (lpBall->angle == 113) lpBall->angle = 225;
				else lpBall->angle = 203;
			}
		}
	} else {
		/* действует приз 'BOHUS_HAND' */
		if (b2 <= r1) {
			lpBall->angle = 203;
			lpBall->hand.offset = -1;
		} else if (b2 <= (r2 + 1)) {
			lpBall->angle = 225;
			lpBall->hand.offset = 4;
		} else if (b2 <= (r3 + 1)) {
			lpBall->angle = 247;
			lpBall->hand.offset = 8;
		} else if (b1 >= r6) {
			lpBall->angle = 337;
			lpBall->hand.offset = 22;
		} else if (b1 >= (r5 - 1)) {
			lpBall->angle = 315;
			lpBall->hand.offset = 17;
		} else {
			lpBall->angle = 293;
			lpBall->hand.offset = 13;
		}
		/* обновим счетчик */
		lpBall->hand.count = TIME_ENDING_HAND;
	}

	/* расчитаем новую скорость */
	SetSpeedBall(lpBall);

	/* озвучим событие */
	play.wall = 1;

	return 1;
}

/* проверка столкновения с блоками при углах менее 90 градусов */
static int CheckBlock90(LPBALL lpBall)
{
	BOUND bound1,bound2,bound3,bound4,bound5;
	int b1,b2,b3,b4,b5,wBall;

	/* получим ширину мячика */
	wBall = GetWidthBall();
	/* получим блоки содержащии проверяемые точки */
	b1 = GetBlock(&bound1,lpBall->ix,lpBall->iy+wBall);          /* нижняя-левая */
	b2 = GetBlock(&bound2,lpBall->ix+wBall-1,lpBall->iy+wBall);  /* нижняя-правая */
	b3 = GetBlock(&bound3,lpBall->ix+wBall,lpBall->iy);          /* правая-верхняя */
	b4 = GetBlock(&bound4,lpBall->ix+wBall,lpBall->iy+wBall-1);  /* правая-нижняя */
	b5 = GetBlock(&bound5,lpBall->ix+wBall,lpBall->iy+wBall);    /* крайняя правая-нижняя */

	/* проверим обе стороны в разных плоскостях */
	if ((b1 && b3) && (bound1.x != bound3.x) &&
		(bound1.y != bound3.y)) {
		int flags = 0;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим данные блоки */
		if (MarkBlock(bound1.x,bound1.y))
			flags = 1;
		if (MarkBlock(bound3.x,bound3.y))
			flags = 1;
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = lpBall->angle + 180;
		SetSpeedBall(lpBall);
		/* скоректируем координаты */
		UpdateBallX(lpBall,bound3.x * 16 + LOGIC_X - wBall);
		UpdateBallY(lpBall,bound1.y * 8 + LOGIC_Y - wBall);

		return flags;
	}

	/* проверим нижнию плоскость */
	if ((b1 && b2) && (bound1.x != bound2.x)) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* определим, на какой блок шарик накладывается больше */
		if ((16 - bound1.offset_x) > (bound2.offset_x + 1)) {
			flags = MarkBlock(bound1.x,bound1.y);
		} else {
			flags = MarkBlock(bound2.x,bound2.y);
		}
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скорректируем координату */
		UpdateBallY(lpBall,bound1.y * 8 + LOGIC_Y - wBall);

		return flags;
	}

	/* проверим правую плоскость */
	if ((b3 && b4) && (bound3.y != bound4.y)) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* определим, на какой блок шарик накладывается больше */
		if ((8 - bound3.offset_y) > (bound4.offset_y + 1)) {
			flags = MarkBlock(bound3.x,bound3.y);
		} else {
			flags = MarkBlock(bound4.x,bound4.y);
		}
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallX(lpBall,bound3.x * 16 + LOGIC_X - wBall);

		return flags;
	}

	/* проверим нижний левый угол */
	if (b1) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блок */
		flags = MarkBlock(bound1.x,bound1.y);
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallY(lpBall,bound1.y * 8 + LOGIC_Y - wBall);

		return flags;
	}

	/* проверим правый верхний угол */
	if (b3) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блок */
		flags = MarkBlock(bound3.x,bound3.y);
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallX(lpBall,bound3.x * 16 + LOGIC_X - wBall);

		return flags;
	}

	/* проверим крайний угол */
	if (b5) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блок */
		flags = MarkBlock(bound5.x,bound5.y);
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* скоректируем координату и угол */
		if ((bound5.offset_x + 1) > (bound5.offset_y + 1)) {
			lpBall->angle = 360 - lpBall->angle;
			UpdateBallY(lpBall,bound5.y * 8 + LOGIC_Y - wBall);
		} else if ((bound5.offset_x + 1) < (bound5.offset_y + 1)) {
			lpBall->angle = 180 - lpBall->angle;
			UpdateBallX(lpBall,bound5.x * 16 + LOGIC_X - wBall);
		} else {
			/* это нужно для предотвращения отскакивания от угла */
			if (lpBall->angle < 45) {
				lpBall->angle = 180 - lpBall->angle;
				UpdateBallX(lpBall,bound5.x * 16 + LOGIC_X - wBall);
			} else {
				lpBall->angle = 360 - lpBall->angle;
				UpdateBallY(lpBall,bound5.y * 8 + LOGIC_Y - wBall);
			}
		}
		/* расчитаем скорость */
		SetSpeedBall(lpBall);

		return flags;
	}

	return 0;
}

static int CheckBlock180(LPBALL lpBall)
{
	BOUND bound1,bound2,bound3,bound4,bound5;
	int b1,b2,b3,b4,b5,wBall;

	/* получим ширину мячика */
	wBall = GetWidthBall();
	/* получим блоки содержащии проверяемые точки */
	b1 = GetBlock(&bound1,lpBall->ix+wBall-1,lpBall->iy+wBall);  /* нижняя-правая */
	b2 = GetBlock(&bound2,lpBall->ix,lpBall->iy+wBall);          /* нижняя-левая */
	b3 = GetBlock(&bound3,lpBall->ix-1,lpBall->iy);              /* левая-верхняя */
	b4 = GetBlock(&bound4,lpBall->ix-1,lpBall->iy+wBall-1);      /* левая-нижняя */
	b5 = GetBlock(&bound5,lpBall->ix-1,lpBall->iy+wBall);        /* крайняя левая-нижняя */

	/* проверим обе стороны в разных плоскостях */
	if ((b1 && b3) && (bound1.x != bound3.x) &&
		(bound1.y != bound3.y)) {
		int flags = 0;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим данные блоки */
		if (MarkBlock(bound1.x,bound1.y))
			flags = 1;
		if (MarkBlock(bound3.x,bound3.y))
			flags = 1;
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = lpBall->angle + 180;
		SetSpeedBall(lpBall);
		/* скоректируем координаты */
		UpdateBallX(lpBall,(bound3.x + 1) * 16 + LOGIC_X);
		UpdateBallY(lpBall,bound1.y * 8 + LOGIC_Y - wBall);

		return flags;
	}

	/* проверим нижнюю плоскость */
	if ((b1 && b2) && (bound1.x != bound2.x)) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* определим на какой блок шарик накладывается больше */
		if ((bound1.offset_x + 1) > (16 - bound2.offset_x)) {
			flags = MarkBlock(bound1.x,bound1.y);
		} else {
			flags = MarkBlock(bound2.x,bound2.y);
		}
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallY(lpBall,bound1.y * 8 + LOGIC_Y - wBall);

		return flags;
	}

	/* проверим левую плоскость */
	if ((b3 && b4) && (bound3.y != bound4.y)) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* определим на какой блок шарик накладывается больше */
		if ((8 - bound3.offset_y) > (bound4.offset_y + 1)) {
			flags = MarkBlock(bound3.x,bound3.y);
		} else {
			flags = MarkBlock(bound4.x,bound4.y);
		}
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallX(lpBall,(bound3.x + 1) * 16 + LOGIC_X);

		return flags;
	}

	/* проверим нижний правый угол */
	if (b1) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блок */
		flags = MarkBlock(bound1.x,bound1.y);
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallY(lpBall,bound1.y * 8 + LOGIC_Y - wBall);

		return flags;
	}

	/* проверим левый верхний угол */
	if (b3) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блок */
		flags = MarkBlock(bound3.x,bound3.y);
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallX(lpBall,(bound3.x + 1) * 16 + LOGIC_X);

		return flags;
	}

	/* проверим крайний угол */
	if (b5) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блок */
		flags = MarkBlock(bound5.x,bound5.y);
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* скоректируем координату и угол */
		if ((16 - bound5.offset_x) > (bound5.offset_y + 1)) {
			lpBall->angle = 360 - lpBall->angle;
			UpdateBallY(lpBall,bound5.y * 8 + LOGIC_Y - wBall);
		} else if ((16 - bound5.offset_x) < (bound5.offset_y + 1)) {
			lpBall->angle = 180 - lpBall->angle;
			UpdateBallX(lpBall,(bound5.x + 1) * 16 + LOGIC_X);
		} else {
			/* это нужно для предотвращения отскакивания от угла */
			if (lpBall->angle > 135) {
				lpBall->angle = 180 - lpBall->angle;
				UpdateBallX(lpBall,(bound5.x + 1) * 16 + LOGIC_X);
			} else {
				lpBall->angle = 360 - lpBall->angle;
				UpdateBallY(lpBall,bound5.y * 8 + LOGIC_Y - wBall);
			}
		}
		/* расчитаем скорость */
		SetSpeedBall(lpBall);

		return flags;
	}

	return 0;
}

static int CheckBlock270(LPBALL lpBall)
{
	BOUND bound1,bound2,bound3,bound4,bound5;
	int b1,b2,b3,b4,b5,wBall;

	/* получим ширину мячика */
	wBall = GetWidthBall();
	/* получим блоки содержащии проверяемые точки */
	b1 = GetBlock(&bound1,lpBall->ix+wBall-1,lpBall->iy-1); /* верхняя-правая */
	b2 = GetBlock(&bound2,lpBall->ix,lpBall->iy-1);         /* верхняя-левая */
	b3 = GetBlock(&bound3,lpBall->ix-1,lpBall->iy+wBall-1); /* левая-нижняя */
	b4 = GetBlock(&bound4,lpBall->ix-1,lpBall->iy);         /* левая-верхняя */
	b5 = GetBlock(&bound5,lpBall->ix-1,lpBall->iy-1);       /* крайняя верхняя-левая */

	/* проверим обе стороны в разных плоскостях */
	if ((b1 && b3) && (bound1.x != bound3.x) &&
		(bound1.y != bound3.y)) {
		int flags = 0;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блоки */
		if (MarkBlock(bound1.x,bound1.y))
			flags = 1;
		if (MarkBlock(bound3.x,bound3.y))
			flags = 1;
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = lpBall->angle - 180;
		SetSpeedBall(lpBall);
		/* скоректируем координаты */
		UpdateBallX(lpBall,(bound3.x + 1) * 16 + LOGIC_X);
		UpdateBallY(lpBall,(bound1.y + 1) * 8 + LOGIC_Y);

		return flags;
	}

	/* проверим верхнюю плоскость */
	if ((b1 && b2) && (bound1.x != bound2.x)) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* определим на какой блок шарик накладывается больше */
		if ((bound1.offset_x + 1) > (16 - bound2.offset_x)) {
			flags = MarkBlock(bound1.x,bound1.y);
		} else {
			flags = MarkBlock(bound2.x,bound2.y);
		}
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallY(lpBall,(bound1.y + 1) * 8 + LOGIC_Y);

		return flags;
	}

	/* проверим левую плоскость */
	if ((b3 && b4) && (bound3.y != bound4.y)) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* определим на какой блок шарик накладывается больше */
		if ((bound3.offset_y + 1) > (8 - bound4.offset_y)) {
			flags = MarkBlock(bound3.x,bound3.y);
		} else {
			flags = MarkBlock(bound4.x,bound4.y);
		}
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 360 + 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallX(lpBall,(bound3.x + 1) * 16 + LOGIC_X);

		return flags;
	}

	/* проверим верхний-правый угол */
	if (b1) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блок */
		flags = MarkBlock(bound1.x,bound1.y);
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallY(lpBall,(bound1.y + 1) * 8 + LOGIC_Y);

		return flags;
	}

	/* проверим левый-нижний угол */
	if (b3) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блок */
		flags = MarkBlock(bound3.x,bound3.y);
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 360 + 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallX(lpBall,(bound3.x + 1) * 16 + LOGIC_X);

		return flags;
	}

	/* проверим крайний угол */
	if (b5) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блок */
		flags = MarkBlock(bound5.x,bound5.y);
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* скоректируем координату и угол */
		if ((16 - bound5.offset_x) > (8 - bound5.offset_y)) {
			lpBall->angle = 360 - lpBall->angle;
			UpdateBallY(lpBall,(bound5.y + 1) * 8 + LOGIC_Y);
		} else if ((16 - bound5.offset_x) < (8 - bound5.offset_y)) {
			lpBall->angle = 360 + 180 - lpBall->angle;
			UpdateBallX(lpBall,(bound5.x + 1) * 16 + LOGIC_X);
		} else {
			/* это нужно для предотвращения отскакивания от угла */
			if (lpBall->angle < 225) {
				lpBall->angle = 360 + 180 - lpBall->angle;
				UpdateBallX(lpBall,(bound5.x + 1) * 16 + LOGIC_X);
			} else {
				lpBall->angle = 360 - lpBall->angle;
				UpdateBallY(lpBall,(bound5.y + 1) * 8 + LOGIC_Y);
			}
		}
		/* расчитаем скорость */
		SetSpeedBall(lpBall);

		return flags;
	}

	return 0;
}

static int CheckBlock360(LPBALL lpBall)
{
	BOUND bound1,bound2,bound3,bound4,bound5;
	int b1,b2,b3,b4,b5,wBall;

	/* получим ширину мячика */
	wBall = GetWidthBall();
	/* получим блоки содержащии проверяемые точки */
	b1 = GetBlock(&bound1,lpBall->ix,lpBall->iy-1);             /* верхняя-левая */
	b2 = GetBlock(&bound2,lpBall->ix+wBall-1,lpBall->iy-1);     /* верхняя-правая */
	b3 = GetBlock(&bound3,lpBall->ix+wBall,lpBall->iy+wBall-1); /* правая-нижняя */
	b4 = GetBlock(&bound4,lpBall->ix+wBall,lpBall->iy);         /* правая-верхняя */
	b5 = GetBlock(&bound5,lpBall->ix+wBall,lpBall->iy-1);       /* кайняя верхняя-правая */

	/* проверим обе стороны в разных плоскостях */
	if ((b1 && b3) && (bound1.x != bound3.x) &&
		(bound1.y != bound3.y)) {
		int flags = 0;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блоки */
		if (MarkBlock(bound1.x,bound1.y))
			flags = 1;
		if (MarkBlock(bound3.x,bound3.y))
			flags = 1;
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = lpBall->angle - 180;
		SetSpeedBall(lpBall);
		/* скоректируем координаты */
		UpdateBallX(lpBall,bound3.x * 16 + LOGIC_X - wBall);
		UpdateBallY(lpBall,(bound1.y + 1) * 8 + LOGIC_Y);

		return flags;
	}

	/* поверим верхнюю плоскость */
	if ((b1 && b2) && (bound1.x != bound2.x)) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* определим на какой блок шарик накладывается больше */
		if ((16 - bound1.offset_x) > (bound2.offset_x + 1)) {
			flags = MarkBlock(bound1.x,bound1.y);
		} else {
			flags = MarkBlock(bound2.x,bound2.y);
		}
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallY(lpBall,(bound1.y + 1) * 8 + LOGIC_Y);

		return flags;
	}

	/* проверим правую плоскость */
	if ((b3 && b4) && (bound3.y != bound4.y)) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* определим на какой блок шарик накладывается больше */
		if ((bound3.offset_y + 1) > (8 - bound4.offset_y)) {
			flags = MarkBlock(bound3.x,bound3.y);
		} else {
			flags = MarkBlock(bound4.x,bound4.y);
		}
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 360 + 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallX(lpBall,bound3.x * 16 + LOGIC_X - wBall);

		return flags;
	}

	/* проверим верхний-левый угол */
	if (b1) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блок */
		flags = MarkBlock(bound1.x,bound1.y);
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallY(lpBall,(bound1.y + 1) * 8 + LOGIC_Y);

		return flags;
	}

	/* проверим правый-нижний угол */
	if (b3) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блок */
		flags = MarkBlock(bound3.x,bound3.y);
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* отразим угол */
		lpBall->angle = 360 + 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* скоректируем координату */
		UpdateBallX(lpBall,bound3.x * 16 + LOGIC_X - wBall);

		return flags;
	}

	/* проверим крайний верхний-правый угол */
	if (b5) {
		int flags;
		/* если мячик был захвачен кругом, то освободим его */
		UnlockSwerve(lpBall);
		/* пометим блок */
		flags = MarkBlock(bound5.x,bound5.y);
		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* скоректируем координату и отразим угол */
		if ((bound5.offset_x + 1) > (8 - bound5.offset_y)) {
			lpBall->angle = 360 - lpBall->angle;
			UpdateBallY(lpBall,(bound5.y + 1) * 8 + LOGIC_Y);
		} else if ((bound5.offset_x + 1) < (8 - bound5.offset_y)) {
			lpBall->angle = 360 + 180 - lpBall->angle;
			UpdateBallX(lpBall,bound5.x * 16 + LOGIC_X - wBall);
		} else {
			/* это нужно для предотвращения отскакивания от угла */
			if (lpBall->angle > 315) {
				lpBall->angle = 360 + 180 - lpBall->angle;
				UpdateBallX(lpBall,bound5.x * 16 + LOGIC_X - wBall);
			} else {
				lpBall->angle = 360 - lpBall->angle;
				UpdateBallY(lpBall,(bound5.y + 1) * 8 + LOGIC_Y);
			}
		}
		/* расчитаем скорость */
		SetSpeedBall(lpBall);

		return flags;
	}

	return 0;
}

/* проверить столкновение с кирпичем */
static int CheckCollideBlock(LPBALL lpBall)
{
	int flags;

	/* взависимости от угла проверяем столкновения */
	if (lpBall->angle < 90)
		flags = CheckBlock90(lpBall);
	else if (lpBall->angle < 180)
		flags = CheckBlock180(lpBall);
	else if (lpBall->angle < 270)
		flags = CheckBlock270(lpBall);
	else
		flags = CheckBlock360(lpBall);

	return flags;
}

/* меняет случайным образом угол движения мячика */
static void CreateNewAngleBall(LPBALL lpBall)
{
	/* углы отражения мячика */
	static const int angle90[] = {113,135,157,293,315,337};
	static const int angle180[] = {203,225,247,23,45,67};
	static const int angle270[] = {293,315,337,113,135,157};
	static const int angle360[] = {23,45,67,203,225,247};
	const int *angle;

	/* сформируем новый угол движения мячика */
	if (lpBall->angle < 90)
		angle = angle90;
	else if (lpBall->angle < 180)
		angle = angle180;
	else if (lpBall->angle < 270)
		angle = angle270;
	else
		angle = angle360;

	/* устанавливаем случайный угол */
	lpBall->angle = angle[random(6)];
	SetSpeedBall(lpBall);
}

/* проверка столкновения с летающим существом */
static int CheckCollideAlien(LPBALL lpBall)
{
	int x,y,rBall;

	/* отображается летающее существо или нет */
	if (!alien.on)
		return 0;

	/* сформируем координаты мячика */
	rBall = GetWidthBall() / 2;
	x = lpBall->ix + rBall;
	y = lpBall->iy + rBall;

	/* мячик не попадает в летающее существо */
	if ((x < alien.ix + 1) || (x > alien.ix + 22))
		return 0;
	if ((y < alien.iy + 1) || (y > alien.iy + 12))
		return 0;

	/* мячик попал в летающее существо, сбросим круг захвата */
	UnlockSwerve(lpBall);
	/* сформируем новый угол движения мячика */
	CreateNewAngleBall(lpBall);

	/* сформируем анимацию взрыва */
	CreateBang();
	/* добавим бонусные очки за подбитое летающее существо */
	AddScore(350);

	/* озвучим событие */
	play.bang = 1;

	return 1;
}

/* переместим ширик с тенью */
static void MoveBall(void)
{
	int i;

	/* проверим окончание действия приза 'BONUS_SMASH_BALL' */
	if (CHECK_FLAG(bonus.flags,F_SMASH_BALL)) {
		/* уменьшим счетчик и если требуется сбросим приз */
		if (--timeEndingSmashBall <= 0) {
			for (i = 0; i < BALLS_COUNT; i++) {
				LPFULL_PICTURE lpPicture = &scene.pBall[i];
				lpPicture->picture.type = BALL_NORMAL;
				lpPicture->shadow.type = BALL_NORMAL;
			}
			bonus.flags &= ~F_SMASH_BALL;
		}
	}

	/* переберем все мячики */
	for (i = 0; i < BALLS_COUNT; i++) {
		LPBALL lpBall = &ball[i];
		LPFULL_PICTURE lpPicture = &scene.pBall[i];

		/* мячик отображается или нет */
		if (!lpBall->on)
			continue;

		/* проверка примагниченных мячиков к ракетке */
		if (lpBall->hand.count <= 0) {
			/* переместим мячик */
			lpBall->x += lpBall->vx;
			lpBall->y += lpBall->vy;

			/* получим целоцисленные значения координат */
			lpBall->ix = ftol(lpBall->x);
			lpBall->iy = ftol(lpBall->y);

			/* проверим столкновение со стенкой */
			CheckCollideBorder(lpBall);
			/* проверить пересечение с кругом откланения */
			CheckCollideSwerve(lpBall);
			/* проверим столкновение с ракеткой */
			if (CheckCollideRacket(lpBall))
				lpBall->count = TIME_NEW_COURSE_BALL;
			/* проверим столкновение с кирпичем */
			if (CheckCollideBlock(lpBall))
				lpBall->count = TIME_NEW_COURSE_BALL;
			/* проверка столкновения с летающим существом */
			if (CheckCollideAlien(lpBall))
				lpBall->count = TIME_NEW_COURSE_BALL;
			/* уменьшим счетчик откланения */
			if (--lpBall->count <= 0) {
				CreateNewAngleBall(lpBall);
				lpBall->count = TIME_NEW_COURSE_BALL;
			}
		} else {
			/* мячик примагничен к ракетке, уменьшим счетчик */
			lpBall->hand.count--;
			/*
			 * Если это старт раунда то сбросим все
			 * призы при достижении нуля счетчиком.
			 */
			if (lpBall->hand.count <= 0) {
				if (CHECK_FLAG(bonus.flags,F_START_RAUND))
					bonus.flags = 0;
			}
			/* расчитаем позицию мячика */
			lpBall->x = racket.x + lpBall->hand.offset;
			lpBall->y = racket.y - 6;
			/* получим целоцисленные значения координат */
			lpBall->ix = ftol(lpBall->x);
			lpBall->iy = ftol(lpBall->y);
		}

		/* проверим действие приза 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL)) {
			/* сформируем положение мячика */
			lpPicture->picture.X = lpBall->ix - 1;
			lpPicture->picture.Y = lpBall->iy - 1;
			/* сформируем положение тени */
			lpPicture->shadow.X = lpBall->ix + 6;
			lpPicture->shadow.Y = lpBall->iy + 6;
		} else {
			/* сформируем положение мячика */
			lpPicture->picture.X = lpBall->ix;
			lpPicture->picture.Y = lpBall->iy;
			/* сформируем положение тени */
			lpPicture->shadow.X = lpBall->ix + 7;
			lpPicture->shadow.Y = lpBall->iy + 7;
		}
	}

	/* проверим увеличение скорости */
	if (--timeAddSpeedBall <= 0) {
		/* увеличим скорость мячиков */
		for (i = 0; i < BALLS_COUNT; i++) {
			LPBALL lpBall = &ball[i];
			if (lpBall->on) {
				lpBall->v += ADD_SPEED_BALL;
				if (lpBall->v > MAX_SPEED_BALL)
					lpBall->v = MAX_SPEED_BALL;
				SetSpeedBall(lpBall);
			}
		}
		/* обновим счетчик увеличения скорости */
		timeAddSpeedBall = TIME_ADD_SPEED_BALL;
	}

	/* если действует приз 'BONUS_HAND' */
	if (CHECK_FLAG(bonus.flags,F_HAND)) {
		/* если нажата управляющая клавиша сбрасываем мячики */
		if (input.lButton || input.SPACE) {
			for (i = 0; i < BALLS_COUNT; i++)
				ball[i].hand.count = 0;
			/* сбросим все бонусы если это начало игры */
			if (CHECK_FLAG(bonus.flags,F_START_RAUND))
				bonus.flags = 0;
		}
	}
}

/* расчитывает скорость по осям 'x' и 'y' для летающего существа */
static void SetSpeedAlien(void)
{
	/* Эта функция полностью аналогична функции 'SetSpeedBall'. */
	double rad;

	/* переведем углы в радианы */
	rad = alien.angle * PI / 180.0;
	/* расчитаем скорость  по 'x' и 'y' */
	alien.vx = alien.v * cos(rad);
	alien.vy = alien.v * sin(rad);
}

/* создать летающее существо и разместить его */
static void CreateAlien(void)
{
	/* сбросим все поля летающего существа */
	memset(&alien,0,sizeof(alien));
	/* создадим летающее существо */
	alien.on = 1;
	/* установим координаты */
	if (random(2)) alien.x = LOGIC_X + 160;
	else alien.x = LOGIC_X + 55;
	/* получим целочисленные координаты */
	alien.ix = ftol(alien.x);
	alien.iy = ftol(alien.y);
	/* сбросим скорость */
	alien.v = 0.0;
	SetSpeedAlien();
	/* подготовим отображение и анимацию */
	scene.pAlien.type = 0;
	if (nLevel % 2) {
		scene.pAlien.show = SHOW_ALIEN_UFO;
		alien.animation.frames = lpAlienUfo;
	} else {
		scene.pAlien.show = SHOW_ALIEN_BIRD;
		alien.animation.frames = lpAlienBird;
	}
}

/* перегружает кадр изображения */
static void NextFrameAlien(void)
{
	int point = alien.animation.point;

	/* увеличим счетчик показа кадров */
	if (++alien.animation.count < alien.animation.frames[point].count)
		return;

	/* перегружаем кадр */
	point++;
	/* проверим конец кадрового буфера */
	if (alien.animation.frames[point].number < 0)
		point = 0;

	/* загрузим следующий кадр */
	scene.pAlien.type = alien.animation.frames[point].number;
	/* перегрузим управляющие переменные */
	alien.animation.point = point;
	alien.animation.count = 0;
}

/* состояния для конечного автомата ИИ */
#define STATE_RUN                   1    /* движение */
#define STATE_DETOUR                2    /* поиск обхода */
#define STATE_CHANGE_COURSE         3    /* изменение направления */
#define STATE_CHANGE_SPEED          4    /* изменение скорости */

/* скорость возвращения при уходе за стенки */
#define V_VIOLATION               0.8

/* сообщения для состояния 'DETOUR' */
#define DETOUR_VIOLATION_TOP        1    /* нарушение верхней границы */
#define DETOUR_VIOLATION_LEFT       2    /* нарушение левой границы */
#define DETOUR_VIOLATION_RIGHT      3    /* нарушение правой границы */

#define DETOUR_BLOCK_RIGHT          4    /* столкновение с правым блоком */
#define DETOUR_BLOCK_BOTTOM         5    /* столкновение с нижним блоком */
#define DETOUR_BLOCK_LEFT           6    /* столкновение с левым блоком */
#define DETOUR_BLOCK_TOP            7    /* столкновение с верхним блоком */

/* для передачи дополнительной информации состояниям */
static struct {
	int subject;                     /* кому преднозначено сообщение */
	int message;                     /* сообщение */
} info;

/* проверяет столкновение летающего существа и ракетки */
static void CollideAlienAndRacket(void)
{
	RECT r,a;

	/* расчитаем координаты ракетки */
	r.left = racket.x;
	r.top = racket.y;
	r.right = racket.x + GetWidthRacket() - 1;
	r.bottom = racket.y + GetHeightRacket() - 1;

	/* расчитаем координаты летающего существа */
	a.left = alien.ix + 5;
	a.top = alien.iy + 3;
	a.right = alien.ix + 18;
	a.bottom = alien.iy + 10;

	/* проверим попадание крайних точек ракетки в летающее существо */
	if (((r.left >= a.left && r.left <= a.right) ||
		(r.right >= a.left && r.right <= a.right)) &&
		((r.top >= a.top && r.top <= a.bottom) ||
		(r.bottom >= a.top && r.bottom <= a.bottom))) {
		/* ракетка пересекается с летающим существом */
		CreateBang();
		/* добавим бонусные очки за подбитое летающее существо */
		AddScore(350);
		return;
	}

	/* проверим попадание крайних точек летающего существа в ракетку */
	if (((a.left >= r.left && a.left <= r.right) ||
		(a.right >= r.left && a.right <= r.right)) &&
		((a.top >= r.top && a.top <= r.bottom) ||
		(a.bottom >= r.top && a.bottom <= r.bottom))) {
		/* летающее существо пересекается с ракеткой */
		CreateBang();
		/* добавим бонусные очки за подбитое летающее существо */
		AddScore(350);
		return;
	}
}

/* поверяет летающее существо на столкновение с объектами */
static void CollideAlien(void)
{
	BOUND bound;
	int b1,b2,b3,b4;

	/* проверим выход за нижнюю границу */
	if (alien.iy + 2 >= LOGIC_Y + LOGIC_HEIGHT) {
		/* перегрузим летающее существо */
		CreateAlien();
		return;
	}

	/* проверим выход за левую стенку */
	if (alien.ix + 7 <= LOGIC_X) {
		/* переключим состояние в 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* разместим дополнительную информацию */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_VIOLATION_LEFT;

		return;
	}

	/* проверим выход за правую стенку */
	if (alien.ix + 16 >= LOGIC_X + LOGIC_WIDTH) {
		/* переключим состояние в 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* разместим дополнительную информацию */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_VIOLATION_RIGHT;

		return;
	}

	/* проверка выхода за верхнюю границу */
	if (alien.iy + 6 <= LOGIC_Y) {
		/* переключим состояние в 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* разместим дополнительную информацию */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_VIOLATION_TOP;

		return;
	}

	/* проверим контрольные точки на столкновения */
	b1 = GetBlock(&bound,alien.ix+13,alien.iy+7);   /* крайняя правая точка */
	b2 = GetBlock(&bound,alien.ix+12,alien.iy+8);   /* крайняя нижняя точка */
	b3 = GetBlock(&bound,alien.ix+10,alien.iy+7);   /* крайняя левая точка */
	b4 = GetBlock(&bound,alien.ix+12,alien.iy+5);   /* крайняя верхняя точка */

	/* проверим столкновения */
	if (b1) {
		/* переключим состояние в 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* разместим дополнительную информацию */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_BLOCK_RIGHT;
	} else if (b2) {
		/* переключим состояние в 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* разместим дополнительную информацию */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_BLOCK_BOTTOM;
	} else if (b3) {
		/* переключим состояние в 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* разместим дополнительную информацию */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_BLOCK_LEFT;
	} else if (b4) {
		/* переключим состояние в 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* разместим дополнительную информацию */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_BLOCK_TOP;
	}
}

/* выполняет действие обхода */
static void StateDetourAlien(void)
{
	/* выполняем движение в заданом направлении */
	if (alien.automat.count > 0) {
		alien.automat.count--;
		return;
	}

	/* проверим, есть сообщение */
	if (info.subject == STATE_DETOUR) {
		/* проверим тип сообщения */
		switch (info.message) {
			case DETOUR_VIOLATION_LEFT:
			case DETOUR_BLOCK_LEFT:
				alien.angle = 340 + random(41);
				if (alien.angle >= 360)
					alien.angle -= 360;
				break;

			case DETOUR_VIOLATION_RIGHT:
			case DETOUR_BLOCK_RIGHT:
				alien.angle = 160 + random(41);
				break;

			case DETOUR_VIOLATION_TOP:
			case DETOUR_BLOCK_TOP:
				alien.angle = 70 + random(41);
				break;

			case DETOUR_BLOCK_BOTTOM:
				alien.angle = 250 + random(41);
				break;
		}
		/* перегрузим скорость */
		alien.v = V_VIOLATION;
		SetSpeedAlien();
		/* установим счетчик и сбросим сообщение */
		alien.automat.count = 2;
		info.subject = 0;
		return;
	}

	/* переключим состояние */
	alien.automat.state = STATE_CHANGE_COURSE;
	alien.automat.count = 0;
}

/* выполняем движение */
static void StateRunAlien(void)
{
	int state;

	/* выполняем движение в заданом направлении */
	if (alien.automat.count > 0) {
		alien.automat.count--;
		return;
	}

	/* выберем новое состояние */
	state = random(6);
	if (state == 0) {
		/* сохраним текушее состояние */
		alien.automat.count = 20 + random(30);
	} else if (state <= 2) {
		/* переключим на состояние изменения направления */
		alien.automat.state = STATE_CHANGE_COURSE;
	} else {
		/* переключим на состояние изменения скорости */
		alien.automat.state = STATE_CHANGE_SPEED;
	}
}

/* изменение направления */
static void StateChangeCourseAlien(void)
{
	/* расчитываем изменение курса */
	if (info.subject != STATE_CHANGE_COURSE) {
		/* выбираем знак прирашения */
		if (random(2))
			info.message = random(3) + 1;
		else
			info.message = -(random(3) + 1);
		/* получаем число итераций */
		alien.automat.count = 30 + random(30);
		/* запоминаем новый курс */
		info.subject = STATE_CHANGE_COURSE;
	}

	/* проверяем выход из состояния */
	if (alien.automat.count <= 0) {
		/* сбрасываем состояние в движение */
		alien.automat.state = STATE_RUN;
		info.subject = 0;
		/* уходим */
		return;
	}

	/* уменьшаем счетчик состояния */
	alien.automat.count--;
	/* изменяем курс */
	alien.angle += info.message;
	if (alien.angle < 0)
		alien.angle += 360;
	else if (alien.angle >= 360)
		alien.angle -= 360;
	SetSpeedAlien();
}

/* изменение скорости */
static void StateChangeSpeedAlien(void)
{
	/* изменяем скорость */
	if (random(2)) alien.v += 0.2;
	else alien.v -= 0.2;
	/* скоректируем результат */
	if (alien.v < 0.2) {
		alien.v = 0.2;
	} else if (alien.v > 1.0) {
		alien.v = 1.0;
	}

	/* переводим в состояние движение */
	alien.automat.state = STATE_RUN;
	alien.automat.count = 10 + random(20);
	/* изменяем скорость */
	SetSpeedAlien();
}

/* возвращает количество разбиваемых блоков */
static int GetCountBlock(void)
{
	int i,j,count;
	UCHAR (*p)[15];

	/* подготовим переменные */
	count = 0;
	p = &block[3];
	/* переберем все блоки */
	for (i = 0; i < 12; i++) {
		for (j = 0; j < 15; j++)
			if ((*p)[j] && !((*p)[j] & 0x80))
				count++;
		p++;
	}

	return count;
}

/* переместить летающее существо */
static void MoveAlien(void)
{
	/* летающее существо отображается или нет */
	if (!alien.on) {
		/* подготовим информацию о летающем существе */
		int level,AlienBlock;

		/* если отображается взрыв то уходим */
		if (bang.on || (bonus.flags & F_SMART_BOMB))
			return;

		/* подготовим информацию о летающем существе */
		level = nLevel % LEVEL_COUNT;
		AlienBlock = levels[level].AlienBlock;

		/* проверим запрет летающего существа */
		if (!AlienBlock)
			return;

		/* проверим условие появления летающего существа */
		if (GetCountBlock() > AlienBlock)
			return;

		/* создадим летающее существо */
		CreateAlien();
	}

	/* переместим летающее существо */
	alien.x += alien.vx;
	alien.y += alien.vy;
	/* получим целочисленные координаты */
	alien.ix = ftol(alien.x);
	alien.iy = ftol(alien.y);

	/* проверяем на столкновения, если состояние не 'обход' */
	if (alien.automat.state != STATE_DETOUR)
		CollideAlien();

	/* проверим столкновение с ракеткой */
	CollideAlienAndRacket();
	/* проверим, что летающее существо еще отображается */
	if (!alien.on)
		return;

	/* выполним действие в зависимости от состояния */
	switch (alien.automat.state) {
		/* движение */
		case STATE_RUN:
			StateRunAlien();
			break;
		/* обход препятствия */
		case STATE_DETOUR:
			StateDetourAlien();
			break;
		/* изменение направления */
		case STATE_CHANGE_COURSE:
			StateChangeCourseAlien();
			break;
		/* изменение скорости */
		case STATE_CHANGE_SPEED:
			StateChangeSpeedAlien();
			break;
		/* устанавливаем состояние по умолчанию */
		default:
			alien.automat.state = STATE_RUN;
			alien.automat.count = 0;
	}

	/* загрузим следующий кадр изображения */
	NextFrameAlien();
	/* обновим координаты */
	scene.pAlien.X = alien.ix;
	scene.pAlien.Y = alien.iy;
}

/* перемещаем бомбу */
static void TransferBomb(void)
{
	RECT r;
	POINT p;

	/* переместим бомбу */
	bomb.y += bomb.vy;
	/* расчитаем скорость */
	if (bomb.vy < MAX_SPEED_Y) {
		/* просчитаем ускарение */
		bomb.vy += ACCELERATION_Y;
		if (bomb.vy > MAX_SPEED_Y)
			bomb.vy = MAX_SPEED_Y;
	}
	/* получим целочисленную координату 'y' */
	bomb.iy = ftol(bomb.y);

	/* расчитаем координаты ракетки */
	r.left = racket.x;
	r.top = racket.y;
	r.right = racket.x + GetWidthRacket() - 1;
	r.bottom = racket.y + GetHeightRacket() - 1;

	/* расчитаем точку в бомбе */
	p.x = bomb.ix + 3;
	p.y = bomb.iy + 5;

	/* проверим поподание в ракетку */
	if ((p.x >= r.left && p.x <= r.right) &&
		(p.y >= r.top && p.y <= r.bottom)) {
		/* разрушаем ракетку */
		SetFlySliverAnima();
		return;
	}

	/* проверим пересечение нижней границы */
	if (bomb.iy > (LOGIC_Y + LOGIC_HEIGHT)) {
		/* бомба вышла за экран */
		bomb.on = 0;
		scene.pBomb.picture.show = 0;
		scene.pBomb.shadow.show = 0;
		/* перегрузим счетчик задержки */
		bomb.count = 25;
	} else {
		/*
		 * Перегрузим координаты бомбы и ее тени
		 * (координата 'x' не изменяется, падает вниз).
		 */
		scene.pBomb.picture.Y = bomb.iy;
		scene.pBomb.shadow.Y = bomb.iy + 7;
	}
}

/* разместить бомбу */
static void CreateBomb(void)
{
	/* проверим счетчик задержки */
	if (bomb.count > 0) {
		bomb.count--;
		return;
	}
	/* внесем немного случайности */
	if (random(3)) {
		bomb.count = 10 + random(20);
		return;
	}

	/* размещаем бомбу */
	bomb.on = 1;
	bomb.ix = alien.ix + 8;
	bomb.iy = alien.iy + 4;
	/* подготовим рабочие переменные */
	bomb.y = bomb.iy;
	bomb.vy = 0.0;

	/* отобразим бомбу и тень */
	scene.pBomb.picture.show = 1;
	scene.pBomb.picture.X = bomb.ix;
	scene.pBomb.picture.Y = bomb.iy;
	scene.pBomb.shadow.show = 1;
	scene.pBomb.shadow.X = bomb.ix + 8;
	scene.pBomb.shadow.Y = bomb.iy + 7;
}

/* переместить бомбу */
static void MoveBomb(void)
{
	/* если бомба отображается, то перемещаем ее */
	if (bomb.on)
		TransferBomb();
	/* разместим бомбу если крылатое существо отображается */
	else if (alien.on)
		CreateBomb();
}

/* сбросить ранее пойманый приз */
static void ResetOldBonus(void)
{
	/* сбрасываем действующие призы */
	if (CHECK_FLAG(bonus.flags,F_GUN)) {
		/* сделаем ракетку нормальной */
		racket.animation.frames = lpGunToNormalRacket;
		racket.animation.point = 0;
		racket.animation.count = 0;
	} else if (CHECK_FLAG(bonus.flags,F_EXTENDED_RACKET)) {
		/* сделаем ракетку нормальной */
		racket.animation.frames = lpExtendedToNormalRacket;
		racket.animation.point = 0;
		racket.animation.count = 0;
	} else if (CHECK_FLAG(bonus.flags,F_HAND)) {
		int i;
		/* сбросим все примагниченные мячики */
		for (i = 0; i < BALLS_COUNT; i++)
			ball[i].hand.count = 0;
	} else if (CHECK_FLAG(bonus.flags,F_SMASH_BALL)) {
		int i;
		/* востановим обычный вид мячиков */
		for (i = 0; i < BALLS_COUNT; i++) {
			LPFULL_PICTURE lpPicture = &scene.pBall[i];
			lpPicture->picture.type = BALL_NORMAL;
			lpPicture->shadow.type = BALL_NORMAL;
		}
	}

	/* сбрасываем флаги */
	bonus.flags &= ~(F_POINTS | F_SMART_BOMB | F_SLOW_BALL |
		F_SMASH_BALL | F_HAND | F_GUN | F_EXTENDED_RACKET | F_ROCKET_PACK);
}

/* пойманы бонусные очки */
static void RunBonusPoints(void)
{
	/* добавим призовые очки */
	AddScore(5000);

	/* установим флаг действия приза */
	bonus.flags |= F_POINTS;
}

/* пойман приз "Kill Aliens" */
static void RunBonusSmartBomb(void)
{
	/* удалим летающее существо */
	if (alien.on)
		CreateBang();

	/* установим флаг действия приза */
	bonus.flags |= F_SMART_BOMB;
}

/* поймана дополнительная жизнь */
static void RunBonusExtraLife(void)
{
	/* добавим жизнь */
	lifes++;

	/* установим флаг действия приза */
	bonus.flags |= F_EXTRA_LIFE;
}

/* пойман бонус "замедление мячика" */
static void RunBonusSlowBall(void)
{
	int i;

	/* переберем все мячики и сбросим скорость */
	for (i = 0; i < BALLS_COUNT; i++) {
		if (ball[i].on) {
			ball[i].v = MIN_SPEED_BALL;
			SetSpeedBall(&ball[i]);
		}
	}

	/* перегрузим счетчик увеличения скорости */
	timeAddSpeedBall = TIME_ADD_SPEED_BALL / 2;

	/* установим флаг действия приза */
	bonus.flags |= F_SLOW_BALL;
}

/* пойман бонус "болшой мячик" */
static void RunBonusSmashBall(void)
{
	int i;

	/* установим большой мячик */
	for (i = 0; i < BALLS_COUNT; i++) {
		LPFULL_PICTURE lpPicture = &scene.pBall[i];
		lpPicture->picture.type = BALL_SMASH;
		lpPicture->shadow.type = BALL_SMASH;
	}

	/* установим время действия приза */
	timeEndingSmashBall = TIME_ENDING_SMASH_BALL;

	/* установим флаг действия приза */
	bonus.flags |= F_SMASH_BALL;
}

/* пойман бонус "три мячика" */
static void RunBonusTripleBall(void)
{
	FULL_PICTURE pBall;
	BALL lBall;
	int angle,angle1,angle2;
	int i;

	/* ищем отображаемый мячик */
	for (i = 0; i < BALLS_COUNT; i++) {
		if (ball[i].on) {
			/* запомним описание мячика */
			memcpy(&pBall,&scene.pBall[i],sizeof(pBall));
			memcpy(&lBall,&ball[i],sizeof(lBall));
			break;
		}
	}

	/* проверим, что мячик найден */
	if (i == BALLS_COUNT)
		return;

	/* получим угол движения мячика */
	if (lBall.swerve.lock)
		angle = lBall.swerve.angle;
	else
		angle = lBall.angle;

	/*
	 * Установим значения углов.
	 * Такая сложная схема расчета углов мячика происходит
	 * по тому, что мячик в момент его дублирования может быть
	 * захвачен 'кругом откланения'. Два новых мячика создаются
	 * не захваченными 'кругом откланения'.
	 */
	if (angle < 90) {
		switch (angle) {
			case 23: angle1 = 45; angle2 = 67; break;
			case 45: angle1 = 23; angle2 = 67; break;
			case 67: angle1 = 23; angle2 = 45; break;
			default: return;
		}
	} else if (angle < 180) {
		switch (angle) {
			case 113: angle1 = 135; angle2 = 157; break;
			case 135: angle1 = 113; angle2 = 157; break;
			case 157: angle1 = 113; angle2 = 135; break;
			default: return;
		}
	} else if (angle < 270) {
		switch (angle) {
			case 203: angle1 = 225; angle2 = 247; break;
			case 225: angle1 = 203; angle2 = 247; break;
			case 247: angle1 = 203; angle2 = 225; break;
			default: return;
		}
	} else {
		switch (angle) {
			case 293: angle1 = 315; angle2 = 337; break;
			case 315: angle1 = 293; angle2 = 337; break;
			case 337: angle1 = 293; angle2 = 315; break;
			default: return;
		}
	}

	/* дублируем мячики */
	memcpy(&scene.pBall[0],&pBall,sizeof(pBall));
	memcpy(&ball[0],&lBall,sizeof(lBall));
	SetSpeedBall(&ball[0]);

	memcpy(&scene.pBall[1],&pBall,sizeof(pBall));
	memcpy(&ball[1],&lBall,sizeof(lBall));
	ball[1].count = TIME_NEW_COURSE_BALL;
	ball[1].swerve.lock = 0;
	ball[1].angle = angle1;
	SetSpeedBall(&ball[1]);

	memcpy(&scene.pBall[2],&pBall,sizeof(pBall));
	memcpy(&ball[2],&lBall,sizeof(lBall));
	ball[2].count = TIME_NEW_COURSE_BALL;
	ball[2].swerve.lock = 0;
	ball[2].angle = angle2;
	SetSpeedBall(&ball[2]);

	/* установим флаг действия приза */
	bonus.flags |= F_TRIPLE_BALL;
}

/* пойман бонус "рука" */
static void RunBonusHand(void)
{
	/* установим флаг действия приза */
	bonus.flags |= F_HAND;
}

/* пойман бонус "стреляющая ракетка" */
static void RunBonusGun(void)
{
	/* загружаем анимацию перехода ракетки */
	if (racket.animation.frames == NULL) {
		racket.animation.frames = lpNormalToGunRacket;
		racket.animation.point = 0;
		racket.animation.count = 0;
	} else
		racket.frames = lpNormalToGunRacket;

	/* установим флаг действия приза */
	bonus.flags |= F_GUN;
}

/* пойман бонус "большая ракетка" */
static void RunBonusExtendedRacket(void)
{
	/* загружаем анимацию перехода ракетки */
	if (racket.animation.frames == NULL) {
		racket.animation.frames = lpNormalToExtendedRacket;
		racket.animation.point = 0;
		racket.animation.count = 0;
	} else
		racket.frames = lpNormalToExtendedRacket;

	/* установим флаг действия приза */
	bonus.flags |= F_EXTENDED_RACKET;
}

/* пойман бонус "ракета" */
static void RunBonusRocketPack(void)
{
	/* запустим анимацию полета на ракете */
	SetFlyRocketAnima();
	/* озвучим событие */
	play.rocket = 1;
	/* установим флаг действия приза */
	bonus.flags |= F_ROCKET_PACK;
}

/* обрабатываем пойманый приз */
static void RunBonus(void)
{
	/* сбросим ранее пойманый приз */
	ResetOldBonus();

	switch (scene.pBonus.type) {
		case BONUS_POINTS:
			RunBonusPoints();
			break;
		case BONUS_SMART_BOMB:
			RunBonusSmartBomb();
			break;
		case BONUS_EXTRA_LIFE:
			RunBonusExtraLife();
			break;
		case BONUS_SLOW_BALL:
			RunBonusSlowBall();
			break;
		case BONUS_SMASH_BALL:
			RunBonusSmashBall();
			break;
		case BONUS_TRIPLE_BALL:
			RunBonusTripleBall();
			break;
		case BONUS_HAND:
			RunBonusHand();
			break;
		case BONUS_GUN:
			RunBonusGun();
			break;
		case BONUS_EXTENDED_RACKET:
			RunBonusExtendedRacket();
			break;
		case BONUS_ROCKET_PACK:
			RunBonusRocketPack();
			break;
	}
}

/* создает отскакивающие очки от ракетки */
static void CreateBonus400Points(void)
{
	int angle,r,b;
	double rad,speed;

	/* озвучим событие, если поймана не ракета */
	if (scene.pBonus.type != BONUS_ROCKET_PACK)
		play.bonus = 1;

	/* прибавим бонусные очки */
	AddScore(400);
	/* обработаем пойманый приз */
	RunBonus();

	/* расчитываем центральные точки объектов */
	r = racket.x + GetWidthRacket() / 2;
	b = bonus.ix + GetWidthBonus() / 2;

	/* расчитываем угол в зависимости от места пересечения */
	if (scene.pRacket.type != TYPE_RACKET_EXTEND) {
		if (b < (r - 9)) {
			angle = 198 + random(10);
			speed = 1.05;
		} else if (b < (r - 6)) {
			angle = 220 + random(10);
			speed = 0.95;
		} else if (b < r) {
			angle = 242 + random(10);
			speed = 0.85;
		} else if (b <= (r + 6)) {
			angle = 288 + random(10);
			speed = 0.85;
		} else if (b <= (r + 9)) {
			angle = 310 + random(10);
			speed = 0.95;
		} else {
			angle = 332 + random(10);
			speed = 1.05;
		}
	} else {
		if (b < (r - 15)) {
			angle = 198 + random(10);
			speed = 1.05;
		} else if (b < (r - 10)) {
			angle = 220 + random(10);
			speed = 0.95;
		} else if (b < r) {
			angle = 242 + random(10);
			speed = 0.85;
		} else if (b <= (r + 10)) {
			angle = 288 + random(10);
			speed = 0.85;
		} else if (b <= (r + 15)) {
			angle = 310 + random(10);
			speed = 0.95;
		} else {
			angle = 332 + random(10);
			speed = 1.05;
		}
	}

	/* переведем угол в радианы */
	rad = angle * PI / 180.0;
	/* расчитываем скорости по осям */
	bonus.vx = speed * cos(rad);
	bonus.vy = speed * sin(rad);

	/* устанавливаем тип отображаемого бонуса */
	scene.pBonus.type = BONUS_400_POINTS;
}

/* переместим падающий приз */
static void MoveBonus(void)
{
	/* если приз не отображается то уходим */
	if (!bonus.on)
		return;

	/* отображается нормальный приз */
	if (scene.pBonus.type != BONUS_400_POINTS) {
		RECT r,b;
		/* переместим приз */
		bonus.y += bonus.vy;
		/* обновим скорость */
		if (bonus.vy < MAX_SPEED_Y) {
			/* просчитаем ускорение */
			bonus.vy += ACCELERATION_Y;
			if (bonus.vy > MAX_SPEED_Y)
				bonus.vy = MAX_SPEED_Y;
		}
		/* получим целочисленные значения координат */
		bonus.iy = ftol(bonus.y);
		/* расчитаем координаты ракетки */
		r.left = racket.x + 4;
		r.top = racket.y + 8;
		r.right = racket.x + GetWidthRacket() - 1 - 4;
		r.bottom = racket.y + GetHeightRacket() - 1;
		/* расчитаем координаты бонуса */
		b.left = bonus.ix;
		b.top = bonus.iy;
		b.right = bonus.ix + GetWidthBonus() - 1;
		b.bottom = bonus.iy + GetHeightBonus() - 1;
		/* проверим попадание крайних точек ракетки в приз */
		if (((r.left >= b.left && r.left <= b.right) ||
			(r.right >= b.left && r.right <= b.right)) &&
			((r.top >= b.top && r.top <= b.bottom) ||
			(r.bottom >= b.top && r.bottom <= b.bottom))) {
			/* ракетка пересекается с бонусом */
			CreateBonus400Points();
		} else if (((b.left >= r.left && b.left <= r.right) ||
			(b.right >= r.left && b.right <= r.right)) &&
			((b.top >= r.top && b.top <= r.bottom) ||
			(b.bottom >= r.top && b.bottom <= r.bottom))) {
			/* бонус пересекается с ракеткой */
			CreateBonus400Points();
		}
		/* перегрузим координаты объекта */
		scene.pBonus.Y = bonus.iy;
		scene.pBonus.X = bonus.ix;
	} else {
		/* получим ширину бонуса */
		int width = GetWidthBonus();
		/* переместим приз */
		bonus.y += bonus.vy;
		bonus.x += bonus.vx;
		/* прибавим ускорение */
		bonus.vy += 0.04;
		/* получим целочисленные значения координат */
		bonus.iy = ftol(bonus.y);
		bonus.ix = ftol(bonus.x);
		/* проверим столкновение со стенками */
		if ((bonus.ix <= LOGIC_X) && (bonus.vx < 0)) {
			bonus.vx = -bonus.vx;
			bonus.ix = LOGIC_X;
		} else if ((bonus.ix >= (LOGIC_X + LOGIC_WIDTH - width)) && (bonus.vx > 0)) {
			bonus.vx = -bonus.vx;
			bonus.ix = LOGIC_X + LOGIC_WIDTH - width;
		}
		/* перегрузим координаты объекта */
		scene.pBonus.Y = bonus.iy;
		scene.pBonus.X = bonus.ix;
	}

	/* проверим выход за пределы игрового поля */
	if (bonus.iy > (LOGIC_Y + LOGIC_HEIGHT)) {
		/* разрушаем объект */
		bonus.on = 0;
		scene.pBonus.show = 0;
	}
}

/* обработаем круг откланения */
static void MoveSwerves(void)
{
	int i;

	/* переберем все круги откланения */
	for (i = 0; i < SWERVES_COUNT; i++) {
		/* сформируем указатели */
		LPPICTURE lpPicture = &scene.pSwerves[i];
		LPSWERVE lpSwerve = &swerve[i];

		/* если круг отображается, обработаем его */
		if (lpPicture->show) {
			/* проверим счетчик смены состояния */
			if (--lpSwerve->count > 0)
				continue;
			/*
			 * Меняем состояние круга откланения.
			 * Вероятность перехода в включенное состояние равно 55%.
			 * Вероятность перехода в выключенное состояние равно 45%.
			 */
			if (random(100) < 45) {
				if (lpSwerve->on) {
					lpSwerve->on = 0;
					lpPicture->type = PICTURE_SWERVE_OFF;
					/* озвучим событие */
					play.swerve = 1;
				}
			} else {
				if (!lpSwerve->on) {
					lpSwerve->on = 1;
					lpPicture->type = PICTURE_SWERVE_ON;
					/* озвучим событие */
					play.swerve = 1;
				}
			}
			/* перегрузим счетчик смены состояния */
			lpSwerve->count = 30 + random(100);
		}
	}
}

/* отобразить кадр взрыва летающего существа */
static void MoveBang(void)
{
	/* проверим отображение взрыва */
	if (!bang.on)
		return;

	/* увеличим счетчик показа кадров */
	if (++bang.count < lpBang[bang.point].count)
		return;

	/* перегружаем кадр */
	bang.point++;
	bang.count = 0;
	/* проверим конец анимации */
	if (lpBang[bang.point].number < 0) {
		bang.on = 0;
		scene.pBang.show = 0;
		return;
	}

	/* загрузим следующий кадр */
	scene.pBang.type = lpBang[bang.point].number;
}

/* обрабатывает столкновение пулек с блоком или летающим существом */
static void CheckCollisionBullet(LPPICTURE lpBullet, LPBULLET_PLOP lpPlop)
{
	BOUND bound1,bound2;
	int b1,b2,y;

	/* получим блоки содержащие проверяемые точки */
	b1 = GetBlock(&bound1,lpBullet->X,lpBullet->Y);     /* верхняя левая */
	b2 = GetBlock(&bound2,lpBullet->X+3,lpBullet->Y);   /* верхняя правая */

	/* проверим столкновение с блоками */
	if ((b1 && b2) && (bound1.x != bound2.x)) {
		/* определим на какой блок пулька накладывается больше */
		if ((16 - bound1.offset_x) >= bound2.offset_x) {
			MarkBlock(bound1.x,bound1.y);
		} else {
			MarkBlock(bound2.x,bound2.y);
		}
		/* скоректируем координату 'y' */
		y = (bound1.y + 1) * 8 + LOGIC_Y;
	} else if (b1) {
		/* пометим блок */
		MarkBlock(bound1.x,bound1.y);
		/* скоректируем координату 'y' */
		y = (bound1.y + 1) * 8 + LOGIC_Y;
	} else if (b2) {
		/* пометим блок */
		MarkBlock(bound2.x,bound2.y);
		/* скоректируем координату 'y' */
		y = (bound2.y + 1) * 8 + LOGIC_Y;
	} else {
		/* проверим столкновение с летающим существом */
		RECT a;
		int x1,x2,y1;

		/* отображается летающее существо или нет */
		if (!alien.on)
			return;

		/* расчитаем координаты летающего существа */
		a.left = alien.ix + 5;
		a.top = alien.iy + 3;
		a.right = alien.ix + 18;
		a.bottom = alien.iy + 10;
		/* расчитаем контрольные точки пульки */
		x1 = lpBullet->X + 1;
		x2 = lpBullet->X + 2;
		y1 = lpBullet->Y;

		/* пулька не попадает в летающее существо */
		if (((x1 < a.left) && (x2 < a.left)) ||
			((x1 > a.right) && (x2 > a.right)))
			return;
		if ((y1 < a.top) || (y1 > a.bottom))
			return;

		/* сформируем анимацию взрыва */
		CreateBang();
		/* добавим бонусные очки за подбитое летающее существо */
		AddScore(350);

		/* скоректируем координату 'y' */
		y = y1;
	}

	/* сформируем рисунок хлопка */
	lpBullet->type = TYPE_BULLET_PLOP_0;
	lpBullet->X -= 2;
	lpBullet->Y = y;
	/* сформируем анимацию хлопка пульки */
	lpPlop->count = 0;
	lpPlop->point = 0;
}

/* переместить пульки */
static void MoveBullet(void)
{
	int i;

	/* переберем все пульки */
	for (i = 0; i < BULLETS_COUNT; i++) {
		LPPICTURE lpBullet = &scene.pBullet[i];
		LPBULLET_PLOP lpPlop = &plop[i];
		/* проверим что пулька существует */
		if (lpBullet->show) {
			/* если это пулька то перемещаем ее */
			if (lpBullet->type < TYPE_BULLET_PLOP_0) {
				lpBullet->Y -= SPEED_BULLET;
				/* проверим выход за пределы игрового окна */
				if (lpBullet->Y <= -8) {
					lpBullet->show = 0;
					continue;
				}
				/* изменим рисунок */
				if (lpBullet->type == TYPE_BULLET_0)
					lpBullet->type = TYPE_BULLET_1;
				else
					lpBullet->type = TYPE_BULLET_0;
				/* проверим столкновение с блоком или летающим существом */
				CheckCollisionBullet(lpBullet,lpPlop);
			} else {
				/* увеличим счетчик показа кадров */
				if (++lpPlop->count < lpBulletPlop[lpPlop->point].count)
					continue;
				/* парегружаем кадр */
				lpPlop->point++;
				lpPlop->count = 0;
				/* проверим конец анимации */
				if (lpBulletPlop[lpPlop->point].number < 0) {
					lpBullet->show = 0;
					continue;
				}
				/* загрузим следующий кадр */
				lpBullet->type = lpBulletPlop[lpPlop->point].number;
			}
		}
	}
}

/*
 * Вызывает обработчик анимации.
 *
 * Анимация используется для следующих состояний:
 * 1 - Начало раунда. Показывает надпись начала
 *     раунда и т.д. (StartRaundAnima)
 * 2 - Окончание раунда. Используется для реализации
 *     задержки. (EndRaundAnima)
 * 3 - Окончание пауза. Показывает отсчет до начала
 *     игры. (PauseEndAnima)
 * 4 - Разлет осколков ракетки. Анимирует разлет осколков
 *     ракетки. (FlySliverAnima)
 * 5 - Полет на ракете (переход в следующий раунд). Анимирует
 *     полет на ракете и накрутка очков за оставшиеся
 *     кирпичи. (FlyRocketAnima).
 */
static int MoveAnima(void)
{
	/* если указан обработчик анимации, вызываем его */
	if (anima.func != NULL)
		return (*anima.func)();

	return 0;
}

/*
 * Выполнить перемещение всех фигур.
 *
 * Возвращаемое значение:
 * 0 - идет игра.
 * 1 - раунд пройден.
 * 2 - игрок проиграл.
 */
int GameMoveScene(void)
{
	/* проверить управляющие клавиши */
	CheckControlKey();

	/* обработаем паузу */
	if (pause.pause)
		return 0;

	/*
	 * Вызвать функцию обработки анимации.
	 *
	 * Возвращаемое значение функцией анимации и действие на него.
	 * 0 - передает управление дальше, т.е. идет обычное
	 *     перемещение фигур.
	 * 1 - передает управление на копирование блоков,
	 *     перемешение фигур не выполняется.
	 * 2 - заставляет возвратить '0'.
	 * 3 - заставляет возвратить '1'.
	 * 4 - заставляет возвратить '2'.
	 */
	switch (MoveAnima()) {
		/* передаем управление на копирование блоков */
		case 1: goto COPY_BLOCK;
		/* возвратим нуль (идет игра) */
		case 2: return 0;
		/* возвратим единицу (раунд пройден) */
		case 3: return 1;
		/* возвратим двойку  (игрок проиграл) */
		case 4: return 2;
	}

	/* обработаем круг откланения */
	MoveSwerves();
	/* переместим летающее существо */
	MoveAlien();
	/* переместим бомбу */
	MoveBomb();
	/* переместим падающий приз */
	MoveBonus();
	/* переместим взрыв летающего существа */
	MoveBang();
	/* переместим пульки */
	MoveBullet();
	/* переместим ракетку */
	MoveRacket();
	/* переместим мячик и проверим столкновения */
	MoveBall();

COPY_BLOCK:
	/* копируем призовые очки */
	scene.score.up1 = score.up1;
	scene.score.up2 = score.up2;
	scene.score.hi = score.hi;
	/* копируем жизни */
	scene.lifes = lifes;
	/* копируем блоки для отображения */
	CopyBlock();

	/* если раунд пройден то запустим анимацию окончания */
	if (GetCountBlock() == 0)
		SetEndRoundAnima();

	return 0;
}

/* создание уровня */
void GameCreateLevel(int level)
{
	/* сохраним номер уровня */
	nLevel = level;

	/* очистим сцену */
	memset(&scene,0,sizeof(scene));

	/* установим фоновое изображение */
	scene.on = 1;
	scene.background = (level % LEVEL_COUNT) % BACKGROUND_COUNT;

	/* получим указатель на текущий раунд */
	raund = levels[level%LEVEL_COUNT].round;

	/* заполним поле кирпичами */
	memset(block,0,sizeof(block));
	memcpy(&block[3][0],raund,sizeof(UCHAR)*(15*12));

	/* загрузим круги откланения */
	SetSwerve();

	/* очистим структуры объектов */
	memset(EventBlock,0,sizeof(EventBlock));
	memset(&racket,0,sizeof(racket));
	memset(ball,0,sizeof(ball));
	memset(&alien,0,sizeof(alien));
	memset(&bang,0,sizeof(bang));
	memset(&bomb,0,sizeof(bomb));
	memset(&bonus,0,sizeof(bonus));
	memset(&anima,0,sizeof(anima));
	memset(sliver,0,sizeof(sliver));

	/* если это начало игры, то сбрасываем очки */
	if (!level) {
		scene.score.up1 = score.up1 = 0;
		scene.score.up2 = score.up2 = 0;
		score.addLifes = 30000;
		/* если игра только начилась, то 'hiscore' = 100000 */
		if (!score.hi) score.hi = 100000;
		scene.score.hi = score.hi;
		/* установим жизнь */
		scene.lifes = lifes = LIFE_COUNT;
	} else {
		scene.score.up1 = score.up1;
		scene.score.up2 = score.up2;
		scene.score.hi = score.hi;
		/* установим жизнь */
		scene.lifes = lifes;
	}

	/* копируем блоки для отображения */
	CopyBlock();

	/* запускаем анимацию старта раунда */
	SetStartRoundAnima();
}
