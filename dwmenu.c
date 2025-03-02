/* dwmenu.c */
#include "global.h"
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "dwmenu.h"
#include "game.h"
#include "color.h"
#include "resource.h"

/* рисунок курсора (12,17) */
static const UCHAR pCursor[] = {
	0,1,1,1,1,1,1,1,1,1,1,1,    /* 1 */
	0,0,1,1,1,1,1,1,1,1,1,1,    /* 2 */
	0,2,0,1,1,1,1,1,1,1,1,1,    /* 3 */
	0,2,2,0,1,1,1,1,1,1,1,1,    /* 4 */
	0,2,2,2,0,1,1,1,1,1,1,1,    /* 5 */
	0,2,2,2,2,0,1,1,1,1,1,1,    /* 6 */
	0,2,2,2,2,2,0,1,1,1,1,1,    /* 7 */
	0,2,2,2,2,2,2,0,1,1,1,1,    /* 8 */
	0,2,2,2,2,2,2,2,0,1,1,1,    /* 9 */
	0,2,2,2,2,2,2,2,2,0,1,1,    /* 10 */
	0,2,2,2,2,2,2,2,2,2,0,1,    /* 11 */
	0,2,2,2,2,0,0,0,0,0,0,0,    /* 12 */
	0,2,2,2,0,1,1,1,1,1,1,1,    /* 13 */
	0,2,2,0,1,1,1,1,1,1,1,1,    /* 14 */
	0,2,0,1,1,1,1,1,1,1,1,1,    /* 15 */
	0,0,1,1,1,1,1,1,1,1,1,1,    /* 16 */
	0,1,1,1,1,1,1,1,1,1,1,1     /* 17 */
};

/* описывает загруженый из ресурса bmp-рисунок */
typedef struct {
	const void *bmp;          /* указатель на начало bmp-рисунка */
	const UCHAR *picture;     /* указатель на картинку */
	int pitch;                /* шаг памяти изображения */
	int width, height;        /* ширина и высота изображения */
} BMP, *LPBMP;

/* структура описывающая меню */
MENUSCENE menu;

/* строка форматирования отладочного сообщения */
static const TCHAR sFormatDebug[] = _T("fps %02d; frame %02d; draw %02d;");
/* координаты для отображения дебажного сообщения */
static DWORD dwDebugX,dwDebugY;

/* переменные для расчета скорости заполнения сцены */
static int lSpeedBuff[TIME_BUFF];
static int pSpeed;
static int lSpeedDraw;

/*
 * Содержит рисунки букв латинского алфавита.
 * Размер одной буквы 8x14 и 26 букв в латинском алфавите.
 */
#define BMP_ABC_WIDTH       (8 * 26)
#define BMP_ABC_HEIGHT           14
static BMP bmpAbc;
/*
 * Содержит рисунки цифр.
 * Размер одной цыфры 8x14, всего 10 цифр.
 */
#define BMP_DIGIT_WIDTH     (8 * 10)
#define BMP_DIGIT_HEIGHT         14
static BMP bmpDigit;
/*
 * Содержит рисунок служебных символов.
 * Размер оного символа 8x14, всего 2 символа.
 */
#define BMP_SYMBOL_WIDTH     (8 * 2)
#define BMP_SYMBOL_HEIGHT        14
static BMP bmpSymbol;
/*
 * Содержит рисунки заставоки 256x192.
 */
#define BMP_SCREEN_WIDTH   256
#define BMP_SCREEN_HEIGHT  192
static BMP bmpScreen;          /* первый номер палитры 60, количество 12 */
static BMP bmpScreenBlack;     /* первый номер палитры 30, количество 23 */
static BMP bmpScreenFirst;     /* первый номер палитры 20, количество 3 */

/* содержит используемые цвета */
static UINT palette[CLM_MAX];

/* рисует горизонтальную линию */
static void HLineMenu(int x1, int x2, int y, int color)
{
	int length, i;
	UINT pixel,*v1,*v2;

	/* сделаем чтобы x2 > x1 */
	if (x1 > x2) {
		unsigned int temp = x1;
		x1 = x2;
		x2 = temp;
	}

	/* расчитаем цвет линии */
	pixel = palette[color];
	/* расчитаем длину линии */
	length = x2 - x1 + (1 * 2);
	/* нарисуем линию */
	v1 = ptImage + SCREEN_WIDTH * y + x1;
	v2 = v1 + SCREEN_WIDTH;
	for (i = 0; i < length; i++) {
		v1[i] = pixel;
		v2[i] = pixel;
	}
}

/* рисует вертикальную линию */
static void VLineMenu(int y1, int y2, int x, int color)
{
	int i,length;
	UINT pixel,*v1,*v2;

	/* сделаем чтобы y2 > y1 */
	if (y1 > y2) {
		int temp = y1;
		y1 = y2;
		y2 = temp;
	}

	/* расчитаем служебные переменные */
	v1 = ptImage + SCREEN_WIDTH * y1 + x;
	v2 = v1 + 1;
	length = y2 - y1 + 1;
	pixel = palette[color];
	/* нарисуем линию */
	for (i = 0; i <= length; i++) {
		/* отобразим точки */
		*v1 = pixel;
		*v2 = pixel;
		/* увеличим координаты */
		v1 += SCREEN_WIDTH;
		v2 += SCREEN_WIDTH;
	}
}

/*
 * Рисует одну букву или цифру.
 * Экран имеет 6 строчек по 16 символов.
 */
static void PlotLetterScale(char letter, int txColor,
					   int bcColor, unsigned int x, unsigned int y)
{
	const BMP *lpBmp;
	const UCHAR *pattern;
	UINT *video;
	int i,j,firstSymbol;

	/* проверим выход за пределы */
	if ((x >= 32) || (y >= 12))
		return;

	/* определим шаблон с изображением */
	if ((letter >= 'a') && (letter <= 'z')) {
		lpBmp = &bmpAbc;
		firstSymbol = 'a';
	} else if ((letter >= '0') && (letter <= '9')) {
		lpBmp = &bmpDigit;
		firstSymbol = '0';
	} else if (letter == ':') {
		lpBmp = &bmpSymbol;
		firstSymbol = ':';
	} else if (letter == '-') {
		lpBmp = &bmpSymbol;
		firstSymbol = ',';
	} else
		return;

	/* получим цвета */
	txColor = palette[txColor];
	bcColor = palette[bcColor];

	/* отобразим букву, занимаемый размер буквы равен 8x16 */
	video = ptImage + GetY(y * 16 + 1) * SCREEN_WIDTH  + GetX(x * 8);
	pattern = lpBmp->picture + (letter - firstSymbol) * 8;
	for (i = 0; i < 14; i++) {
		for (j = 0; j < 8; j++) {
			int color;
			UINT *p;
			/* получим цвет отображаемого пикселя */
			color = pattern[j] ? bcColor : txColor;
			/* получим точку на поверхности */
			p = video + (j * SCALE);
			/* отобразим точку */
			*(p) = color;
			*(p + 1) = color;
			*(p + SCREEN_WIDTH) = color;
			*(p + SCREEN_WIDTH + 1) = color;
		}
		video += SCREEN_WIDTH * SCALE;
		pattern += lpBmp->pitch;
	}
}

/* отоброжает текст */
static void PlotText(char *string, int txColor,
					 int bcColor, int x, int y)
{
	int i,len;

	/* отобразим строку */
	len = strlen(string);
	for (i = 0; i < len; i++)
		PlotLetterScale(string[i],txColor,bcColor,x + i,y);
}

/* рисует поверхности с учетом прозрачности */
static void PlotCursor(const UCHAR *pattern, int pitchPattern,
							int width, int height, int x, int y)
{
	UINT *video,background,black;
	int i,j;

	/* получим цвета */
	background = palette[CLM_GREEN];
	black = palette[CLM_WHITE];
	/* расчитаем точку от которой начинается прорисовка поверхности */
	video = ptImage + y * SCREEN_WIDTH + x;

	/* отобразим поверхность */
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			UCHAR pixel;
			UINT *p;
			int py,px;

			/* проверим, что точка не пересекла крайнии линии игрового окна */
			py = y + i;
			if ((py >= GAME_Y + GAME_HEIGHT) || (py < GAME_Y))
				continue;
			px = x + j;
			if ((px < GAME_X) || (px >= GAME_X + GAME_WIDTH))
				continue;

			/* прозрачный пиксель */
			pixel = pattern[j];
			if (pixel == CL_TRANSP)
				continue;
			/* получим точку на поле */
			p = video + j;
			/* отобразим точку */
			if (pixel) {
				/* цвет поверхности */
				*(p) = background;
			} else {
				/* видимая точка поверхности */
				*(p) = black;
			}
		}
		/* периместим поверхности на следующие строчки */
		video += SCREEN_WIDTH;
		pattern += pitchPattern;
	}
}

/* отображает курсор */
static void ShapeCursorMenu(void)
{
	/* отобразим курсор */
	if (menu.pCursor.show) {
		PlotCursor(pCursor,12,12,17,
			GetX(menu.pCursor.X),GetY(menu.pCursor.Y));
	}
}

/* производит заливку игровой поверхности */
static void ShapeFillScreenMenu(void)
{
	UINT *video,pixel;
	int x,y;

	/* зальем задний фон */
	video = ptImage;
	pixel = palette[CLM_SCREEN_BACK];
	for (y = 0; y < SCREEN_HEIGHT; y++, video += SCREEN_WIDTH) {
		for (x = 0; x < SCREEN_WIDTH; x++)
			video[x] = pixel;
	}
}

/* отобразить отладочную информацию */
static void ShapeDebugMenu(void)
{
	TCHAR sBuff[256];

	/* сформируем отображаемую строку */
	_stprintf(sBuff,sFormatDebug,giFps,giFrame,lSpeedDraw);
	TextOut(ghMemDC,dwDebugX,dwDebugY,sBuff,lstrlen(sBuff));
}

/* отображает ограждение */
static void ShapeBarrierMenu(void)
{
	/* левая стенка */
	VLineMenu(GetY(0),GetY(WIN_GAME_HEIGHT-1),
		GetX(0),CLM_SHADE_ROSY);
	VLineMenu(GetY(1),GetY(WIN_GAME_HEIGHT-2),
		GetX(1),CLM_SHADE_ROSY);
	/* правая стенка */
	VLineMenu(GetY(0),GetY(WIN_GAME_HEIGHT-1),
		GetX(WIN_GAME_WIDTH-1),CLM_SHADE_ROSY);
	VLineMenu(GetY(1),GetY(WIN_GAME_HEIGHT-2),
		GetX(WIN_GAME_WIDTH-2),CLM_SHADE_ROSY);
	/* верхняя стенка */
	HLineMenu(GetX(0),GetX(WIN_GAME_WIDTH-1),
		GetY(0),CLM_SHADE_ROSY);
	HLineMenu(GetX(1),GetX(WIN_GAME_WIDTH-2),
		GetY(1),CLM_SHADE_ROSY);
	/* нижняя стенка */
	HLineMenu(GetX(0),GetX(WIN_GAME_WIDTH-1),
		GetY(WIN_GAME_HEIGHT-1),CLM_SHADE_ROSY);
	HLineMenu(GetX(1),GetX(WIN_GAME_WIDTH-2),
		GetY(WIN_GAME_HEIGHT-2),CLM_SHADE_ROSY);
}

/* нарисовать очередной кадр меню */
int MenuDrawScene(void)
{
	DWORD dwTime;

	/* получим время для начала отсчета */
	if (giDebug)
		dwTime = timeGetTime();

	/* очистим поверхность (заливаем цветом фона) */
	ShapeFillScreenMenu();

	PlotText("1 - 1 player",CLM_YELLOW,CLM_SCREEN_BACK,9,2);
	PlotText("2 - 2 players",CLM_YELLOW,CLM_SCREEN_BACK,9,3);
	PlotText("3 - double play",CLM_YELLOW,CLM_SCREEN_BACK,9,4);
	PlotText("0 - start game",CLM_YELLOW,CLM_SCREEN_BACK,9,6);
/*	PlotText("wtart jame 12345 zxcmfjkrrrtlbmw",CLM_YELLOW,CLM_SCREEN_BACK,0,1);
	PlotText("wtart jame 12345 zxcmfjkrrrtlbmw",CLM_YELLOW,CLM_SCREEN_BACK,0,2);
//	PlotText("wtart jame 12345 zxcmfjkrrrtlbmj",CLM_YELLOW,CLM_SCREEN_BACK,0,3);
	PlotText("start game 12345 zxcmfjkrrrtlbmw",CLM_YELLOW,CLM_SCREEN_BACK,0,4);
	PlotText("start game 12345 zxcmfjkrrrtlbmj",CLM_YELLOW,CLM_SCREEN_BACK,0,5);
	PlotText("start game 12345 zxcmfjkrrrtlbmj",CLM_YELLOW,CLM_SCREEN_BACK,0,6);
	PlotText("start game 12345 zxcmfjkrrrtlbmj",CLM_YELLOW,CLM_SCREEN_BACK,0,7);
	PlotText("start game 12345 zxcmfjkrrrtlbmj",CLM_YELLOW,CLM_SCREEN_BACK,0,8);
	PlotText("start game 12345 zxcmfjkrrrtlbmj",CLM_YELLOW,CLM_SCREEN_BACK,0,9);
	PlotText("start game 12345 zxcmfjkrrrtlbmj",CLM_YELLOW,CLM_SCREEN_BACK,0,10);
	PlotText("start game 12345 zxcmfjkrrrtlbmj",CLM_YELLOW,CLM_SCREEN_BACK,0,11);
	PlotText("start game 12345 zxcmfjkrrrtlbmjdd",CLM_YELLOW,CLM_SCREEN_BACK,0,12);
*/
	/* отображает курсор */
	ShapeCursorMenu();
	/* отобразим ограждение */
	ShapeBarrierMenu();

	/* расчитаем время заполнения сцены */
	if (giDebug) {
		lSpeedBuff[pSpeed] = timeGetTime() - dwTime;
		if (++pSpeed >= TIME_BUFF) {
			int i,sum;
			for (i = sum = 0; i < TIME_BUFF; i++)
				sum += lSpeedBuff[i];
			lSpeedDraw = sum / TIME_BUFF;
			pSpeed = 0;
		}
		ShapeDebugMenu();
	}

	return 0;
}

/* подготовка модуля рисования меню */
int MenuInitDraw(void)
{
	return 0;
}

/*
 * Ищет и загружает bmp-рисунок из ресурса.
 * Если рисунок в bmp-файле строиться снизу в верх, то
 * после загрузки он переворачивается.
 * Если переданные ширина и высота не соответствую анналогичным
 * параметрам храняшимся в описании BMP-файла, то функция
 * возвращает ошибку.
 */
static int LoadBmp(int id, LPBMP lpBmp, int width, int height)
{
	HRSRC hrSrc;
	HGLOBAL hRes;
	const BITMAPFILEHEADER *lpBmpFile;
	const BITMAPINFOHEADER *lpBmpInfo;

	/* найдем запрашиваемый ресурс */
	if ((hrSrc = FindResource(NULL,MAKEINTRESOURCE(id),_T("bmp"))) == NULL)
		return 0;
	/* загружаем ресурс в память */
	if ((hRes = LoadResource(NULL,hrSrc)) == NULL)
		return 0;
	/* получим указатель на загруженный ресурс */
	if ((lpBmp->bmp = LockResource(hRes)) == NULL)
		return 0;

	/* сформируем служебные указатели */
	lpBmpFile = lpBmp->bmp;
	lpBmpInfo = (void *)((UCHAR *)lpBmp->bmp + sizeof(*lpBmpFile));

	/* проверим все параметры */
	if (lpBmpFile->bfType != 0x4d42)
		return 0;
	if ((abs(lpBmpInfo->biHeight) != height) ||
		(lpBmpInfo->biWidth != width))
		return 0;

	/* сформируем переменные описывающие изображение */
	lpBmp->picture = (UCHAR *)lpBmp->bmp + lpBmpFile->bfOffBits;
	lpBmp->pitch = lpBmpInfo->biSizeImage / abs(lpBmpInfo->biHeight);
	lpBmp->width = width;
	lpBmp->height = height;

	/* если изображение строиться снизу вверх то перевернем его */
	if (lpBmpInfo->biHeight > 0) {
		const UCHAR *from;
		UCHAR *to, *buff;
		int i;

		/* выделим место под промежуточный буфер */
		buff = malloc(lpBmp->pitch * height);
		if (!buff)
			return 0;

		/* перевернем изображение */
		from = lpBmp->picture + lpBmp->pitch * (height - 1);
		to = buff;
		for (i = 0; i < height; i++) {
			memcpy(to,from,lpBmp->pitch);
			from -= lpBmp->pitch;
			to += lpBmp->pitch;
		}
		/* перенесем изображение обратно */
		from = buff;
		to = (UCHAR *)lpBmp->picture;
		for (i = 0; i < height; i++) {
			memcpy(to,from,lpBmp->pitch);
			from += lpBmp->pitch;
			to += lpBmp->pitch;
		}

		free(buff);
	}

	return 1;
}

/* начальная инициализация модуля рисования игрового меню */
int InitDrawMenu(void)
{
	const RGBQUAD *lpRgb;
	int i;

	/* получим параметры шрифта */
	if (giDebug) {
		TEXTMETRIC tm;
		/* расчитаем положение для дебажного сообщения */
		GetTextMetrics(ghMemDC,&tm);
		dwDebugX = GAME_X + 2 + (GAME_WIDTH - _tcslen(sFormatDebug) *
			(tm.tmAveCharWidth - 1)) / 2;
		dwDebugY = GAME_Y - (tm.tmHeight + 4);
	}

	/* загрузим буквы латинского алфавита */
	if (!LoadBmp(IDR_BMP1,&bmpAbc,BMP_ABC_WIDTH,BMP_ABC_HEIGHT))
		return 1;
	/* загрузим цифры */
	if (!LoadBmp(IDR_BMP2,&bmpDigit,BMP_DIGIT_WIDTH,BMP_DIGIT_HEIGHT))
		return 1;
	/* загрузим служебные символы */
	if (!LoadBmp(IDR_BMP6,&bmpSymbol,BMP_SYMBOL_WIDTH,BMP_SYMBOL_HEIGHT))
		return 1;
	/* загрузим основной рисунок заставки */
	if (!LoadBmp(IDR_BMP3,&bmpScreen,BMP_SCREEN_WIDTH,BMP_SCREEN_HEIGHT))
		return 1;
	/* загрузим черно-белый рисунок заставки */
	if (!LoadBmp(IDR_BMP4,&bmpScreenBlack,BMP_SCREEN_WIDTH,BMP_SCREEN_HEIGHT))
		return 1;
	/* загрузим начальный рисунок заставки */
	if (!LoadBmp(IDR_BMP5,&bmpScreenFirst,BMP_SCREEN_WIDTH,BMP_SCREEN_HEIGHT))
		return 1;

	/* сформируем палитру */
	memset(palette,0,sizeof(palette));

	/* заполним палитру цветами */
	palette[CLM_BLACK] = RGB32BIT(0,0,0);                  /* черный */
	palette[CLM_WHITE] = RGB32BIT(248,248,248);            /* белый */
	palette[CLM_SHADE_WHITE] = RGB32BIT(200,200,200);      /* затененый белый */
	palette[CLM_YELLOW] = RGB32BIT(248,248,0);             /* желтый */
	palette[CLM_SHADE_YELLOW] = RGB32BIT(200,200,0);       /* затененый желтый */
	palette[CLM_ROSY] = RGB32BIT(248,0,248);               /* розовый */
	palette[CLM_SHADE_ROSY] = RGB32BIT(200,0,200);         /* затененый розовый */
	palette[CLM_RED] = RGB32BIT(248,0,0);                  /* красный */
	palette[CLM_SHADE_RED] = RGB32BIT(200,0,0);            /* затененый красный */
	palette[CLM_BLUE] = RGB32BIT(0,0,248);                 /* синий */
	palette[CLM_SHADE_BLUE] = RGB32BIT(0,0,200);           /* затененый синий */
	palette[CLM_GREEN] = RGB32BIT(0,248,0);                /* зеленый */
	palette[CLM_SHADE_GREEN] = RGB32BIT(0,200,0);          /* затененый зеленый */
	palette[CLM_SCREEN_BACK] = RGB32BIT(0,0,60);           /* задний фон */

	/* перенесем палитру рисунка начальной заставки */
	lpRgb = (void *)((UCHAR *)bmpScreenFirst.bmp +
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
	for (i = 20; i < 20 + 3; i++)
		palette[i] = RGB32BIT(lpRgb[i].rgbRed,lpRgb[i].rgbGreen,lpRgb[i].rgbBlue);
	/* перенесем палитру черно-белого рисунка заставки */
	lpRgb = (void *)((UCHAR *)bmpScreenBlack.bmp + 
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
	for (i = 30; i < 30 + 23; i++)
		palette[i] = RGB32BIT(lpRgb[i].rgbRed,lpRgb[i].rgbGreen,lpRgb[i].rgbBlue);
	/* перенесем палитру рисунка заставки */
	lpRgb = (void *)((UCHAR *)bmpScreen.bmp +
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
	for (i = 60; i < 60 + 12; i++)
		palette[i] = RGB32BIT(lpRgb[i].rgbRed,lpRgb[i].rgbGreen,lpRgb[i].rgbBlue);

	return 0;
}

/* деинициализация модуля рисования игрового меню */
void ReleaseDrawMenu(void)
{
}
