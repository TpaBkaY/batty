/* sound.h */
#ifndef _SOUND_H_
#define _SOUND_H_

/* описывает звуки */
typedef struct {
	int brik;           /* попадает в кирпич */
	int stone;          /* попадает в двойной кирпич */
	int wall;           /* попадание в ракетку или стену */
	int bang;           /* попадает в летающее существо */
	int swerve;         /* переключение круга откланения */
	int destroy;        /* взрыв ракетки */
	int transform;      /* преобразование ракетки */
	int bonus;          /* поймали бонус */
	int rocket;         /* поймали ракету */
} GAMEPLAY;

/* инициализация модуля */
int InitSound(void);
/* деинициализация модуля */
void ReleaseSound(void);

/* воспроизвести звуки */
int GameSound(void);

extern GAMEPLAY play;

#endif  /* _SOUND_H_ */
