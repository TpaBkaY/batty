/* draw.h */
#ifndef _DRAW_H_
#define _DRAW_H_

#include <windows.h>

/* хранит описание рисунка */
typedef struct {
	int show;                          /* флаг отоброжения */
	int type;                          /* тип рисунка */
	int X,Y;                           /* координаты */
} PICTURE, *LPPICTURE;

/* поверхность для рисования */
extern UINT *ptImage;
/* контекст для рисования в памяти */
extern HDC ghMemDC;

/* инициализация модуля рисования кадра */
int InitDraw(void);
/* деинициализация модуля рисования кадра */
void ReleaseDraw(void);

/* обновляет изображение (переключает буфер) */
void UpdateDraw(void);

#endif  /* _DRAW_H */
