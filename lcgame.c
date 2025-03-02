/*
 * lggame.c
 *
 * ��������: ��� ���������� ����� � ������ ������������� �
 * ��������� � ����������� ���� � ��������� (256,192) �
 * �������� ������ � (0,0). � �����, � ������ 'draw.h'
 * ��� ����������������� � �������� ����������, �������
 * ����������� �������� � ����������� ����, ��� ���� ��� ��
 * ��� ���� ���������� �������. (�� 'global.h')
 */
#include "global.h"
#include <stdlib.h>
#include <math.h>
#include "lcgame.h"
#include "dwgame.h"
#include "input.h"
#include "sound.h"

/* ���������� ������ � ������ ���� */
#define LIFE_COUNT             2

/* ���������� ���������� ������� ���� */
#define LOGIC_WIDTH            (WIN_GAME_WIDTH - 8 - 8)
#define LOGIC_HEIGHT           (WIN_GAME_HEIGHT - 8)
#define LOGIC_X                8
#define LOGIC_Y                8

/* pi */
#define PI                3.14159265358979

/* ���������� ������� � ������ ���� */
#define RACKET_X              (LOGIC_X+108)
#define RACKET_Y              (LOGIC_Y+165)
/* ��������� ��� ������ ��������� ����������� ������� */
#define RACKET_FACTOR               0.3

/* �������� ������ */
#define START_SPEED_BALL            2.8
#define MIN_SPEED_BALL              2.0
#define MAX_SPEED_BALL              5.6
#define ADD_SPEED_BALL              0.8

/*
 * ����� ����� ������� ����� ������� ���� �����������,
 * ���� �� �� ���������� � �������� ��� �� ��������
 * ������ ��� �� ������� � �������� �������� (15 ���).
 */
#define TIME_NEW_COURSE_BALL       (15 * FPS)
/* ����� ����� ������� ������������� �������� ������ (15 ���) */
#define TIME_ADD_SPEED_BALL        (15 * FPS)
/* ����� ����� ������� ����� �������� �� ���� (4 ���) */
#define TIME_ENDING_HAND            (4 * FPS)
/* ����� ����� ������� ����� ������� ������ ������� (6 ���) */
#define TIME_ENDING_SMASH_BALL      (6 * FPS)

/* �������� ���������� �������� ������� */
#define SPEED_SLIVER               1.65

/* ������ ����� ���������� */
#define SWERVE_RADIUS                11

/* ��������� �������� ����� �� ��� 'y' */
#define ACCELERATION_Y            0.029
/* ������������ �������� �������� ������ */
#define MAX_SPEED_Y                 1.9

/*
 * ����������� ��������� ����� ������� � ��������
 * ��� ������� ����� ��������� ����� ������ (� ������).
 */
#define MIN_SPACE_BULLET             44
/* �������� �������� ������ */
#define SPEED_BULLET                  6

/* ��������� ���� ���������� */
typedef struct {
	int on;                 /* ���� ������� */
	int count;              /* ������� �� ���������� ��������� */
	int x,y;                /* ����������� ����� ����� */
} SWERVE, *LPSWERVE;

/* ��������� ���� � ������� ������������� ����� */
typedef struct {
	int x,y;                    /* ��� ��������� � ������ */
	int offset_x, offset_y;     /* ��������� ������� ����� ����� */
} BOUND, *LPBOUND;

/* ��������� ����� */
typedef struct {
	int on;                /* ����� ������������ */
	int count;             /* ������� ��������� ����������� */
	int angle;             /* ���� ����������� �������� (� ��������) */
	/*
	 * ��� ��������� �������� ��� ������� �����������
	 * � 'double' � ����������� �� ����������� � 'int'.
	 */
	int ix,iy;             /* ������������� ���������� */
	double v;              /* �������� ������ */
	double vx,vy;          /* �������� �������� �� ���������� 'x' � 'y' */
	double x,y;            /* ���������� */
	/* �������������� ������ � ������ ���������� */
	struct {
		int lock;          /* ���� �������� ����� */
		int angle;         /* ���� � �������� ���� ���������� */
		int num;           /* ����� ����� ���������� ������������ ����� */
	} swerve;
	/* ������������� ������ � ������ 'BONUS_HAND' */
	struct {
		int count;         /* ��������� ������� */
		int offset;        /* � �������������� ��������� �������� ������ */
	} hand;
} BALL, *LPBALL;

/* ��������� ���� ���� �������� */
typedef struct {
	int number;                     /* ����� ����� */
	int count;                      /* ����� ���������� ����� */
} FRAME;

/* ��������� �������� */
typedef struct {
	int point;                   /* ������� ������������ ���� */
	int count;                   /* ������� ���������� ����� */
	const FRAME *frames;         /* ��������� �� ����� � ������� */
} ANIMATION;

/* ��������� ������� */
typedef struct {
	int on;                          /* ������� ������������ */
	int x,y;                         /* ���������� ������� */
	const FRAME *frames;             /* ��������� �� ��������� ����� � ������� */
	ANIMATION animation;             /* �������� ��������� ������� */
} RACKET;

/* ��������� �������� �������� */
typedef struct {
	int on;                         /* 'alien' ������������ */
	int angle;                      /* ���� ����������� �������� (� ��������) */
	int ix,iy;                      /* ������������� ���������� */
	double v;                       /* �������� 'alien' */
	double vx,vy;                   /* �������� �������� �� 'x' � �� 'y' */
	double x,y;                     /* ���������� */
	ANIMATION animation;            /* �������� ��������� �������� */
	/* ��������� ��������� �������� �� */
	struct {
		int state;                  /* ��������� ��������� �������� */
		int count;                  /* ������� �� ���������� ��������� */
	} automat;
} ALIEN;

/* ��������� ����� ��������� �������� */
typedef struct {
	int on;                         /* ����� ������������ */
	int point;                      /* ������� ������������ ���� */
	int count;                      /* ������� ���������� ������ */
} BANG;

/* ��������� ����� */
typedef struct {
	int on;                         /* ����� ������������ */
	int ix,iy;                      /* ������������� ���������� */
	int count;                      /* ����� ������ �� �������� ����� */
	double vy;                      /* �������� ����� (�������� � ��������� 'y') */
	double y;                       /* ���������� 'y' */
} BOMB;

/* ����������� ����� ������ */
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
/* ���� ������ �������� ������ */
#define F_START_RAUND              (1<<31)

/* �������� ����� */
#define CHECK_FLAG(val,flags)       (val & flags)

/* ��������� ���� */
typedef struct {
	int on;                         /* ���� ������������ */
	int ix,iy;                      /* ������������� ���������� */
	unsigned int flags;             /* ����������� ����� */
	double vx,vy;                   /* �������� �� ��������������� ���������� */
	double x,y;                     /* ���������� */
} BONUS;

/* ��������� ������ ������ */
typedef struct {
	int point;                      /* ������� ������������ ���� */
	int count;                      /* ������� ���������� ������ */
} BULLET_PLOP, *LPBULLET_PLOP;

/* ��������� ������� �������� */
typedef struct {
	int state;                      /* ������� ��������� �������� */
	int count;                      /* ������� ��������� */
	int (*func)(void);              /* ��������� �� ������� ���������� �������� */
} ANIMA;

/* ��������� ������� ������� */
typedef struct {
	double vx,vy;                   /* �������� ��������� �� 'x' � 'y' */
	double x,y;                     /* ���������� */
} SLIVER, *LPSLIVER;

/* ��������� ������ � ��������� ������ */
typedef struct {
	int ix,iy;                      /* ������������� ���������� */
	double y;                       /* ���������� 'y' */
	double vy;                      /* �������� �� 'y' */
} ROCKET;

/* �������� ���� ��������� ������� */
typedef struct {
	int up1,up2,hi;
	int addLifes;
} SCORE;

/* ������� ������ ��� ����� */
static const FRAME lpAlienBird[] = {
	{0,3},{1,3},{2,3},{3,3},
	{4,3},{3,3},{2,3},{1,3},
	{-1,0}   /* ����������� ������� */
};

/* ������� ������ ��� ��� */
static const FRAME lpAlienUfo[] = {
	{0,3},{1,3},{2,3},{3,3},{4,3},
	{5,3},{4,3},{3,3},{2,3},{1,3},
	{-1,0}   /* ����������� ������� */
};

/* ������� ������ ��� ������ */
static const FRAME lpBang[] = {
	{0,2},{1,2},{2,2},{3,2},{4,2},
	{3,2},{2,2},{1,2},{0,2},
	{-1,0}   /* ����������� ������� */
};

/* ������� ������ �������� ������� �� ������� � ���������� */
static const FRAME lpNormalToGunRacket[] = {
	{0,1},{3,8},{4,8},{5,8},{6,8},{1,1},
	{-1,0}   /* ����������� ������� */
};

/* ������� ������ �������� ������� �� ���������� � ������� */
static const FRAME lpGunToNormalRacket[] = {
	{1,1},{6,8},{5,8},{4,8},{3,8},{0,1},
	{-1,0}   /* ����������� ������� */
};

/* ������� ������ �������� ������� �� ������� � ������� */
static const FRAME lpNormalToExtendedRacket[] = {
	{0,1},{7,2},{8,2},{9,2},{10,2},
	{11,2},{12,2},{13,2},{2,1},
	{-1,0}   /* ����������� ������� */
};

/* ������� ������ �������� ������� �� ������� � ������� */
static const FRAME lpExtendedToNormalRacket[] = {
	{2,1},{13,2},{12,2},{11,2},{10,2},
	{9,2},{8,2},{7,2},{0,1},
	{-1,0}   /* ����������� ������� */
};

/* ������� ������ ������ ������ */
static const FRAME lpBulletPlop[] = {
	{2,2},{3,2},{4,2},{5,2},
	{-1,0}   /* ����������� ������� */
};

/*
 * ������� ������ �������� ��������
 * 0 - ������� ������� ����
 * 1 - ������� ������� ����
 * 2 - ������� �������� ����
 * 3 - ������� ���������� ����
 * 4 - ������� ������ ����
 */
static const FRAME lpFlySliverAnima[] = {
	{0,1},{0,35},{1,14},{2,7},{3,3},{4,2},{5,55},
	{-1,0}   /* ����������� ������� */
};

/*
 * �������� ������ ����
 * 0 - ������ �����
 * 1 - ������� ���� � ��������� � ������
 * 2 - ������� ����, �������� ������� ������� � ��������� � ������
 */
static const FRAME lpStartRoundAnima[] = {
	{0,5},{1,60},{2,14},
	{-1,0}   /* ����������� ������� */
};

/*
 * �������� ��������� ������, ���������
 * ������� �������� �������.
 */
static const FRAME lpEndRoundAnima[] = {
	{0,40},
	{-1,0}   /* ����������� ������� */
};

/*
 * �������� ���������� �����
 * 0 - ������������ ����� 3
 * 1 - ������������ ����� 2
 * 2 - ������������ ����� 1
 */
static const FRAME lpPauseEndAnima[] = {
	{0,25},{1,25},{2,25},
	{-1,0}   /* ����������� ������� */
};

/*
 * �������� ������ ������ �� ������. ����� ������� ��
 * ������������.
 * 0 - ����� �� ������
 * 1 - ������� �������� �����
 */
static const FRAME lpFlyRocketAnima[] = {
	{0,0},{1,0},
	{-1,0}   /* ����������� ������� */
};

/*
 * 1 - ������ ������
 * 2 - ������� ������
 * 3 - ������� ������
 * 4 - ������� ������
 * 5 - ����� ������
 * 6 - ������ ������
 *
 * ���������� �������� � ������:
 * 0x40 - ���� ����������� ����� ���� ���������
 * 0x80 - ���� �� �����������
 */
/* ����� 1 (15,12) */
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

/* ����� 2 (15,12) */
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

/* ����� 3 (15,12) */
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

/* ����� 4 (15,12) */
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

/* ����� 5 (15,12) */
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

/* ����� 6 (15,12) */
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

/* ����� 7 (15,12) */
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

/* ����� 8 (15,12) */
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

/* ����� 9 (15,12) */
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

/* ����� 10 (15,12) */
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

/* ����� 11 (15,12) */
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

/* ����� 12 (15,12) */
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

/* ����� 13 (15,12) */
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

/* ����� 14 (15,12) */
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

/* ����� 15 (15,12) */
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

/* ��������� ����������� ������� */
typedef struct {
	/*
	 * ���������� ���������� ������ ��� �������
	 * ���������� �������� ��������.
	 *
	 * 0  - ������� �� ����������
	 */
	int AlienBlock;
	/* ��������� ������ ���� ���������� */
	int swer0,swer0X,swer0Y;
	/* ��������� ������ ���� ���������� */
	int swer1,swer1X,swer1Y;
	/* ��������� ������ ���� ���������� */
	int swer2,swer2X,swer2Y;
	/* ��������� ��������� ���� ���������� */
	int swer3,swer3X,swer3Y;
	/* ��������� �� ����������� ���� */
	const UCHAR *round;
} LEVEL;

/* ���������� ������� */
#define LEVEL_COUNT                 15

/* ��������� ����������� ������ */
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

/* ������ ��������� �� ������� ����� */
static const UCHAR *raund;
/* ������ ����� ������ */
static int nLevel;
/*
 * ���� � ��������� ������������ (15,16)
 * 
 * �.�. ������ ��������� � ������� ������������ (15,12),
 * ������� � ������� 'block' ������ 3 � �������� 1 �������
 * ������.
 */
static UCHAR block[16][15];
/* ������ �������� ������� �������� � ������� */
static int EventBlock[16][15];
/* ����� ���������� */
static SWERVE swerve[SWERVES_COUNT];
/* ������ */
static BALL ball[BALLS_COUNT];
/* ������� */
static RACKET racket;
/* �������� �������� */
static ALIEN alien;
/* ����� */
static BANG bang;
/* ����� */
static BOMB bomb;
/* ���� */
static BONUS bonus;
/* ������� �������� */
static ANIMA anima;
/* ������ � ��������� ����� */
static ROCKET rocket;
/* ������� ������� */
static SLIVER sliver[SLIVERS_COUNT];
/* �������� ������ ������ */
static BULLET_PLOP plop[BULLETS_COUNT];
/* �������� ���� ��������� ������� */
static SCORE score;
/* ���������� ���������� ������ */
static int lifes;

/* ����� ����� ������� ���������� �������� ������ */
static int timeAddSpeedBall;
/* ����� ����� ������� ���������� �������� ����� 'BONUS_SMASH_BALL' */
static int timeEndingSmashBall;

/* ��������� ����� */
static struct {
	int deny;               /* ���� ������� ������� � ����� */
	int pause;              /* ���� ����� */
} pause;

/* ���������� ����������� ������������� �������� */
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

/* ���������� ��������� ����� �� ��������� ��������� */
static int random(int range)
{
	return (range * rand() / (RAND_MAX + 1));
}

/* ���������� ����������� ������ ��� ����������� */
static void CopyBlock(void)
{
	int i,j;
	UCHAR *to,*from;
	UCHAR symbol;
	int *event;

	/* ��������� ��� ����� */
	to = &scene.block[0][0];
	from = &block[3][0];
	event = &EventBlock[3][0];
	for (i = 0; i < 12; i++) {
		/* �������� �� ������ */
		for (j = 0; j < 15; j++) {
			int evn = event[j];
			symbol = from[j] & 0x0f;
			/* ���������� ������� ��� ������� ����� */
			if (evn) {
				/* ������� ������ */
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
					/* ������� ������� */
					to[j] = symbol;
					event[j] = 0;
				}
			} else {
				/* ��� �������, �������� ���� */
				to[j] = symbol;
			}
		}
		/* ���������� ��������� */
		to += 15;
		from += 15;
		event += 15;
	}
}

/* ���������� ������ ������ */
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

/* ���������� ������ ������ */
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
 * ���������� ��������� �������� ���� ������.
 *
 * �������� ���������� � �������:
 * 1 - ����� �� ������ �����������.
 * 2 - ���� ���� ������ ��������� �� �� �� ������ ����������.
 * 3 - ����� BONUS_EXTRA_LIFE ����� ��������� ������ ���� ��� �� �����.
 * 3 - ����������� ��������� ������:
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

	/* ��������� ������ ������� */
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

/* ������������ ����������� �������� ������ */
static void CreateBonus(int x, int y)
{
	/* ���� ���� ������������ �� ������ */
	if (bonus.on)
		return;

	/* ����������� ��������� ����� ����� 14.28% */
	if (random(7))
		return;

	/* �������� ���� */
	scene.pBonus.type = GetRandomBonus();
	/* ��������� ��� ��������� */
	bonus.on = 1;
	bonus.ix = x * 16 + LOGIC_X - (GetWidthBonus() - 16) / 2;
	bonus.iy = y * 8 + LOGIC_Y;
	bonus.x = bonus.ix;
	bonus.y = bonus.iy;
	bonus.vy = 0.0;
	/* ��������� ���� */
	scene.pBonus.show = 1;
	scene.pBonus.X = bonus.ix;
	scene.pBonus.Y = bonus.iy;
}

/* ����������� �������� ���� */
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
 * ����������� �������� ���� � ����������� �� ��������� �����.
 * ����� ������� ������� ����� 120, ������� ���� 110, ������� ���
 * ���� 100 � �.�.
 */
static void BlockScore(int x, int y)
{
	const UCHAR *block;
	int point;

	/* ��������� ����� �������� ����� */
	y -= 3;
	point = 10 * (12 - y);
	/* ���� ���� ����������� �� ������� ���� ������ ���� */
	block = raund + y * 15 + x;
	if (*block & 0x40)
		point *= 2;

	AddScore(point);
}

/* ������������ ������������ � ������ */
static int MarkBlock(int x, int y)
{
	UCHAR *point;
	int attr;

	/* ��������� ���� */
	point = &block[y][x];
	if (*point == 0)
		return 0;

	/* ������� �������� ����� */
	attr = *point & 0xf0;
	/* ���� �� ����������� */
	if (attr == 0x80) {
		EventBlock[y][x] = 1;
		play.stone = 1;
		return 0;
	}

	/* ���� ����������� �� ������� ����� */
	if (attr == 0x40) {
		/* �������� �������� ������ 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL)) {
			*point = 0;
			EventBlock[y][x] = 0;
			BlockScore(x,y);
			CreateBonus(x,y);
			play.brik = 1;
		} else {
			/* ���� ����������� �� ������� ����� */
			EventBlock[y][x] = 1;
			*point &= 0x0f;
			play.stone = 1;
		}
	} else {
		/* ������� ���� */
		*point = 0;
		EventBlock[y][x] = 0;
		BlockScore(x,y);
		CreateBonus(x,y);
		play.brik = 1;
	}

	return 1;
}

/* ����������� �������� �� ���� 'x' � 'y' ��� ������ */
static void SetSpeedBall(LPBALL lpBall)
{
	double rad;

	/* ��������� ���� � ������� */
	rad = lpBall->angle * PI / 180.0;
	/* ��������� �������� �� 'x' � 'y' */
	lpBall->vx = lpBall->v * cos(rad);
	lpBall->vy = lpBall->v * sin(rad);
}

/* ���� ����� ����������� �����, �� ������������ ��� ��������� */
static int GetBlock(LPBOUND bound, int px, int py)
{
	int x,y;

	/* ���������� ��������� � �������� ��� */
	x = px - LOGIC_X;
	y = py - LOGIC_Y;
	if ((x < 0) || (y < 0))
		return 0;
	x /= 16;
	y /= 8;
	if ((x >= 15) || (y >= 16))
		return 0;

	/* ��������, ��� ���� ���������� */
	if (!block[y][x])
		return 0;

	/* ���������� ��������� ����������� ��������� ����� */
	bound->x = x;
	bound->y = y;
	bound->offset_x = px - (x * 16 + LOGIC_X);
	bound->offset_y = py - (y * 8 + LOGIC_Y);

	return 1;
}

/* ���������� ������ ������� */
static int GetWidthRacket(void)
{
	int width;

	/* ������������ �� ���� ������� ��������� �� ������ */
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

/* ���������� ������ ������� */
static int GetHeightRacket(void)
{
	/* � ���� ������� ������ ����� 10 */
	return 10;
}

/* ���������� ������ ������ */
static int GetWidthBall(void)
{
	/* ��������� ������ �� ��������� */
	return 7;
}

/* ���������� � ��������� ������� ������� */
static void SetRacket(void)
{
	/* ������� ��������� ������� */
	memset(&racket,0,sizeof(racket));
	memset(&scene.pRacket,0,sizeof(scene.pRacket));

	/* ��������� ��� ������� � �� ��������� */
	racket.on = 1;
	racket.x = RACKET_X;
	racket.y = RACKET_Y;
	/* ��������� ������� */
	scene.pRacket.show = 1;
	scene.pRacket.type = TYPE_RACKET_NORMAL;
	scene.pRacket.X = racket.x;
	scene.pRacket.Y = racket.y;
}

/* ���������� � ��������� ������� ������ */
static void SetBall(void)
{
	/* ������� ��� ����������� � �������� ������� */
	memset(scene.pBall,0,sizeof(scene.pBall));
	memset(ball,0,sizeof(ball));

	/* ��������� ��� � ��������� ������ */
	scene.pBall[0].picture.show = 1;
	scene.pBall[0].picture.type = BALL_NORMAL;
	scene.pBall[0].picture.X = racket.x + 12;
	scene.pBall[0].picture.Y = racket.y - 6;
	/* ��������� ���� �� ������ */
	scene.pBall[0].shadow.show = 1;
	scene.pBall[0].shadow.type = BALL_NORMAL;
	scene.pBall[0].shadow.X = scene.pBall[0].picture.X + GetWidthBall();
	scene.pBall[0].shadow.Y = scene.pBall[0].picture.Y + GetWidthBall();

	/* ��������� ��������� �������� ������ */
	ball[0].on = 1;
	ball[0].count = TIME_NEW_COURSE_BALL;
	ball[0].angle = 293;
	ball[0].v = START_SPEED_BALL;
	SetSpeedBall(&ball[0]);
	ball[0].x = scene.pBall[0].picture.X;
	ball[0].y = scene.pBall[0].picture.Y;

	/* ��������� ������� ���������� �������� */
	timeAddSpeedBall = TIME_ADD_SPEED_BALL;
}

/* ���������� � ��������� ������� ����� ���������� */
static void SetSwerve(void)
{
	int i;
	const LEVEL *lpLevel;

	/* ������� ��� ����������� � �������� ������ */
	memset(scene.pSwerves,0,sizeof(scene.pSwerves));
	memset(swerve,0,sizeof(swerve));

	/* ��������� ����������� ������� */
	lpLevel = &levels[nLevel%LEVEL_COUNT];

	/* �������� ����� ���������� */
	for (i = 0; i < SWERVES_COUNT; i++) {
		LPSWERVE lpSwerve;
		LPPICTURE lpPicture;
		int flags,X,Y;

		/* ��������� ���� �����������, � ���������� */
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

		/* �������� ��������� */
		lpSwerve = &swerve[i];
		lpPicture = &scene.pSwerves[i];

		/* ��������� �������� ��� ����� */
		if (flags) {
			/* ���� ������������ */
			lpPicture->show = 1;
			/* ��������� ��������� */
			if (random(2)) {
				lpPicture->type = PICTURE_SWERVE_ON;
				lpSwerve->on = 1;
			} else {
				lpPicture->type = PICTURE_SWERVE_OFF;
				lpSwerve->on = 0;
			}
			/* ��������� ���������� */
			lpPicture->X = X;
			lpPicture->Y = Y;
			/* �������� ����������� ����� ����� ���������� */
			lpSwerve->x = X + 12;
			lpSwerve->y = Y + 12;
			lpSwerve->count = 30 + random(25);
		}
	}
}

/* ������� �������� ��������, � ��������� �������� ������ */
static void CreateBang(void)
{
	/* ������ �������� �������� */
	alien.on = 0;
	scene.pAlien.show = 0;
	/* ���������� �������� ������ */
	bang.on = 1;
	bang.count = 0;
	bang.point = 0;
	scene.pBang.show = 1;
	scene.pBang.type = lpBang[bang.point].number;
	scene.pBang.X = alien.ix + 4;
	scene.pBang.Y = alien.iy - 3;
}

/* ������� ��� ������� */
static void CleanObject(void)
{
	int i;

	/* ������ ������ */
	for (i = 0; i < BALLS_COUNT; i++) {
		LPFULL_PICTURE lpPicture = &scene.pBall[i];
		lpPicture->picture.show = 0;
		lpPicture->shadow.show = 0;
		ball[i].on = 0;
	}
	/* ������ ������ */
	for (i = 0; i < BULLETS_COUNT; i++)
		scene.pBullet[i].show = 0;
	/* ������ ������� */
	racket.on = 0;
	scene.pRacket.show = 0;
	/* ������ �������� �������� */
	alien.on = 0;
	scene.pAlien.show = 0;
	/* ������ ����� */
	bang.on = 0;
	scene.pBang.show = 0;
	/* ������ ����� */
	bonus.on = 0;
	scene.pBonus.show = 0;
	/* ������ ����� */
	bomb.on = 0;
	scene.pBomb.picture.show = 0;
	scene.pBomb.shadow.show = 0;
}

/* �������� ��������� ������ */
static int EndRoundAnima(void)
{
	/* �������� ������� ��������� */
	if (++anima.count < lpEndRoundAnima[anima.state].count)
		/* �������� ���������� ���� (���� ����) */
		return 2;

	/* ������� ������� �������� */
	anima.func = NULL;
	/* ���������, ��� ����� ������� */
	return 3;
}

/* ��������� �������� ��������� ������ */
static void SetEndRoundAnima(void)
{
	/* ���������� ��������� �������� */
	anima.func = EndRoundAnima;
	anima.count = 0;
	anima.state = 0;
	/* �������� ����� */
	pause.deny = 1;
	/* ������ ��� ������� */
	CleanObject();
}

/* �������� ������ ������ */
static int StartRoundAnima(void)
{
	int state;

	/* �������� ������� ��������� */
	if (++anima.count < lpStartRoundAnima[anima.state].count)
		/* �������� ���������� �� ����������� ������ */
		return 1;

	/* ������� ������� */
	anima.count = 0;

	/* ����������� ��������� �������� */
	state = lpStartRoundAnima[++anima.state].number;
	if (state == 1) {
		/* ��������� ����� */
		scene.on = 1;
		/* ��������� ����� ���������� */
		SetSwerve();
		/* ��������� ��������� */
		scene.message.show = 1;
		scene.message.flags = MESSAGE_ROUND;
		scene.message.player = 1;
		scene.message.round = nLevel + 1;
	} else if (state == 2) {
		UCHAR (*b)[15];
		int (*e)[15];
		int i,j;
		
		/*
		 * ��������� �������� ������ (������������ ������
		 * ������� �� ����������� ��� ����������� �� ������� ����),
		 * ��������� ������� �����.
		 */
		b = &block[3];
		e = &EventBlock[3];
		for (i = 0; i < 12; i++) {
			for (j = 0; j < 15; j++) {
				if ((*b)[j] & (0x80 | 0x40)) {
					(*e)[j] = 1;
					/* �������� ���� �� ������������ ����� */
					play.stone = 1;
				}
			}
			b++; e++;
		}
	} else {
		/* �������� ��������� */
		anima.func = NULL;
		/* ������� ��������� */
		scene.message.show = 0;
		/* ��������� ����� */
		pause.deny = 0;
		/* ��������� ������� */
		SetRacket();
		/* ��������� ������ */
		SetBall();
		/* ������� ������ ����� ����������� � ������� */
		bonus.flags = F_START_RAUND | F_HAND;
		/* ����� ��������� ���������� � ������� */
		ball[0].hand.count = TIME_ENDING_HAND;
		ball[0].hand.offset = 12;
	}

	/* �������� ���������� �� ����������� ������ */
	return 1;
}

/* ��������� �������� ������� ������ */
static void SetStartRoundAnima(void)
{
	/* ���������� ��������� �������� */
	anima.func = StartRoundAnima;
	anima.count = 0;
	anima.state = 0;
	/* �������� ����������� ����� */
	scene.on = 0;
	/* �������� ����� */
	pause.deny = 1;
	/* ������ ��� ������� */
	CleanObject();
	/* ����� ���� ������� ������ */
	memset(EventBlock,0,sizeof(EventBlock));
	/* ������� ��� ����� */
	bonus.flags = 0;
}

/* ��������� �������� ������� �������� */
static int MoveSliver(void)
{
	int i,offset;

	/* �������� ���������� �������� */
	if (++anima.count >= lpFlySliverAnima[anima.state].count) {
		int state;

		/* ������� ������� �������� */
		anima.count = 0;
		/* ���������� �������� �� ��������� ���� */
		state = lpFlySliverAnima[++anima.state].number;
		/* �������� ���������� �������� */
		if (state == 5) {
			/* ������ �� ���������� ������� */
			scene.pSlivers.show = 0;
			for (i = 0; i < SLIVERS_COUNT; i++) {
				LPFULL_PICTURE lpPicture = &scene.pSlivers.picture[i];
				lpPicture->picture.show = 0;
				lpPicture->shadow.show = 0;
			}
			/*
			 * ����� ���������� ������ �� ���������� �.�. �� �����������
			 * �������� ������������� ��������� ����� � ������ ��������
			 * ��������� ������.
			 */
		} else if (state < 0) {
			/* ������ ���� ������� �������� */
			anima.func = NULL;
			/*
			 * ���� ������ �� �������� �� ���������, ��� �����
			 * �������� ����� �������� �������� ������ ������.
			 */
			if (--lifes < 0)
				/* �������� ��������� ������  (����� ��������) */
				return 4;
			else
				SetStartRoundAnima();
			/* �������� ���������� ���� (���� ����) */
			return 2;
		} else {
			/* �������� ��������� ��� ������� */
			for (i = 0; i < SLIVERS_COUNT; i++) {
				LPFULL_PICTURE lpPicture = &scene.pSlivers.picture[i];
				lpPicture->picture.type = state;
				lpPicture->shadow.type = state;
			}
		}
	}

	/* ������� ������ �� ������������ */
	if (lpFlySliverAnima[anima.state].number == 5)
		/* �������� ���������� �� ����������� ������ */
		return 1;

	/* ��������� �������� ���� ������� */
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

	/* ���������� ������� */
	for (i = 0; i < SLIVERS_COUNT; i++) {
		LPFULL_PICTURE lpPicture = &scene.pSlivers.picture[i];
		LPSLIVER lpSliver = &sliver[i];
		int x,y;
		/* ��������� ����� ������� */
		lpSliver->x += lpSliver->vx;
		lpSliver->y += lpSliver->vy;
		/*
		 * ���������� ������������ �� ��������.
		 * ������������ � ������� ������� �� �����������, �.�.
		 * ������� �� ������� ������ �� ��������. ���� ����� ���������
		 * �������� �������� ��������, �� ���� ����� �������� ��������
		 * ������������ � ������� ������� (���������� 'y').
		 */
		if (lpSliver->x < LOGIC_X) {
			lpSliver->vx = -lpSliver->vx;
			lpSliver->x = LOGIC_X;
		} else if (lpSliver->x > LOGIC_X + LOGIC_WIDTH - 7) {
			lpSliver->vx = -lpSliver->vx;
			lpSliver->x = LOGIC_X + LOGIC_WIDTH - 7;
		}
		/* ������� ������������ ���������� */
		x = ftol(lpSliver->x);
		y = ftol(lpSliver->y);
		lpPicture->picture.X = x;
		lpPicture->picture.Y = y;
		lpPicture->shadow.X = x + offset;
		lpPicture->shadow.Y = y + offset;
	}

	/* �������� ���������� �� ����������� ������ */
	return 1;
}

/* ������� �������� ������� �������� ������� */
static int FlySliverAnima(void)
{
	/* �������, ������������ ������� ��� ��� */
	if (!anima.state) {
		/* ��������� ������� �� ���� */
		int i,type,racketWidth;
		double x,y,x0,y0,angle;

		/* �������� ������� ��������� �������� */
		anima.state++;
		/* �������� ��� �������� */
		type = lpFlySliverAnima[anima.state].number;
		/*
		 * ������� �� ���� ����������� ����������,
		 * ��� ����� ������������ ������� ��������.
		 * ��������� ��������� ��������� a = 15, b = -5.
		 */
		/* ��������� ��������� ���������� x,y */
		racketWidth = GetWidthRacket();
		if (racketWidth < 36)
			x = racket.x - ((36 - racketWidth) / 2);
		else if (racketWidth > 36)
			x = racket.x + ((racketWidth - 36) / 2);
		else
			x = racket.x;
		y = RACKET_Y;
		/* ��������� ��������� ���������� x0,y0 */
		x0 = x + 15.0;
		y0 = y + 5.0;
		/* ��������� ���� */
		angle = 180.0;
		/* ��������� ��������� �������� */
		for (i = 0; i < SLIVERS_COUNT; i++) {
			LPFULL_PICTURE lpPicture;
			LPSLIVER lpSliver;
			double rad;
			int lx,ly;
			/* ����������� ���������� x */
			x += 3.0;
			/* ������� ���������� 'y' �� ������� �������� */
			y = -5.0 * sqrt(fabs(1.0 - pow((x - x0) / 15.0,2.0))) + y0;
			/* �������� ������������� ���������� */
			lx = ftol(x);
			/* ������������ ���������� x */
			if (lx < LOGIC_X)
				lx = LOGIC_X;
			else if (lx > LOGIC_X + LOGIC_WIDTH - 7)
				lx = LOGIC_X + LOGIC_WIDTH - 7;
			ly = ftol(y);
			/* �������� ���������� ��� ������ */
			lpSliver = &sliver[i];
			lpSliver->x = lx;
			lpSliver->y = ly;
			/* ����������� �������� (vx,vy) ��� ������ */
			rad = angle * PI / 180.0;
			lpSliver->vx = SPEED_SLIVER * cos(rad);
			lpSliver->vy = SPEED_SLIVER * sin(rad);
			/* �������� ���� */
			angle += 22.5;
			/* �������� ���������� ��� ����������� �������� */
			lpPicture = &scene.pSlivers.picture[i];
			lpPicture->picture.show = 1;
			lpPicture->picture.type = type;
			lpPicture->picture.X = lx;
			lpPicture->picture.Y = ly;
			/* �������� ���������� ��� ����������� ���� �� �������� */
			lpPicture->shadow.show = 1;
			lpPicture->shadow.type = type;
			lpPicture->shadow.X = lx + 6;
			lpPicture->shadow.Y = ly + 6;
		}
		/* ������� ������������ */
		scene.pSlivers.show = 1;
	} else {
		/* ���������� ������� */
		return MoveSliver();
	}

	/* �������� ���������� �� ����������� ������ */
	return 1;
}

/* �������� �������� ������� �������� ������� */
static void SetFlySliverAnima(void)
{
	/* ���������� �������� ������� �������� ������� */
	anima.func = FlySliverAnima;
	anima.count = 0;
	anima.state = 0;
	/* �������� ����� */
	pause.deny = 1;
	/* ������ ��� ������� */
	CleanObject();
	/* ������� ������� */
	play.destroy = 1;
}

/* �������� ��������� ����� */
static int PauseEndAnima(void)
{
	int state;

	/* �������� ������� ��������� */
	if (++anima.count < lpPauseEndAnima[anima.state].count)
		/* �������� ���������� ���� (���� ����) */
		return 2;

	/* ������� ������� */
	anima.count = 0;

	/* ����������� ��������� �������� */
	state = lpPauseEndAnima[++anima.state].number;
	/* �������� ����� ���������� ����� */
	if (state == 1) {
		scene.message.pause = 2;
	} else if (state == 2) {
		scene.message.pause = 1;
	} else {
		/* �������� ����������� */
		anima.func = NULL;
		/* ������ ��������� */
		scene.message.show = 0;
		/* ��������� ����� */
		pause.deny = 0;
	}

	/* �������� ���������� ���� (���� ����) */
	return 2;
}

/* ��������� �������� ��������� ����� */
static void SetPauseEndAnima(void)
{
	/* ���������� ��������� �������� */
	anima.func = PauseEndAnima;
	anima.count = 0;
	anima.state = 0;
	/* �������� ����� */
	pause.deny = 1;
	/* ��������� ��������� ����� */
	scene.message.show = 1;
	scene.message.flags = MESSAGE_PAUSE;
	scene.message.pause = 3;
}

/* ������� �������� ������ �� ������ � ��������� ������ */
static int FlyRocketAnima(void)
{
	static int count = 0;
	UCHAR *point;
	int state;
	div_t n;

	/* ��������� ��������� �������� */
	state = lpFlyRocketAnima[anima.state].number;

	/*
	 * ��������� ��� ������, ���������� ������
	 * ��� ������������ �������� ����.
	 */
	if (!state) {
		/* ���������� ������ */
		rocket.y += rocket.vy;
		rocket.iy = ftol(rocket.y);
		/* ������� ��������� */
		rocket.vy -= ACCELERATION_Y;

		/* ������� ������ ��� ����������� */
		scene.pRocket.Y = rocket.iy;
		if (scene.pRocket.type == TYPE_ROCKET_0)
			scene.pRocket.type = TYPE_ROCKET_1;
		else
			scene.pRocket.type = TYPE_ROCKET_0;

		/*
		 * �������� ���������� ������ ������.
		 * 120 - ��� �� ���������� ������, �� ������� ������
		 * ����� �� ������� �������� ����.
		 */
		if (++count >= 120) {
			anima.state++;
			count = 0;
		}
	} else if (state == 1) {
		/* ��������� ����������� ������ � ������� �������� */
		n = div(count,15);
		n.quot += 3;
		point = &block[n.quot][n.rem];
		/* ���� ���� ���� � �� �����������, �� ������� �������� ���� */
		if (*point && !(*point & 0x80))
			BlockScore(n.rem,n.quot);

		/* �������� ��������� ���� */
		count++;
		n = div(count,15);
		n.quot += 3;
		point = &block[n.quot][n.rem];
		if (*point && !(*point & 0x80))
			BlockScore(n.rem,n.quot);

		/* ������ ������� � ������� ����� ������ (15,12) */
		if (++count >= 15 * 12) {
			anima.state++;
			count = 0;
		}
	} else {
		/* ������ ������� �������� */
		anima.func = NULL;
		/* �������� �������� ��������� ������ */
		SetEndRoundAnima();
	}

	/* �������� ���������� �� ����������� ������ */
	return 1;
}

/* ������ �������� ������ ������ � ��������� ������ */
static void SetFlyRocketAnima(void)
{
	/* �������������� �������� */
	anima.func = FlyRocketAnima;
	anima.count = 0;
	anima.state = 0;
	/* �������� ����� */
	pause.deny = 1;
	/* ���������� ��������� ������ */
	rocket.ix = scene.pRacket.X;
	rocket.iy = scene.pRacket.Y;
	rocket.y = rocket.iy;
	rocket.vy = 0.0;
	/* ��������� ������ � ��������� ����� */
	scene.pRocket.show = 1;
	scene.pRocket.type = TYPE_ROCKET_0;
	scene.pRocket.X = scene.pRacket.X;
	scene.pRocket.Y = scene.pRacket.Y;
	/* ������ ��� ������� */
	CleanObject();
}

/* ��������� ����������� ������� */
static void CheckControlKey(void)
{
	/* ���������� ������ */
	if (input.P) {
		if (!pause.deny) {
			if (!pause.pause) {
				scene.message.show = 1;
				scene.message.flags = MESSAGE_PAUSE;
				scene.message.pause = 0;
				pause.pause = 1;
			} else {
				/* ������� ����� */
				pause.pause = 0;
				/* �������� �������� ��������� ����� */
				SetPauseEndAnima();
			}
		}
	}
}

/* ��������� ��������� ���� �������� ������� */
static void NextFrameRacket(void)
{
	int point,curentWidth,oldWidth;

	/* �������� ������� ������ ������ */
	point = racket.animation.point;
	if (++racket.animation.count < racket.animation.frames[point].count)
		return;

	/* ����������� ���� */
	point++;
	/* �������� ���������� �������� */
	if (racket.animation.frames[point].number < 0) {
		/*
		 * ���� ���������� �������������� ����� ��� �����������,
		 * �� �������� ���. ��� ����� ����� �������� ������� ������� �
		 * ������ ���� "���������� �������" �� � �������� �����������
		 * ����� "������� �������" -> "������� �������" � � ���������
		 * ����� ����������� ����� "������� �������" -> "���������� �������".
		 */
		if (racket.frames) {
			racket.animation.frames = racket.frames;
			racket.animation.point = 0;
			racket.animation.count = 0;
			/* ������� ��������� ����� ������ */
			racket.frames = NULL;
		} else
			racket.animation.frames = NULL;

		return;
	}

	/* ������� ������� ������ ������� */
	oldWidth = GetWidthRacket();
	/* �������� ����� ��� ������� */
	scene.pRacket.type = racket.animation.frames[point].number;
	/* ������� ����� ������ ������� */
	curentWidth = GetWidthRacket();
	/* ������������ ��������� ������� */
	racket.x = racket.x + oldWidth / 2 - curentWidth / 2;
	if (racket.x < LOGIC_X)
		racket.x = LOGIC_X;
	else if (racket.x > (LOGIC_X + LOGIC_WIDTH - curentWidth))
		racket.x = LOGIC_X + LOGIC_WIDTH - curentWidth;

	/* ���������� ����������� ���������� */
	racket.animation.point = point;
	racket.animation.count = 0;
}

/* ������� ������ */
static int CreateBullet(LPPICTURE lpBullet)
{
	int i,flags;

	/* ���� ��� ������ ��� ������������ �� ������ */
	if (lpBullet->show)
		return 0;

	/* ��������� ������� ������ */
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

	/* ������ ��������� ������ */
	if (flags)
		return 0;

	/* ������� ������ */
	lpBullet->show = 1;
	lpBullet->type = TYPE_BULLET_0;
	lpBullet->X = racket.x + GetWidthRacket() / 2 - 2;
	lpBullet->Y = RACKET_Y - 8;

	return 1;
}

/* ���������� ������� */
static void MoveRacket(void)
{
	int width;

	/* ��������, ������������ ������� ��� ��� */
	if (!racket.on)
		return;

	/* ���� ���� �������� �� ��������� �� */
	if (racket.animation.frames != NULL)
		NextFrameRacket();

	/* ������� ������ ������� */
	width = GetWidthRacket();

	/* �������� ���������� � ���������� */
	if (input.K) {
		/* ���������� ������� � ����� ������� */
		racket.x -= 4;
		if (racket.x < LOGIC_X)
			racket.x = LOGIC_X;
	}
	if (input.L) {
		/* ���������� ������� � ������ ������� */
		racket.x += 4;
		if (racket.x > (LOGIC_X + LOGIC_WIDTH - width))
			racket.x = LOGIC_X + LOGIC_WIDTH - width;
	}

	/* �������� ���������� � ���� */
	if (input.x) {
		/* ���������� ������� */
		racket.x += ftol(input.x * RACKET_FACTOR);
		if (racket.x < LOGIC_X)
			racket.x = LOGIC_X;
		else if (racket.x > (LOGIC_X + LOGIC_WIDTH - width))
			racket.x = LOGIC_X + LOGIC_WIDTH - width;
	}

	/* ��������� ���� 'BONUS_GUN' */
	if (CHECK_FLAG(bonus.flags,F_GUN)) {
		/* �������� ��� ������� �� �������� ���� ��� */
		if (racket.animation.frames == NULL) {
			/* �������� ����������� ������� */
			if (input.lButton || input.SPACE) {
				int i;
				/* �������� ������ */
				for (i = 0; i < BULLETS_COUNT; i++) {
					if (CreateBullet(&scene.pBullet[i]))
						break;
				}
			}
		}
	}

	/* ���������� ����������� */
	scene.pRacket.X = racket.x;
}

/* ������ ������ ������ ���������� */
static int LockSwerve(LPSWERVE lpSwerve, LPBALL lpBall)
{
	int rBall,xBall,yBall;
	int flags = 0;

	/* ��������� ����������� ����� ������ */
	rBall = GetWidthBall() / 2;
	xBall = lpBall->ix + rBall;
	yBall = lpBall->iy + rBall;
	/* � ����������� �� ���� �������� ������ ������� */
	switch (lpBall->angle) {
		/* ��� ������������ ����� �������� */
		case 293:
			/* ��������, ��� ����� ������� ����������� ����� */
			if (yBall <= lpSwerve->y + 1) {
				/* ��������� ���� ��������� */
				flags = 1;
				/*
				 * � ����������� �� ����� �����������
				 * ��������� �������������� ����.
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

		/* ��� ������������ ����� �������� */
		case 203:
			/* ��������, ��� ����� ������� ����������� ����� */
			if (xBall <= lpSwerve->x + 1) {
				/* ��������� ���� ��������� */
				flags = 1;
				/*
				 * � ����������� �� ����� �����������
				 * ��������� �������������� ����.
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

		/* ��� ������ ����� �������� */
		case 315:
			/* ��������, ��� ����� ����� � ���� �������� ������� */
			if ((xBall >= lpSwerve->x - 7) && (yBall <= lpSwerve->y + 7)) {
				/* ��������� ���� ��������� */
				flags = 1;
				/*
				 * � ����������� �� ����� �����������
				 * ��������� �������������� ����.
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

	/* ���� ����� ��� ��������� */
	if (flags) {
		/* ���������� ���� ������� � ��������� ����� */
		lpBall->swerve.lock = 1;
		/* �������� �������� */
		SetSpeedBall(lpBall);
	}

	return flags;
}

/* ������������ ������ ������ ���������� */
static void UnlockSwerve(LPBALL lpBall)
{
	/* ����������� ����� */
	if (lpBall->swerve.lock) {
		lpBall->swerve.lock = 0;
		/* ������������� ���� � �������� ��������� ����� */
		lpBall->angle = lpBall->swerve.angle;
		/* ��������� �������� */
		SetSpeedBall(lpBall);
	}
}

/* ��������� ����������� � "������ ����������" */
static int CheckCollideSwerve(LPBALL lpBall)
{
	int i,rSum,rBall,flags = 0;

	/* ��������� ������� ����� �������� "����� ����������" � ������ */
	rBall = GetWidthBall() / 2;
	rSum = SWERVE_RADIUS + rBall;
	rSum *= rSum;

	/* �������� ����������� � ������ ���������� */
	for (i = 0; i < SWERVES_COUNT; i++) {
		LPSWERVE lpSwerve = &swerve[i];
		/* ������� ���� ���� ������� */
		if (lpSwerve->on) {
			int dx = (lpBall->ix + rBall) - lpSwerve->x;
			int dy = (lpBall->iy + rBall) - lpSwerve->y;
			/* ��������, ��� ����� ��������� � ���� ����� */
			if ((dx * dx + dy * dy) <= rSum) {
				/* ���� ����� ����� ��������, �� ����������� ��� */
				if (!lpBall->swerve.lock) {
					/* ���� ����� ��� ��������� �� ���������� ��� */
					if (LockSwerve(lpSwerve,lpBall)) {
						lpBall->swerve.num = i;
						flags = 1;
					}
				}
			} else {
				/* ���� ����� ��� �������� ���� ������, �� ��������� ��� */
				if (lpBall->swerve.lock && (lpBall->swerve.num == i)) {
					UnlockSwerve(lpBall);
					flags = 1;
				}
			}
		} else {
			/* ���� ����� ��� ��������, ��������� ��� */
			if (lpBall->swerve.lock && (lpBall->swerve.num == i)) {
				UnlockSwerve(lpBall);
				/* ����� ��� ��������� */
				flags = 1;
			}
		}
	}

	return flags;
}

/* ������������� ���������� 'X' ������ */
static void UpdateBallX(LPBALL lpBall, int displace)
{
	lpBall->x -= (lpBall->x - displace) * 2;
	lpBall->ix = ftol(lpBall->x);
}

/* ������������� ���������� 'Y' ������ */
static void UpdateBallY(LPBALL lpBall, int displace)
{
	lpBall->y -= (lpBall->y - displace) * 2;
	lpBall->iy = ftol(lpBall->y);
}

/* ��������� ������������ �� ������� */
static int CheckCollideBorder(LPBALL lpBall)
{
	int wBall,flags = 0;

	/* ������� ������ ������ */
	wBall = GetWidthBall();
	/* �������� ������������ � �������� ��������, ��������� 'y' */
	if ((lpBall->ix <= LOGIC_X) &&
		(lpBall->angle > 90) && (lpBall->angle < 270)) {
		flags = 1;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ��������� ��������� ���� � ��������� 'y' */
		lpBall->angle = 180 - lpBall->angle;
		if (lpBall->angle < 0) lpBall->angle += 360;
		/* ��������� ����� �������� */
		SetSpeedBall(lpBall);
		/* ������������ ���������� 'x' */
		UpdateBallX(lpBall,LOGIC_X);
		/* ������� ������� */
		play.wall = 1;
	} else if ((lpBall->ix >= (LOGIC_X + LOGIC_WIDTH - wBall)) &&
		((lpBall->angle > 270) || (lpBall->angle < 90))) {
		flags = 1;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ��������� ��������� ���� � ��������� 'y' */
		lpBall->angle = 180 - lpBall->angle;
		if (lpBall->angle < 0) lpBall->angle += 360;
		/* ��������� ����� �������� */
		SetSpeedBall(lpBall);
		/* ������������ ���������� 'x' */
		UpdateBallX(lpBall,LOGIC_X+LOGIC_WIDTH-wBall);
		/* ������� ������� */
		play.wall = 1;
	}

	/* �������� ������������ � ������� �������, ��������� 'x' */
	if ((lpBall->iy <= LOGIC_Y) && (lpBall->angle > 180)) {
		flags = 1;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ��������� ��������� ���� � ��������� 'x' */
		lpBall->angle = 360 - lpBall->angle;
		/* ��������� ����� �������� */
		SetSpeedBall(lpBall);
		/* ������������ ���������� 'y' */
		UpdateBallY(lpBall,LOGIC_Y);
		/* ������� ������� */
		play.wall = 1;
	} else if (lpBall->iy > (LOGIC_Y + LOGIC_HEIGHT)) {
		/* ����� ������ �� ������ ����� */
		int i, count;
		/* ������� ������ ����� */
		lpBall->on = 0;
		/* ���������� ���������� ���������� ������� */
		for (i = count = 0; i < BALLS_COUNT; i++) {
			if (ball[i].on)
				count++;
		}
		/* �������� ���������� ���������� ������� */
		if (count == 0)
			/* ��� �������, ��������� ������� */
			SetFlySliverAnima();
		else if (count == 1)
			/* ���� �����, ������� ���� "��� ������" */
			bonus.flags &= ~F_TRIPLE_BALL;
	}

	return flags;
}

/* ��������� ������������ � �������� */
static int CheckCollideRacket(LPBALL lpBall)
{
	int b1,b2,r1,r2,r3,r4,r5,r6;
	int wRacket,wBall,temp;

	/* ��������, ��� �������� �������� �� 'y' ������������ */
	if (lpBall->angle > 180)
		return 0;
	/* ������� ������ ������ */
	wBall = GetWidthBall();
	/* ��������, ��� ����� ��������� �� ������ ������� */
	if ((lpBall->iy < (RACKET_Y - wBall)) || (lpBall->iy > RACKET_Y))
		return 0;
	/* ������� ������ ������� */
	wRacket = GetWidthRacket();
	/* ��������, ��� ����� ������������ � �������� */
	if (((lpBall->ix + wBall) <= racket.x) ||
		(lpBall->ix >= (racket.x + wRacket)))
		return 0;

	/* ��������� ���������� ����������� ����� ������� */
	temp = wRacket / 2;
	r3 = temp - 1;
	r4 = wRacket - (r3 + 1);
	r2 = temp / 2;
	r5 = wRacket - (r2 + 1);
	r1 = r2 / 2 + 1;
	r6 = wRacket - (r1 + 1);
	/* ������� �������� ������� */
	r1 += racket.x;
	r2 += racket.x;
	r3 += racket.x;
	r4 += racket.x;
	r5 += racket.x;
	r6 += racket.x;

	/* ��������� ���������� ����� ������ */
	b1 = lpBall->ix + 2;
	b2 = lpBall->ix + 4;

	/*
	 * �������� ��� �� ��������� ���� 'BONUS_HAND' ���
	 * ������� �������� ���� ���.
	 */
	if (!CHECK_FLAG(bonus.flags,F_HAND) ||
		(racket.animation.frames != NULL)) {
		/* ������� ����������� �������� �� ��������� 'x'.*/
		if (lpBall->angle < 90) {
			/* �������� �������� �� 'x' �������������. */
			/*
			 * ��������� ���� ��������� ������������ �� 
			 * ����� ��������������� � ��������.
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
			/* �������� �������� �� 'x' �������������. */
			/*
			 * ��������� ���� ��������� ������������ �� 
			 * ����� ��������������� � ��������.
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
		/* ��������� ���� 'BOHUS_HAND' */
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
		/* ������� ������� */
		lpBall->hand.count = TIME_ENDING_HAND;
	}

	/* ��������� ����� �������� */
	SetSpeedBall(lpBall);

	/* ������� ������� */
	play.wall = 1;

	return 1;
}

/* �������� ������������ � ������� ��� ����� ����� 90 �������� */
static int CheckBlock90(LPBALL lpBall)
{
	BOUND bound1,bound2,bound3,bound4,bound5;
	int b1,b2,b3,b4,b5,wBall;

	/* ������� ������ ������ */
	wBall = GetWidthBall();
	/* ������� ����� ���������� ����������� ����� */
	b1 = GetBlock(&bound1,lpBall->ix,lpBall->iy+wBall);          /* ������-����� */
	b2 = GetBlock(&bound2,lpBall->ix+wBall-1,lpBall->iy+wBall);  /* ������-������ */
	b3 = GetBlock(&bound3,lpBall->ix+wBall,lpBall->iy);          /* ������-������� */
	b4 = GetBlock(&bound4,lpBall->ix+wBall,lpBall->iy+wBall-1);  /* ������-������ */
	b5 = GetBlock(&bound5,lpBall->ix+wBall,lpBall->iy+wBall);    /* ������� ������-������ */

	/* �������� ��� ������� � ������ ���������� */
	if ((b1 && b3) && (bound1.x != bound3.x) &&
		(bound1.y != bound3.y)) {
		int flags = 0;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ������ ����� */
		if (MarkBlock(bound1.x,bound1.y))
			flags = 1;
		if (MarkBlock(bound3.x,bound3.y))
			flags = 1;
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = lpBall->angle + 180;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallX(lpBall,bound3.x * 16 + LOGIC_X - wBall);
		UpdateBallY(lpBall,bound1.y * 8 + LOGIC_Y - wBall);

		return flags;
	}

	/* �������� ������ ��������� */
	if ((b1 && b2) && (bound1.x != bound2.x)) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ���������, �� ����� ���� ����� ������������� ������ */
		if ((16 - bound1.offset_x) > (bound2.offset_x + 1)) {
			flags = MarkBlock(bound1.x,bound1.y);
		} else {
			flags = MarkBlock(bound2.x,bound2.y);
		}
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������� ���������� */
		UpdateBallY(lpBall,bound1.y * 8 + LOGIC_Y - wBall);

		return flags;
	}

	/* �������� ������ ��������� */
	if ((b3 && b4) && (bound3.y != bound4.y)) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ���������, �� ����� ���� ����� ������������� ������ */
		if ((8 - bound3.offset_y) > (bound4.offset_y + 1)) {
			flags = MarkBlock(bound3.x,bound3.y);
		} else {
			flags = MarkBlock(bound4.x,bound4.y);
		}
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallX(lpBall,bound3.x * 16 + LOGIC_X - wBall);

		return flags;
	}

	/* �������� ������ ����� ���� */
	if (b1) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ���� */
		flags = MarkBlock(bound1.x,bound1.y);
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallY(lpBall,bound1.y * 8 + LOGIC_Y - wBall);

		return flags;
	}

	/* �������� ������ ������� ���� */
	if (b3) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ���� */
		flags = MarkBlock(bound3.x,bound3.y);
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallX(lpBall,bound3.x * 16 + LOGIC_X - wBall);

		return flags;
	}

	/* �������� ������� ���� */
	if (b5) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ���� */
		flags = MarkBlock(bound5.x,bound5.y);
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������������ ���������� � ���� */
		if ((bound5.offset_x + 1) > (bound5.offset_y + 1)) {
			lpBall->angle = 360 - lpBall->angle;
			UpdateBallY(lpBall,bound5.y * 8 + LOGIC_Y - wBall);
		} else if ((bound5.offset_x + 1) < (bound5.offset_y + 1)) {
			lpBall->angle = 180 - lpBall->angle;
			UpdateBallX(lpBall,bound5.x * 16 + LOGIC_X - wBall);
		} else {
			/* ��� ����� ��� �������������� ������������ �� ���� */
			if (lpBall->angle < 45) {
				lpBall->angle = 180 - lpBall->angle;
				UpdateBallX(lpBall,bound5.x * 16 + LOGIC_X - wBall);
			} else {
				lpBall->angle = 360 - lpBall->angle;
				UpdateBallY(lpBall,bound5.y * 8 + LOGIC_Y - wBall);
			}
		}
		/* ��������� �������� */
		SetSpeedBall(lpBall);

		return flags;
	}

	return 0;
}

static int CheckBlock180(LPBALL lpBall)
{
	BOUND bound1,bound2,bound3,bound4,bound5;
	int b1,b2,b3,b4,b5,wBall;

	/* ������� ������ ������ */
	wBall = GetWidthBall();
	/* ������� ����� ���������� ����������� ����� */
	b1 = GetBlock(&bound1,lpBall->ix+wBall-1,lpBall->iy+wBall);  /* ������-������ */
	b2 = GetBlock(&bound2,lpBall->ix,lpBall->iy+wBall);          /* ������-����� */
	b3 = GetBlock(&bound3,lpBall->ix-1,lpBall->iy);              /* �����-������� */
	b4 = GetBlock(&bound4,lpBall->ix-1,lpBall->iy+wBall-1);      /* �����-������ */
	b5 = GetBlock(&bound5,lpBall->ix-1,lpBall->iy+wBall);        /* ������� �����-������ */

	/* �������� ��� ������� � ������ ���������� */
	if ((b1 && b3) && (bound1.x != bound3.x) &&
		(bound1.y != bound3.y)) {
		int flags = 0;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ������ ����� */
		if (MarkBlock(bound1.x,bound1.y))
			flags = 1;
		if (MarkBlock(bound3.x,bound3.y))
			flags = 1;
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = lpBall->angle + 180;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallX(lpBall,(bound3.x + 1) * 16 + LOGIC_X);
		UpdateBallY(lpBall,bound1.y * 8 + LOGIC_Y - wBall);

		return flags;
	}

	/* �������� ������ ��������� */
	if ((b1 && b2) && (bound1.x != bound2.x)) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ��������� �� ����� ���� ����� ������������� ������ */
		if ((bound1.offset_x + 1) > (16 - bound2.offset_x)) {
			flags = MarkBlock(bound1.x,bound1.y);
		} else {
			flags = MarkBlock(bound2.x,bound2.y);
		}
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallY(lpBall,bound1.y * 8 + LOGIC_Y - wBall);

		return flags;
	}

	/* �������� ����� ��������� */
	if ((b3 && b4) && (bound3.y != bound4.y)) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ��������� �� ����� ���� ����� ������������� ������ */
		if ((8 - bound3.offset_y) > (bound4.offset_y + 1)) {
			flags = MarkBlock(bound3.x,bound3.y);
		} else {
			flags = MarkBlock(bound4.x,bound4.y);
		}
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallX(lpBall,(bound3.x + 1) * 16 + LOGIC_X);

		return flags;
	}

	/* �������� ������ ������ ���� */
	if (b1) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ���� */
		flags = MarkBlock(bound1.x,bound1.y);
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallY(lpBall,bound1.y * 8 + LOGIC_Y - wBall);

		return flags;
	}

	/* �������� ����� ������� ���� */
	if (b3) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ���� */
		flags = MarkBlock(bound3.x,bound3.y);
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallX(lpBall,(bound3.x + 1) * 16 + LOGIC_X);

		return flags;
	}

	/* �������� ������� ���� */
	if (b5) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ���� */
		flags = MarkBlock(bound5.x,bound5.y);
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������������ ���������� � ���� */
		if ((16 - bound5.offset_x) > (bound5.offset_y + 1)) {
			lpBall->angle = 360 - lpBall->angle;
			UpdateBallY(lpBall,bound5.y * 8 + LOGIC_Y - wBall);
		} else if ((16 - bound5.offset_x) < (bound5.offset_y + 1)) {
			lpBall->angle = 180 - lpBall->angle;
			UpdateBallX(lpBall,(bound5.x + 1) * 16 + LOGIC_X);
		} else {
			/* ��� ����� ��� �������������� ������������ �� ���� */
			if (lpBall->angle > 135) {
				lpBall->angle = 180 - lpBall->angle;
				UpdateBallX(lpBall,(bound5.x + 1) * 16 + LOGIC_X);
			} else {
				lpBall->angle = 360 - lpBall->angle;
				UpdateBallY(lpBall,bound5.y * 8 + LOGIC_Y - wBall);
			}
		}
		/* ��������� �������� */
		SetSpeedBall(lpBall);

		return flags;
	}

	return 0;
}

static int CheckBlock270(LPBALL lpBall)
{
	BOUND bound1,bound2,bound3,bound4,bound5;
	int b1,b2,b3,b4,b5,wBall;

	/* ������� ������ ������ */
	wBall = GetWidthBall();
	/* ������� ����� ���������� ����������� ����� */
	b1 = GetBlock(&bound1,lpBall->ix+wBall-1,lpBall->iy-1); /* �������-������ */
	b2 = GetBlock(&bound2,lpBall->ix,lpBall->iy-1);         /* �������-����� */
	b3 = GetBlock(&bound3,lpBall->ix-1,lpBall->iy+wBall-1); /* �����-������ */
	b4 = GetBlock(&bound4,lpBall->ix-1,lpBall->iy);         /* �����-������� */
	b5 = GetBlock(&bound5,lpBall->ix-1,lpBall->iy-1);       /* ������� �������-����� */

	/* �������� ��� ������� � ������ ���������� */
	if ((b1 && b3) && (bound1.x != bound3.x) &&
		(bound1.y != bound3.y)) {
		int flags = 0;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ����� */
		if (MarkBlock(bound1.x,bound1.y))
			flags = 1;
		if (MarkBlock(bound3.x,bound3.y))
			flags = 1;
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = lpBall->angle - 180;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallX(lpBall,(bound3.x + 1) * 16 + LOGIC_X);
		UpdateBallY(lpBall,(bound1.y + 1) * 8 + LOGIC_Y);

		return flags;
	}

	/* �������� ������� ��������� */
	if ((b1 && b2) && (bound1.x != bound2.x)) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ��������� �� ����� ���� ����� ������������� ������ */
		if ((bound1.offset_x + 1) > (16 - bound2.offset_x)) {
			flags = MarkBlock(bound1.x,bound1.y);
		} else {
			flags = MarkBlock(bound2.x,bound2.y);
		}
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallY(lpBall,(bound1.y + 1) * 8 + LOGIC_Y);

		return flags;
	}

	/* �������� ����� ��������� */
	if ((b3 && b4) && (bound3.y != bound4.y)) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ��������� �� ����� ���� ����� ������������� ������ */
		if ((bound3.offset_y + 1) > (8 - bound4.offset_y)) {
			flags = MarkBlock(bound3.x,bound3.y);
		} else {
			flags = MarkBlock(bound4.x,bound4.y);
		}
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 360 + 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallX(lpBall,(bound3.x + 1) * 16 + LOGIC_X);

		return flags;
	}

	/* �������� �������-������ ���� */
	if (b1) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ���� */
		flags = MarkBlock(bound1.x,bound1.y);
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallY(lpBall,(bound1.y + 1) * 8 + LOGIC_Y);

		return flags;
	}

	/* �������� �����-������ ���� */
	if (b3) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ���� */
		flags = MarkBlock(bound3.x,bound3.y);
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 360 + 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallX(lpBall,(bound3.x + 1) * 16 + LOGIC_X);

		return flags;
	}

	/* �������� ������� ���� */
	if (b5) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ���� */
		flags = MarkBlock(bound5.x,bound5.y);
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������������ ���������� � ���� */
		if ((16 - bound5.offset_x) > (8 - bound5.offset_y)) {
			lpBall->angle = 360 - lpBall->angle;
			UpdateBallY(lpBall,(bound5.y + 1) * 8 + LOGIC_Y);
		} else if ((16 - bound5.offset_x) < (8 - bound5.offset_y)) {
			lpBall->angle = 360 + 180 - lpBall->angle;
			UpdateBallX(lpBall,(bound5.x + 1) * 16 + LOGIC_X);
		} else {
			/* ��� ����� ��� �������������� ������������ �� ���� */
			if (lpBall->angle < 225) {
				lpBall->angle = 360 + 180 - lpBall->angle;
				UpdateBallX(lpBall,(bound5.x + 1) * 16 + LOGIC_X);
			} else {
				lpBall->angle = 360 - lpBall->angle;
				UpdateBallY(lpBall,(bound5.y + 1) * 8 + LOGIC_Y);
			}
		}
		/* ��������� �������� */
		SetSpeedBall(lpBall);

		return flags;
	}

	return 0;
}

static int CheckBlock360(LPBALL lpBall)
{
	BOUND bound1,bound2,bound3,bound4,bound5;
	int b1,b2,b3,b4,b5,wBall;

	/* ������� ������ ������ */
	wBall = GetWidthBall();
	/* ������� ����� ���������� ����������� ����� */
	b1 = GetBlock(&bound1,lpBall->ix,lpBall->iy-1);             /* �������-����� */
	b2 = GetBlock(&bound2,lpBall->ix+wBall-1,lpBall->iy-1);     /* �������-������ */
	b3 = GetBlock(&bound3,lpBall->ix+wBall,lpBall->iy+wBall-1); /* ������-������ */
	b4 = GetBlock(&bound4,lpBall->ix+wBall,lpBall->iy);         /* ������-������� */
	b5 = GetBlock(&bound5,lpBall->ix+wBall,lpBall->iy-1);       /* ������ �������-������ */

	/* �������� ��� ������� � ������ ���������� */
	if ((b1 && b3) && (bound1.x != bound3.x) &&
		(bound1.y != bound3.y)) {
		int flags = 0;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ����� */
		if (MarkBlock(bound1.x,bound1.y))
			flags = 1;
		if (MarkBlock(bound3.x,bound3.y))
			flags = 1;
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = lpBall->angle - 180;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallX(lpBall,bound3.x * 16 + LOGIC_X - wBall);
		UpdateBallY(lpBall,(bound1.y + 1) * 8 + LOGIC_Y);

		return flags;
	}

	/* ������� ������� ��������� */
	if ((b1 && b2) && (bound1.x != bound2.x)) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ��������� �� ����� ���� ����� ������������� ������ */
		if ((16 - bound1.offset_x) > (bound2.offset_x + 1)) {
			flags = MarkBlock(bound1.x,bound1.y);
		} else {
			flags = MarkBlock(bound2.x,bound2.y);
		}
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallY(lpBall,(bound1.y + 1) * 8 + LOGIC_Y);

		return flags;
	}

	/* �������� ������ ��������� */
	if ((b3 && b4) && (bound3.y != bound4.y)) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ��������� �� ����� ���� ����� ������������� ������ */
		if ((bound3.offset_y + 1) > (8 - bound4.offset_y)) {
			flags = MarkBlock(bound3.x,bound3.y);
		} else {
			flags = MarkBlock(bound4.x,bound4.y);
		}
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 360 + 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallX(lpBall,bound3.x * 16 + LOGIC_X - wBall);

		return flags;
	}

	/* �������� �������-����� ���� */
	if (b1) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ���� */
		flags = MarkBlock(bound1.x,bound1.y);
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 360 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallY(lpBall,(bound1.y + 1) * 8 + LOGIC_Y);

		return flags;
	}

	/* �������� ������-������ ���� */
	if (b3) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ���� */
		flags = MarkBlock(bound3.x,bound3.y);
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������� ���� */
		lpBall->angle = 360 + 180 - lpBall->angle;
		SetSpeedBall(lpBall);
		/* ������������ ���������� */
		UpdateBallX(lpBall,bound3.x * 16 + LOGIC_X - wBall);

		return flags;
	}

	/* �������� ������� �������-������ ���� */
	if (b5) {
		int flags;
		/* ���� ����� ��� �������� ������, �� ��������� ��� */
		UnlockSwerve(lpBall);
		/* ������� ���� */
		flags = MarkBlock(bound5.x,bound5.y);
		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL) && flags)
			return flags;
		/* ������������ ���������� � ������� ���� */
		if ((bound5.offset_x + 1) > (8 - bound5.offset_y)) {
			lpBall->angle = 360 - lpBall->angle;
			UpdateBallY(lpBall,(bound5.y + 1) * 8 + LOGIC_Y);
		} else if ((bound5.offset_x + 1) < (8 - bound5.offset_y)) {
			lpBall->angle = 360 + 180 - lpBall->angle;
			UpdateBallX(lpBall,bound5.x * 16 + LOGIC_X - wBall);
		} else {
			/* ��� ����� ��� �������������� ������������ �� ���� */
			if (lpBall->angle > 315) {
				lpBall->angle = 360 + 180 - lpBall->angle;
				UpdateBallX(lpBall,bound5.x * 16 + LOGIC_X - wBall);
			} else {
				lpBall->angle = 360 - lpBall->angle;
				UpdateBallY(lpBall,(bound5.y + 1) * 8 + LOGIC_Y);
			}
		}
		/* ��������� �������� */
		SetSpeedBall(lpBall);

		return flags;
	}

	return 0;
}

/* ��������� ������������ � �������� */
static int CheckCollideBlock(LPBALL lpBall)
{
	int flags;

	/* ������������ �� ���� ��������� ������������ */
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

/* ������ ��������� ������� ���� �������� ������ */
static void CreateNewAngleBall(LPBALL lpBall)
{
	/* ���� ��������� ������ */
	static const int angle90[] = {113,135,157,293,315,337};
	static const int angle180[] = {203,225,247,23,45,67};
	static const int angle270[] = {293,315,337,113,135,157};
	static const int angle360[] = {23,45,67,203,225,247};
	const int *angle;

	/* ���������� ����� ���� �������� ������ */
	if (lpBall->angle < 90)
		angle = angle90;
	else if (lpBall->angle < 180)
		angle = angle180;
	else if (lpBall->angle < 270)
		angle = angle270;
	else
		angle = angle360;

	/* ������������� ��������� ���� */
	lpBall->angle = angle[random(6)];
	SetSpeedBall(lpBall);
}

/* �������� ������������ � �������� ��������� */
static int CheckCollideAlien(LPBALL lpBall)
{
	int x,y,rBall;

	/* ������������ �������� �������� ��� ��� */
	if (!alien.on)
		return 0;

	/* ���������� ���������� ������ */
	rBall = GetWidthBall() / 2;
	x = lpBall->ix + rBall;
	y = lpBall->iy + rBall;

	/* ����� �� �������� � �������� �������� */
	if ((x < alien.ix + 1) || (x > alien.ix + 22))
		return 0;
	if ((y < alien.iy + 1) || (y > alien.iy + 12))
		return 0;

	/* ����� ����� � �������� ��������, ������� ���� ������� */
	UnlockSwerve(lpBall);
	/* ���������� ����� ���� �������� ������ */
	CreateNewAngleBall(lpBall);

	/* ���������� �������� ������ */
	CreateBang();
	/* ������� �������� ���� �� �������� �������� �������� */
	AddScore(350);

	/* ������� ������� */
	play.bang = 1;

	return 1;
}

/* ���������� ����� � ����� */
static void MoveBall(void)
{
	int i;

	/* �������� ��������� �������� ����� 'BONUS_SMASH_BALL' */
	if (CHECK_FLAG(bonus.flags,F_SMASH_BALL)) {
		/* �������� ������� � ���� ��������� ������� ���� */
		if (--timeEndingSmashBall <= 0) {
			for (i = 0; i < BALLS_COUNT; i++) {
				LPFULL_PICTURE lpPicture = &scene.pBall[i];
				lpPicture->picture.type = BALL_NORMAL;
				lpPicture->shadow.type = BALL_NORMAL;
			}
			bonus.flags &= ~F_SMASH_BALL;
		}
	}

	/* ��������� ��� ������ */
	for (i = 0; i < BALLS_COUNT; i++) {
		LPBALL lpBall = &ball[i];
		LPFULL_PICTURE lpPicture = &scene.pBall[i];

		/* ����� ������������ ��� ��� */
		if (!lpBall->on)
			continue;

		/* �������� �������������� ������� � ������� */
		if (lpBall->hand.count <= 0) {
			/* ���������� ����� */
			lpBall->x += lpBall->vx;
			lpBall->y += lpBall->vy;

			/* ������� ������������� �������� ��������� */
			lpBall->ix = ftol(lpBall->x);
			lpBall->iy = ftol(lpBall->y);

			/* �������� ������������ �� ������� */
			CheckCollideBorder(lpBall);
			/* ��������� ����������� � ������ ���������� */
			CheckCollideSwerve(lpBall);
			/* �������� ������������ � �������� */
			if (CheckCollideRacket(lpBall))
				lpBall->count = TIME_NEW_COURSE_BALL;
			/* �������� ������������ � �������� */
			if (CheckCollideBlock(lpBall))
				lpBall->count = TIME_NEW_COURSE_BALL;
			/* �������� ������������ � �������� ��������� */
			if (CheckCollideAlien(lpBall))
				lpBall->count = TIME_NEW_COURSE_BALL;
			/* �������� ������� ���������� */
			if (--lpBall->count <= 0) {
				CreateNewAngleBall(lpBall);
				lpBall->count = TIME_NEW_COURSE_BALL;
			}
		} else {
			/* ����� ����������� � �������, �������� ������� */
			lpBall->hand.count--;
			/*
			 * ���� ��� ����� ������ �� ������� ���
			 * ����� ��� ���������� ���� ���������.
			 */
			if (lpBall->hand.count <= 0) {
				if (CHECK_FLAG(bonus.flags,F_START_RAUND))
					bonus.flags = 0;
			}
			/* ��������� ������� ������ */
			lpBall->x = racket.x + lpBall->hand.offset;
			lpBall->y = racket.y - 6;
			/* ������� ������������� �������� ��������� */
			lpBall->ix = ftol(lpBall->x);
			lpBall->iy = ftol(lpBall->y);
		}

		/* �������� �������� ����� 'BONUS_SMASH_BALL' */
		if (CHECK_FLAG(bonus.flags,F_SMASH_BALL)) {
			/* ���������� ��������� ������ */
			lpPicture->picture.X = lpBall->ix - 1;
			lpPicture->picture.Y = lpBall->iy - 1;
			/* ���������� ��������� ���� */
			lpPicture->shadow.X = lpBall->ix + 6;
			lpPicture->shadow.Y = lpBall->iy + 6;
		} else {
			/* ���������� ��������� ������ */
			lpPicture->picture.X = lpBall->ix;
			lpPicture->picture.Y = lpBall->iy;
			/* ���������� ��������� ���� */
			lpPicture->shadow.X = lpBall->ix + 7;
			lpPicture->shadow.Y = lpBall->iy + 7;
		}
	}

	/* �������� ���������� �������� */
	if (--timeAddSpeedBall <= 0) {
		/* �������� �������� ������� */
		for (i = 0; i < BALLS_COUNT; i++) {
			LPBALL lpBall = &ball[i];
			if (lpBall->on) {
				lpBall->v += ADD_SPEED_BALL;
				if (lpBall->v > MAX_SPEED_BALL)
					lpBall->v = MAX_SPEED_BALL;
				SetSpeedBall(lpBall);
			}
		}
		/* ������� ������� ���������� �������� */
		timeAddSpeedBall = TIME_ADD_SPEED_BALL;
	}

	/* ���� ��������� ���� 'BONUS_HAND' */
	if (CHECK_FLAG(bonus.flags,F_HAND)) {
		/* ���� ������ ����������� ������� ���������� ������ */
		if (input.lButton || input.SPACE) {
			for (i = 0; i < BALLS_COUNT; i++)
				ball[i].hand.count = 0;
			/* ������� ��� ������ ���� ��� ������ ���� */
			if (CHECK_FLAG(bonus.flags,F_START_RAUND))
				bonus.flags = 0;
		}
	}
}

/* ����������� �������� �� ���� 'x' � 'y' ��� ��������� �������� */
static void SetSpeedAlien(void)
{
	/* ��� ������� ��������� ���������� ������� 'SetSpeedBall'. */
	double rad;

	/* ��������� ���� � ������� */
	rad = alien.angle * PI / 180.0;
	/* ��������� ��������  �� 'x' � 'y' */
	alien.vx = alien.v * cos(rad);
	alien.vy = alien.v * sin(rad);
}

/* ������� �������� �������� � ���������� ��� */
static void CreateAlien(void)
{
	/* ������� ��� ���� ��������� �������� */
	memset(&alien,0,sizeof(alien));
	/* �������� �������� �������� */
	alien.on = 1;
	/* ��������� ���������� */
	if (random(2)) alien.x = LOGIC_X + 160;
	else alien.x = LOGIC_X + 55;
	/* ������� ������������� ���������� */
	alien.ix = ftol(alien.x);
	alien.iy = ftol(alien.y);
	/* ������� �������� */
	alien.v = 0.0;
	SetSpeedAlien();
	/* ���������� ����������� � �������� */
	scene.pAlien.type = 0;
	if (nLevel % 2) {
		scene.pAlien.show = SHOW_ALIEN_UFO;
		alien.animation.frames = lpAlienUfo;
	} else {
		scene.pAlien.show = SHOW_ALIEN_BIRD;
		alien.animation.frames = lpAlienBird;
	}
}

/* ����������� ���� ����������� */
static void NextFrameAlien(void)
{
	int point = alien.animation.point;

	/* �������� ������� ������ ������ */
	if (++alien.animation.count < alien.animation.frames[point].count)
		return;

	/* ����������� ���� */
	point++;
	/* �������� ����� ��������� ������ */
	if (alien.animation.frames[point].number < 0)
		point = 0;

	/* �������� ��������� ���� */
	scene.pAlien.type = alien.animation.frames[point].number;
	/* ���������� ����������� ���������� */
	alien.animation.point = point;
	alien.animation.count = 0;
}

/* ��������� ��� ��������� �������� �� */
#define STATE_RUN                   1    /* �������� */
#define STATE_DETOUR                2    /* ����� ������ */
#define STATE_CHANGE_COURSE         3    /* ��������� ����������� */
#define STATE_CHANGE_SPEED          4    /* ��������� �������� */

/* �������� ����������� ��� ����� �� ������ */
#define V_VIOLATION               0.8

/* ��������� ��� ��������� 'DETOUR' */
#define DETOUR_VIOLATION_TOP        1    /* ��������� ������� ������� */
#define DETOUR_VIOLATION_LEFT       2    /* ��������� ����� ������� */
#define DETOUR_VIOLATION_RIGHT      3    /* ��������� ������ ������� */

#define DETOUR_BLOCK_RIGHT          4    /* ������������ � ������ ������ */
#define DETOUR_BLOCK_BOTTOM         5    /* ������������ � ������ ������ */
#define DETOUR_BLOCK_LEFT           6    /* ������������ � ����� ������ */
#define DETOUR_BLOCK_TOP            7    /* ������������ � ������� ������ */

/* ��� �������� �������������� ���������� ���������� */
static struct {
	int subject;                     /* ���� ������������� ��������� */
	int message;                     /* ��������� */
} info;

/* ��������� ������������ ��������� �������� � ������� */
static void CollideAlienAndRacket(void)
{
	RECT r,a;

	/* ��������� ���������� ������� */
	r.left = racket.x;
	r.top = racket.y;
	r.right = racket.x + GetWidthRacket() - 1;
	r.bottom = racket.y + GetHeightRacket() - 1;

	/* ��������� ���������� ��������� �������� */
	a.left = alien.ix + 5;
	a.top = alien.iy + 3;
	a.right = alien.ix + 18;
	a.bottom = alien.iy + 10;

	/* �������� ��������� ������� ����� ������� � �������� �������� */
	if (((r.left >= a.left && r.left <= a.right) ||
		(r.right >= a.left && r.right <= a.right)) &&
		((r.top >= a.top && r.top <= a.bottom) ||
		(r.bottom >= a.top && r.bottom <= a.bottom))) {
		/* ������� ������������ � �������� ��������� */
		CreateBang();
		/* ������� �������� ���� �� �������� �������� �������� */
		AddScore(350);
		return;
	}

	/* �������� ��������� ������� ����� ��������� �������� � ������� */
	if (((a.left >= r.left && a.left <= r.right) ||
		(a.right >= r.left && a.right <= r.right)) &&
		((a.top >= r.top && a.top <= r.bottom) ||
		(a.bottom >= r.top && a.bottom <= r.bottom))) {
		/* �������� �������� ������������ � �������� */
		CreateBang();
		/* ������� �������� ���� �� �������� �������� �������� */
		AddScore(350);
		return;
	}
}

/* �������� �������� �������� �� ������������ � ��������� */
static void CollideAlien(void)
{
	BOUND bound;
	int b1,b2,b3,b4;

	/* �������� ����� �� ������ ������� */
	if (alien.iy + 2 >= LOGIC_Y + LOGIC_HEIGHT) {
		/* ���������� �������� �������� */
		CreateAlien();
		return;
	}

	/* �������� ����� �� ����� ������ */
	if (alien.ix + 7 <= LOGIC_X) {
		/* ���������� ��������� � 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* ��������� �������������� ���������� */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_VIOLATION_LEFT;

		return;
	}

	/* �������� ����� �� ������ ������ */
	if (alien.ix + 16 >= LOGIC_X + LOGIC_WIDTH) {
		/* ���������� ��������� � 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* ��������� �������������� ���������� */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_VIOLATION_RIGHT;

		return;
	}

	/* �������� ������ �� ������� ������� */
	if (alien.iy + 6 <= LOGIC_Y) {
		/* ���������� ��������� � 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* ��������� �������������� ���������� */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_VIOLATION_TOP;

		return;
	}

	/* �������� ����������� ����� �� ������������ */
	b1 = GetBlock(&bound,alien.ix+13,alien.iy+7);   /* ������� ������ ����� */
	b2 = GetBlock(&bound,alien.ix+12,alien.iy+8);   /* ������� ������ ����� */
	b3 = GetBlock(&bound,alien.ix+10,alien.iy+7);   /* ������� ����� ����� */
	b4 = GetBlock(&bound,alien.ix+12,alien.iy+5);   /* ������� ������� ����� */

	/* �������� ������������ */
	if (b1) {
		/* ���������� ��������� � 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* ��������� �������������� ���������� */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_BLOCK_RIGHT;
	} else if (b2) {
		/* ���������� ��������� � 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* ��������� �������������� ���������� */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_BLOCK_BOTTOM;
	} else if (b3) {
		/* ���������� ��������� � 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* ��������� �������������� ���������� */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_BLOCK_LEFT;
	} else if (b4) {
		/* ���������� ��������� � 'DETOUR' */
		alien.automat.state = STATE_DETOUR;
		alien.automat.count = 0;
		/* ��������� �������������� ���������� */
		info.subject = STATE_DETOUR;
		info.message = DETOUR_BLOCK_TOP;
	}
}

/* ��������� �������� ������ */
static void StateDetourAlien(void)
{
	/* ��������� �������� � ������� ����������� */
	if (alien.automat.count > 0) {
		alien.automat.count--;
		return;
	}

	/* ��������, ���� ��������� */
	if (info.subject == STATE_DETOUR) {
		/* �������� ��� ��������� */
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
		/* ���������� �������� */
		alien.v = V_VIOLATION;
		SetSpeedAlien();
		/* ��������� ������� � ������� ��������� */
		alien.automat.count = 2;
		info.subject = 0;
		return;
	}

	/* ���������� ��������� */
	alien.automat.state = STATE_CHANGE_COURSE;
	alien.automat.count = 0;
}

/* ��������� �������� */
static void StateRunAlien(void)
{
	int state;

	/* ��������� �������� � ������� ����������� */
	if (alien.automat.count > 0) {
		alien.automat.count--;
		return;
	}

	/* ������� ����� ��������� */
	state = random(6);
	if (state == 0) {
		/* �������� ������� ��������� */
		alien.automat.count = 20 + random(30);
	} else if (state <= 2) {
		/* ���������� �� ��������� ��������� ����������� */
		alien.automat.state = STATE_CHANGE_COURSE;
	} else {
		/* ���������� �� ��������� ��������� �������� */
		alien.automat.state = STATE_CHANGE_SPEED;
	}
}

/* ��������� ����������� */
static void StateChangeCourseAlien(void)
{
	/* ����������� ��������� ����� */
	if (info.subject != STATE_CHANGE_COURSE) {
		/* �������� ���� ���������� */
		if (random(2))
			info.message = random(3) + 1;
		else
			info.message = -(random(3) + 1);
		/* �������� ����� �������� */
		alien.automat.count = 30 + random(30);
		/* ���������� ����� ���� */
		info.subject = STATE_CHANGE_COURSE;
	}

	/* ��������� ����� �� ��������� */
	if (alien.automat.count <= 0) {
		/* ���������� ��������� � �������� */
		alien.automat.state = STATE_RUN;
		info.subject = 0;
		/* ������ */
		return;
	}

	/* ��������� ������� ��������� */
	alien.automat.count--;
	/* �������� ���� */
	alien.angle += info.message;
	if (alien.angle < 0)
		alien.angle += 360;
	else if (alien.angle >= 360)
		alien.angle -= 360;
	SetSpeedAlien();
}

/* ��������� �������� */
static void StateChangeSpeedAlien(void)
{
	/* �������� �������� */
	if (random(2)) alien.v += 0.2;
	else alien.v -= 0.2;
	/* ������������ ��������� */
	if (alien.v < 0.2) {
		alien.v = 0.2;
	} else if (alien.v > 1.0) {
		alien.v = 1.0;
	}

	/* ��������� � ��������� �������� */
	alien.automat.state = STATE_RUN;
	alien.automat.count = 10 + random(20);
	/* �������� �������� */
	SetSpeedAlien();
}

/* ���������� ���������� ����������� ������ */
static int GetCountBlock(void)
{
	int i,j,count;
	UCHAR (*p)[15];

	/* ���������� ���������� */
	count = 0;
	p = &block[3];
	/* ��������� ��� ����� */
	for (i = 0; i < 12; i++) {
		for (j = 0; j < 15; j++)
			if ((*p)[j] && !((*p)[j] & 0x80))
				count++;
		p++;
	}

	return count;
}

/* ����������� �������� �������� */
static void MoveAlien(void)
{
	/* �������� �������� ������������ ��� ��� */
	if (!alien.on) {
		/* ���������� ���������� � �������� �������� */
		int level,AlienBlock;

		/* ���� ������������ ����� �� ������ */
		if (bang.on || (bonus.flags & F_SMART_BOMB))
			return;

		/* ���������� ���������� � �������� �������� */
		level = nLevel % LEVEL_COUNT;
		AlienBlock = levels[level].AlienBlock;

		/* �������� ������ ��������� �������� */
		if (!AlienBlock)
			return;

		/* �������� ������� ��������� ��������� �������� */
		if (GetCountBlock() > AlienBlock)
			return;

		/* �������� �������� �������� */
		CreateAlien();
	}

	/* ���������� �������� �������� */
	alien.x += alien.vx;
	alien.y += alien.vy;
	/* ������� ������������� ���������� */
	alien.ix = ftol(alien.x);
	alien.iy = ftol(alien.y);

	/* ��������� �� ������������, ���� ��������� �� '�����' */
	if (alien.automat.state != STATE_DETOUR)
		CollideAlien();

	/* �������� ������������ � �������� */
	CollideAlienAndRacket();
	/* ��������, ��� �������� �������� ��� ������������ */
	if (!alien.on)
		return;

	/* �������� �������� � ����������� �� ��������� */
	switch (alien.automat.state) {
		/* �������� */
		case STATE_RUN:
			StateRunAlien();
			break;
		/* ����� ����������� */
		case STATE_DETOUR:
			StateDetourAlien();
			break;
		/* ��������� ����������� */
		case STATE_CHANGE_COURSE:
			StateChangeCourseAlien();
			break;
		/* ��������� �������� */
		case STATE_CHANGE_SPEED:
			StateChangeSpeedAlien();
			break;
		/* ������������� ��������� �� ��������� */
		default:
			alien.automat.state = STATE_RUN;
			alien.automat.count = 0;
	}

	/* �������� ��������� ���� ����������� */
	NextFrameAlien();
	/* ������� ���������� */
	scene.pAlien.X = alien.ix;
	scene.pAlien.Y = alien.iy;
}

/* ���������� ����� */
static void TransferBomb(void)
{
	RECT r;
	POINT p;

	/* ���������� ����� */
	bomb.y += bomb.vy;
	/* ��������� �������� */
	if (bomb.vy < MAX_SPEED_Y) {
		/* ���������� ��������� */
		bomb.vy += ACCELERATION_Y;
		if (bomb.vy > MAX_SPEED_Y)
			bomb.vy = MAX_SPEED_Y;
	}
	/* ������� ������������� ���������� 'y' */
	bomb.iy = ftol(bomb.y);

	/* ��������� ���������� ������� */
	r.left = racket.x;
	r.top = racket.y;
	r.right = racket.x + GetWidthRacket() - 1;
	r.bottom = racket.y + GetHeightRacket() - 1;

	/* ��������� ����� � ����� */
	p.x = bomb.ix + 3;
	p.y = bomb.iy + 5;

	/* �������� ��������� � ������� */
	if ((p.x >= r.left && p.x <= r.right) &&
		(p.y >= r.top && p.y <= r.bottom)) {
		/* ��������� ������� */
		SetFlySliverAnima();
		return;
	}

	/* �������� ����������� ������ ������� */
	if (bomb.iy > (LOGIC_Y + LOGIC_HEIGHT)) {
		/* ����� ����� �� ����� */
		bomb.on = 0;
		scene.pBomb.picture.show = 0;
		scene.pBomb.shadow.show = 0;
		/* ���������� ������� �������� */
		bomb.count = 25;
	} else {
		/*
		 * ���������� ���������� ����� � �� ����
		 * (���������� 'x' �� ����������, ������ ����).
		 */
		scene.pBomb.picture.Y = bomb.iy;
		scene.pBomb.shadow.Y = bomb.iy + 7;
	}
}

/* ���������� ����� */
static void CreateBomb(void)
{
	/* �������� ������� �������� */
	if (bomb.count > 0) {
		bomb.count--;
		return;
	}
	/* ������ ������� ����������� */
	if (random(3)) {
		bomb.count = 10 + random(20);
		return;
	}

	/* ��������� ����� */
	bomb.on = 1;
	bomb.ix = alien.ix + 8;
	bomb.iy = alien.iy + 4;
	/* ���������� ������� ���������� */
	bomb.y = bomb.iy;
	bomb.vy = 0.0;

	/* ��������� ����� � ���� */
	scene.pBomb.picture.show = 1;
	scene.pBomb.picture.X = bomb.ix;
	scene.pBomb.picture.Y = bomb.iy;
	scene.pBomb.shadow.show = 1;
	scene.pBomb.shadow.X = bomb.ix + 8;
	scene.pBomb.shadow.Y = bomb.iy + 7;
}

/* ����������� ����� */
static void MoveBomb(void)
{
	/* ���� ����� ������������, �� ���������� �� */
	if (bomb.on)
		TransferBomb();
	/* ��������� ����� ���� �������� �������� ������������ */
	else if (alien.on)
		CreateBomb();
}

/* �������� ����� �������� ���� */
static void ResetOldBonus(void)
{
	/* ���������� ����������� ����� */
	if (CHECK_FLAG(bonus.flags,F_GUN)) {
		/* ������� ������� ���������� */
		racket.animation.frames = lpGunToNormalRacket;
		racket.animation.point = 0;
		racket.animation.count = 0;
	} else if (CHECK_FLAG(bonus.flags,F_EXTENDED_RACKET)) {
		/* ������� ������� ���������� */
		racket.animation.frames = lpExtendedToNormalRacket;
		racket.animation.point = 0;
		racket.animation.count = 0;
	} else if (CHECK_FLAG(bonus.flags,F_HAND)) {
		int i;
		/* ������� ��� �������������� ������ */
		for (i = 0; i < BALLS_COUNT; i++)
			ball[i].hand.count = 0;
	} else if (CHECK_FLAG(bonus.flags,F_SMASH_BALL)) {
		int i;
		/* ���������� ������� ��� ������� */
		for (i = 0; i < BALLS_COUNT; i++) {
			LPFULL_PICTURE lpPicture = &scene.pBall[i];
			lpPicture->picture.type = BALL_NORMAL;
			lpPicture->shadow.type = BALL_NORMAL;
		}
	}

	/* ���������� ����� */
	bonus.flags &= ~(F_POINTS | F_SMART_BOMB | F_SLOW_BALL |
		F_SMASH_BALL | F_HAND | F_GUN | F_EXTENDED_RACKET | F_ROCKET_PACK);
}

/* ������� �������� ���� */
static void RunBonusPoints(void)
{
	/* ������� �������� ���� */
	AddScore(5000);

	/* ��������� ���� �������� ����� */
	bonus.flags |= F_POINTS;
}

/* ������ ���� "Kill Aliens" */
static void RunBonusSmartBomb(void)
{
	/* ������ �������� �������� */
	if (alien.on)
		CreateBang();

	/* ��������� ���� �������� ����� */
	bonus.flags |= F_SMART_BOMB;
}

/* ������� �������������� ����� */
static void RunBonusExtraLife(void)
{
	/* ������� ����� */
	lifes++;

	/* ��������� ���� �������� ����� */
	bonus.flags |= F_EXTRA_LIFE;
}

/* ������ ����� "���������� ������" */
static void RunBonusSlowBall(void)
{
	int i;

	/* ��������� ��� ������ � ������� �������� */
	for (i = 0; i < BALLS_COUNT; i++) {
		if (ball[i].on) {
			ball[i].v = MIN_SPEED_BALL;
			SetSpeedBall(&ball[i]);
		}
	}

	/* ���������� ������� ���������� �������� */
	timeAddSpeedBall = TIME_ADD_SPEED_BALL / 2;

	/* ��������� ���� �������� ����� */
	bonus.flags |= F_SLOW_BALL;
}

/* ������ ����� "������ �����" */
static void RunBonusSmashBall(void)
{
	int i;

	/* ��������� ������� ����� */
	for (i = 0; i < BALLS_COUNT; i++) {
		LPFULL_PICTURE lpPicture = &scene.pBall[i];
		lpPicture->picture.type = BALL_SMASH;
		lpPicture->shadow.type = BALL_SMASH;
	}

	/* ��������� ����� �������� ����� */
	timeEndingSmashBall = TIME_ENDING_SMASH_BALL;

	/* ��������� ���� �������� ����� */
	bonus.flags |= F_SMASH_BALL;
}

/* ������ ����� "��� ������" */
static void RunBonusTripleBall(void)
{
	FULL_PICTURE pBall;
	BALL lBall;
	int angle,angle1,angle2;
	int i;

	/* ���� ������������ ����� */
	for (i = 0; i < BALLS_COUNT; i++) {
		if (ball[i].on) {
			/* �������� �������� ������ */
			memcpy(&pBall,&scene.pBall[i],sizeof(pBall));
			memcpy(&lBall,&ball[i],sizeof(lBall));
			break;
		}
	}

	/* ��������, ��� ����� ������ */
	if (i == BALLS_COUNT)
		return;

	/* ������� ���� �������� ������ */
	if (lBall.swerve.lock)
		angle = lBall.swerve.angle;
	else
		angle = lBall.angle;

	/*
	 * ��������� �������� �����.
	 * ����� ������� ����� ������� ����� ������ ����������
	 * �� ����, ��� ����� � ������ ��� ������������ ����� ����
	 * �������� '������ ����������'. ��� ����� ������ ���������
	 * �� ������������ '������ ����������'.
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

	/* ��������� ������ */
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

	/* ��������� ���� �������� ����� */
	bonus.flags |= F_TRIPLE_BALL;
}

/* ������ ����� "����" */
static void RunBonusHand(void)
{
	/* ��������� ���� �������� ����� */
	bonus.flags |= F_HAND;
}

/* ������ ����� "���������� �������" */
static void RunBonusGun(void)
{
	/* ��������� �������� �������� ������� */
	if (racket.animation.frames == NULL) {
		racket.animation.frames = lpNormalToGunRacket;
		racket.animation.point = 0;
		racket.animation.count = 0;
	} else
		racket.frames = lpNormalToGunRacket;

	/* ��������� ���� �������� ����� */
	bonus.flags |= F_GUN;
}

/* ������ ����� "������� �������" */
static void RunBonusExtendedRacket(void)
{
	/* ��������� �������� �������� ������� */
	if (racket.animation.frames == NULL) {
		racket.animation.frames = lpNormalToExtendedRacket;
		racket.animation.point = 0;
		racket.animation.count = 0;
	} else
		racket.frames = lpNormalToExtendedRacket;

	/* ��������� ���� �������� ����� */
	bonus.flags |= F_EXTENDED_RACKET;
}

/* ������ ����� "������" */
static void RunBonusRocketPack(void)
{
	/* �������� �������� ������ �� ������ */
	SetFlyRocketAnima();
	/* ������� ������� */
	play.rocket = 1;
	/* ��������� ���� �������� ����� */
	bonus.flags |= F_ROCKET_PACK;
}

/* ������������ �������� ���� */
static void RunBonus(void)
{
	/* ������� ����� �������� ���� */
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

/* ������� ������������� ���� �� ������� */
static void CreateBonus400Points(void)
{
	int angle,r,b;
	double rad,speed;

	/* ������� �������, ���� ������� �� ������ */
	if (scene.pBonus.type != BONUS_ROCKET_PACK)
		play.bonus = 1;

	/* �������� �������� ���� */
	AddScore(400);
	/* ���������� �������� ���� */
	RunBonus();

	/* ����������� ����������� ����� �������� */
	r = racket.x + GetWidthRacket() / 2;
	b = bonus.ix + GetWidthBonus() / 2;

	/* ����������� ���� � ����������� �� ����� ����������� */
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

	/* ��������� ���� � ������� */
	rad = angle * PI / 180.0;
	/* ����������� �������� �� ���� */
	bonus.vx = speed * cos(rad);
	bonus.vy = speed * sin(rad);

	/* ������������� ��� ������������� ������ */
	scene.pBonus.type = BONUS_400_POINTS;
}

/* ���������� �������� ���� */
static void MoveBonus(void)
{
	/* ���� ���� �� ������������ �� ������ */
	if (!bonus.on)
		return;

	/* ������������ ���������� ���� */
	if (scene.pBonus.type != BONUS_400_POINTS) {
		RECT r,b;
		/* ���������� ���� */
		bonus.y += bonus.vy;
		/* ������� �������� */
		if (bonus.vy < MAX_SPEED_Y) {
			/* ���������� ��������� */
			bonus.vy += ACCELERATION_Y;
			if (bonus.vy > MAX_SPEED_Y)
				bonus.vy = MAX_SPEED_Y;
		}
		/* ������� ������������� �������� ��������� */
		bonus.iy = ftol(bonus.y);
		/* ��������� ���������� ������� */
		r.left = racket.x + 4;
		r.top = racket.y + 8;
		r.right = racket.x + GetWidthRacket() - 1 - 4;
		r.bottom = racket.y + GetHeightRacket() - 1;
		/* ��������� ���������� ������ */
		b.left = bonus.ix;
		b.top = bonus.iy;
		b.right = bonus.ix + GetWidthBonus() - 1;
		b.bottom = bonus.iy + GetHeightBonus() - 1;
		/* �������� ��������� ������� ����� ������� � ���� */
		if (((r.left >= b.left && r.left <= b.right) ||
			(r.right >= b.left && r.right <= b.right)) &&
			((r.top >= b.top && r.top <= b.bottom) ||
			(r.bottom >= b.top && r.bottom <= b.bottom))) {
			/* ������� ������������ � ������� */
			CreateBonus400Points();
		} else if (((b.left >= r.left && b.left <= r.right) ||
			(b.right >= r.left && b.right <= r.right)) &&
			((b.top >= r.top && b.top <= r.bottom) ||
			(b.bottom >= r.top && b.bottom <= r.bottom))) {
			/* ����� ������������ � �������� */
			CreateBonus400Points();
		}
		/* ���������� ���������� ������� */
		scene.pBonus.Y = bonus.iy;
		scene.pBonus.X = bonus.ix;
	} else {
		/* ������� ������ ������ */
		int width = GetWidthBonus();
		/* ���������� ���� */
		bonus.y += bonus.vy;
		bonus.x += bonus.vx;
		/* �������� ��������� */
		bonus.vy += 0.04;
		/* ������� ������������� �������� ��������� */
		bonus.iy = ftol(bonus.y);
		bonus.ix = ftol(bonus.x);
		/* �������� ������������ �� �������� */
		if ((bonus.ix <= LOGIC_X) && (bonus.vx < 0)) {
			bonus.vx = -bonus.vx;
			bonus.ix = LOGIC_X;
		} else if ((bonus.ix >= (LOGIC_X + LOGIC_WIDTH - width)) && (bonus.vx > 0)) {
			bonus.vx = -bonus.vx;
			bonus.ix = LOGIC_X + LOGIC_WIDTH - width;
		}
		/* ���������� ���������� ������� */
		scene.pBonus.Y = bonus.iy;
		scene.pBonus.X = bonus.ix;
	}

	/* �������� ����� �� ������� �������� ���� */
	if (bonus.iy > (LOGIC_Y + LOGIC_HEIGHT)) {
		/* ��������� ������ */
		bonus.on = 0;
		scene.pBonus.show = 0;
	}
}

/* ���������� ���� ���������� */
static void MoveSwerves(void)
{
	int i;

	/* ��������� ��� ����� ���������� */
	for (i = 0; i < SWERVES_COUNT; i++) {
		/* ���������� ��������� */
		LPPICTURE lpPicture = &scene.pSwerves[i];
		LPSWERVE lpSwerve = &swerve[i];

		/* ���� ���� ������������, ���������� ��� */
		if (lpPicture->show) {
			/* �������� ������� ����� ��������� */
			if (--lpSwerve->count > 0)
				continue;
			/*
			 * ������ ��������� ����� ����������.
			 * ����������� �������� � ���������� ��������� ����� 55%.
			 * ����������� �������� � ����������� ��������� ����� 45%.
			 */
			if (random(100) < 45) {
				if (lpSwerve->on) {
					lpSwerve->on = 0;
					lpPicture->type = PICTURE_SWERVE_OFF;
					/* ������� ������� */
					play.swerve = 1;
				}
			} else {
				if (!lpSwerve->on) {
					lpSwerve->on = 1;
					lpPicture->type = PICTURE_SWERVE_ON;
					/* ������� ������� */
					play.swerve = 1;
				}
			}
			/* ���������� ������� ����� ��������� */
			lpSwerve->count = 30 + random(100);
		}
	}
}

/* ���������� ���� ������ ��������� �������� */
static void MoveBang(void)
{
	/* �������� ����������� ������ */
	if (!bang.on)
		return;

	/* �������� ������� ������ ������ */
	if (++bang.count < lpBang[bang.point].count)
		return;

	/* ����������� ���� */
	bang.point++;
	bang.count = 0;
	/* �������� ����� �������� */
	if (lpBang[bang.point].number < 0) {
		bang.on = 0;
		scene.pBang.show = 0;
		return;
	}

	/* �������� ��������� ���� */
	scene.pBang.type = lpBang[bang.point].number;
}

/* ������������ ������������ ����� � ������ ��� �������� ��������� */
static void CheckCollisionBullet(LPPICTURE lpBullet, LPBULLET_PLOP lpPlop)
{
	BOUND bound1,bound2;
	int b1,b2,y;

	/* ������� ����� ���������� ����������� ����� */
	b1 = GetBlock(&bound1,lpBullet->X,lpBullet->Y);     /* ������� ����� */
	b2 = GetBlock(&bound2,lpBullet->X+3,lpBullet->Y);   /* ������� ������ */

	/* �������� ������������ � ������� */
	if ((b1 && b2) && (bound1.x != bound2.x)) {
		/* ��������� �� ����� ���� ������ ������������� ������ */
		if ((16 - bound1.offset_x) >= bound2.offset_x) {
			MarkBlock(bound1.x,bound1.y);
		} else {
			MarkBlock(bound2.x,bound2.y);
		}
		/* ������������ ���������� 'y' */
		y = (bound1.y + 1) * 8 + LOGIC_Y;
	} else if (b1) {
		/* ������� ���� */
		MarkBlock(bound1.x,bound1.y);
		/* ������������ ���������� 'y' */
		y = (bound1.y + 1) * 8 + LOGIC_Y;
	} else if (b2) {
		/* ������� ���� */
		MarkBlock(bound2.x,bound2.y);
		/* ������������ ���������� 'y' */
		y = (bound2.y + 1) * 8 + LOGIC_Y;
	} else {
		/* �������� ������������ � �������� ��������� */
		RECT a;
		int x1,x2,y1;

		/* ������������ �������� �������� ��� ��� */
		if (!alien.on)
			return;

		/* ��������� ���������� ��������� �������� */
		a.left = alien.ix + 5;
		a.top = alien.iy + 3;
		a.right = alien.ix + 18;
		a.bottom = alien.iy + 10;
		/* ��������� ����������� ����� ������ */
		x1 = lpBullet->X + 1;
		x2 = lpBullet->X + 2;
		y1 = lpBullet->Y;

		/* ������ �� �������� � �������� �������� */
		if (((x1 < a.left) && (x2 < a.left)) ||
			((x1 > a.right) && (x2 > a.right)))
			return;
		if ((y1 < a.top) || (y1 > a.bottom))
			return;

		/* ���������� �������� ������ */
		CreateBang();
		/* ������� �������� ���� �� �������� �������� �������� */
		AddScore(350);

		/* ������������ ���������� 'y' */
		y = y1;
	}

	/* ���������� ������� ������ */
	lpBullet->type = TYPE_BULLET_PLOP_0;
	lpBullet->X -= 2;
	lpBullet->Y = y;
	/* ���������� �������� ������ ������ */
	lpPlop->count = 0;
	lpPlop->point = 0;
}

/* ����������� ������ */
static void MoveBullet(void)
{
	int i;

	/* ��������� ��� ������ */
	for (i = 0; i < BULLETS_COUNT; i++) {
		LPPICTURE lpBullet = &scene.pBullet[i];
		LPBULLET_PLOP lpPlop = &plop[i];
		/* �������� ��� ������ ���������� */
		if (lpBullet->show) {
			/* ���� ��� ������ �� ���������� �� */
			if (lpBullet->type < TYPE_BULLET_PLOP_0) {
				lpBullet->Y -= SPEED_BULLET;
				/* �������� ����� �� ������� �������� ���� */
				if (lpBullet->Y <= -8) {
					lpBullet->show = 0;
					continue;
				}
				/* ������� ������� */
				if (lpBullet->type == TYPE_BULLET_0)
					lpBullet->type = TYPE_BULLET_1;
				else
					lpBullet->type = TYPE_BULLET_0;
				/* �������� ������������ � ������ ��� �������� ��������� */
				CheckCollisionBullet(lpBullet,lpPlop);
			} else {
				/* �������� ������� ������ ������ */
				if (++lpPlop->count < lpBulletPlop[lpPlop->point].count)
					continue;
				/* ����������� ���� */
				lpPlop->point++;
				lpPlop->count = 0;
				/* �������� ����� �������� */
				if (lpBulletPlop[lpPlop->point].number < 0) {
					lpBullet->show = 0;
					continue;
				}
				/* �������� ��������� ���� */
				lpBullet->type = lpBulletPlop[lpPlop->point].number;
			}
		}
	}
}

/*
 * �������� ���������� ��������.
 *
 * �������� ������������ ��� ��������� ���������:
 * 1 - ������ ������. ���������� ������� ������
 *     ������ � �.�. (StartRaundAnima)
 * 2 - ��������� ������. ������������ ��� ����������
 *     ��������. (EndRaundAnima)
 * 3 - ��������� �����. ���������� ������ �� ������
 *     ����. (PauseEndAnima)
 * 4 - ������ �������� �������. ��������� ������ ��������
 *     �������. (FlySliverAnima)
 * 5 - ����� �� ������ (������� � ��������� �����). ���������
 *     ����� �� ������ � �������� ����� �� ����������
 *     �������. (FlyRocketAnima).
 */
static int MoveAnima(void)
{
	/* ���� ������ ���������� ��������, �������� ��� */
	if (anima.func != NULL)
		return (*anima.func)();

	return 0;
}

/*
 * ��������� ����������� ���� �����.
 *
 * ������������ ��������:
 * 0 - ���� ����.
 * 1 - ����� �������.
 * 2 - ����� ��������.
 */
int GameMoveScene(void)
{
	/* ��������� ����������� ������� */
	CheckControlKey();

	/* ���������� ����� */
	if (pause.pause)
		return 0;

	/*
	 * ������� ������� ��������� ��������.
	 *
	 * ������������ �������� �������� �������� � �������� �� ����.
	 * 0 - �������� ���������� ������, �.�. ���� �������
	 *     ����������� �����.
	 * 1 - �������� ���������� �� ����������� ������,
	 *     ����������� ����� �� �����������.
	 * 2 - ���������� ���������� '0'.
	 * 3 - ���������� ���������� '1'.
	 * 4 - ���������� ���������� '2'.
	 */
	switch (MoveAnima()) {
		/* �������� ���������� �� ����������� ������ */
		case 1: goto COPY_BLOCK;
		/* ��������� ���� (���� ����) */
		case 2: return 0;
		/* ��������� ������� (����� �������) */
		case 3: return 1;
		/* ��������� ������  (����� ��������) */
		case 4: return 2;
	}

	/* ���������� ���� ���������� */
	MoveSwerves();
	/* ���������� �������� �������� */
	MoveAlien();
	/* ���������� ����� */
	MoveBomb();
	/* ���������� �������� ���� */
	MoveBonus();
	/* ���������� ����� ��������� �������� */
	MoveBang();
	/* ���������� ������ */
	MoveBullet();
	/* ���������� ������� */
	MoveRacket();
	/* ���������� ����� � �������� ������������ */
	MoveBall();

COPY_BLOCK:
	/* �������� �������� ���� */
	scene.score.up1 = score.up1;
	scene.score.up2 = score.up2;
	scene.score.hi = score.hi;
	/* �������� ����� */
	scene.lifes = lifes;
	/* �������� ����� ��� ����������� */
	CopyBlock();

	/* ���� ����� ������� �� �������� �������� ��������� */
	if (GetCountBlock() == 0)
		SetEndRoundAnima();

	return 0;
}

/* �������� ������ */
void GameCreateLevel(int level)
{
	/* �������� ����� ������ */
	nLevel = level;

	/* ������� ����� */
	memset(&scene,0,sizeof(scene));

	/* ��������� ������� ����������� */
	scene.on = 1;
	scene.background = (level % LEVEL_COUNT) % BACKGROUND_COUNT;

	/* ������� ��������� �� ������� ����� */
	raund = levels[level%LEVEL_COUNT].round;

	/* �������� ���� ��������� */
	memset(block,0,sizeof(block));
	memcpy(&block[3][0],raund,sizeof(UCHAR)*(15*12));

	/* �������� ����� ���������� */
	SetSwerve();

	/* ������� ��������� �������� */
	memset(EventBlock,0,sizeof(EventBlock));
	memset(&racket,0,sizeof(racket));
	memset(ball,0,sizeof(ball));
	memset(&alien,0,sizeof(alien));
	memset(&bang,0,sizeof(bang));
	memset(&bomb,0,sizeof(bomb));
	memset(&bonus,0,sizeof(bonus));
	memset(&anima,0,sizeof(anima));
	memset(sliver,0,sizeof(sliver));

	/* ���� ��� ������ ����, �� ���������� ���� */
	if (!level) {
		scene.score.up1 = score.up1 = 0;
		scene.score.up2 = score.up2 = 0;
		score.addLifes = 30000;
		/* ���� ���� ������ ��������, �� 'hiscore' = 100000 */
		if (!score.hi) score.hi = 100000;
		scene.score.hi = score.hi;
		/* ��������� ����� */
		scene.lifes = lifes = LIFE_COUNT;
	} else {
		scene.score.up1 = score.up1;
		scene.score.up2 = score.up2;
		scene.score.hi = score.hi;
		/* ��������� ����� */
		scene.lifes = lifes;
	}

	/* �������� ����� ��� ����������� */
	CopyBlock();

	/* ��������� �������� ������ ������ */
	SetStartRoundAnima();
}
