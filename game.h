/* game.h */
#ifndef _GAME_H_
#define _GAME_H_

/* инициализация игры */
int InitGame(void);
/* основной цикл игры */
void MainGame(void);
/* закрытие игры */
void ShutdownGame(void);

/* среднее количество кадров в секунду */
extern int giFps;
/* среднее время построения одного кадра */
extern int giFrame;

#endif  /* _GAME_H_ */
