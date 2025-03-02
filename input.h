/* input.h */
#ifndef _INPUT_H_
#define _INPUT_H_

#include <windows.h>

/* описывает произведенный ввод */
typedef struct {
	/* описывает нажатие клавишь */
	int K;          /* переместить ракетку в лево */
	int L;          /* в право */
	int SPACE;      /* стрелять */
	int P;
	/* описывает перемещение мыши */
	int x,y;          /* перемещение ракетки */
	/* описывает левую кнопку мыши */
	int lButton;    /* стрелять */
} GAMEINPUT;

/* инициализация модуля ввода данных */
int InitInput(void);
/* освобождение модуля ввода данных */
void ReleaseInput(void);

/* получить ввод данных */
int GameInput(void);
/* захватить устройства ввода */
int AcquireInput(BOOL flags);

/* описание ввода */
extern GAMEINPUT input;

#endif  /* _INPUT_H_ */
