/* color.h */
#ifndef _COLOR_H_
#define _COLOR_H_

/* формирование пикселя */
#define RGB32BIT(r,g,b)   ((b)+((g)<<8)+((r)<<16))

/* Цвета для рисования игровых стадий */
/*
 * Следующие три цвета изменять нельзя, т.к. они жестко
 * зашиты в логику создания фигур.
 */
#define CL_TRANSP                 1     /* прозрачный */

#define CL_BLACK                  0     /* черный */
#define CL_WHITE                  1     /* белый */
#define CL_YELLOW                 2     /* желтый */
#define CL_ROSY                   3     /* розовый */
#define CL_RED                    4     /* красный */
#define CL_BLUE                   5     /* синий */
#define CL_GREEN                  6     /* зеленый */

#define CL_YELLOW_BACK           11     /* фон желтый */
#define CL_GREEN_BACK            12     /* фон зеленый */
#define CL_SKY_BACK              13     /* фон голубой */
#define CL_WHITE_BACK            14     /* фон белый */
#define CL_SHADE_YELLOW_BACK     15     /* фон тень желтый */
#define CL_SHADE_GREEN_BACK      16     /* фон тень зеленый */
#define CL_SHADE_SKY_BACK        17     /* фон тень голубой */
#define CL_SHADE_WHITE_BACK      18     /* фон тень белый */

#define CL_SCREEN_BACK           19     /* задний фон окна */

#define CL_MAX                   20     /* количество цветов */

/* цвета для рисования игрового меню */
#define CLM_BLACK                 0     /* черный */
#define CLM_WHITE                 1     /* белый */
#define CLM_SHADE_WHITE           2     /* затененый белый */
#define CLM_YELLOW                3     /* желтый */
#define CLM_SHADE_YELLOW          4     /* затененый желтый */
#define CLM_ROSY                  5     /* розовый */
#define CLM_SHADE_ROSY            6     /* затененый розовый */
#define CLM_RED                   7     /* красный */
#define CLM_SHADE_RED             8     /* затененый красный */
#define CLM_BLUE                  9     /* синий */
#define CLM_SHADE_BLUE           10     /* затененый синий */
#define CLM_GREEN                11     /* зеленый */
#define CLM_SHADE_GREEN          12     /* затененый зеленый */

#define CLM_SCREEN_BACK          13     /* задний фон */

/*
 *  0 - 19 : используется для рисования меню.
 * 20 - 29 : используется для рисунка 'screen_first.bmp'.
 * 30 - 59 : используется для рисунка 'screen_black.bmp'.
 * 60 - 79 : используется для рисунка 'screen.bmp'.
 */
#define CLM_MAX                 100     /* количество цветов */

#endif  /* _COLOR_H_ */
