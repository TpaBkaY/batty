/* draw.c */
#include "global.h"
#include <windows.h>
#include "draw.h"

/* поверхность для рисования */
UINT *ptImage;
/* контекст для рисования в памяти */
HDC ghMemDC;

/* BITMAP для рисования */
static HBITMAP hBmp,hOldBmp;

/* инициализация модуля рисования кадра */
int InitDraw(void)
{
	BITMAPINFO bmpInfo;
	HDC hdc;

	/* создадим контекст в памяти */
	if ((hdc = GetDC(ghWndMain)) == NULL)
		return 1;
	ghMemDC = CreateCompatibleDC(hdc);
	ReleaseDC(ghWndMain,hdc);
	if (!ghMemDC)
		return 1;

	/* создадим BITMAP */
	memset(&bmpInfo,0,sizeof(bmpInfo));
	bmpInfo.bmiHeader.biSize = sizeof(bmpInfo);
	bmpInfo.bmiHeader.biWidth = SCREEN_WIDTH;
	bmpInfo.bmiHeader.biHeight = -SCREEN_HEIGHT;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 32;
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	hBmp = CreateDIBSection(NULL,&bmpInfo,
		DIB_RGB_COLORS,(void **)&ptImage,NULL,0);
	if (hBmp == NULL || ptImage == NULL)
		return 1;

	/* присоединяем BITMAP к контексту */
	hOldBmp = SelectObject(ghMemDC,hBmp);
	if (!hOldBmp)
		return 1;

	/* установим параметры шрифта */
	SelectObject(ghMemDC,GetStockObject(ANSI_VAR_FONT));
	SetTextColor(ghMemDC,RGB(255,255,255));   /* белый цвет */
	SetBkColor(ghMemDC,RGB(0,0,60));          /* цвет заднего фона */

	return 0;
}

/* деинициализация модуля рисования кадра */
void ReleaseDraw(void)
{
	/* удаляем контекст в памяти */
	if (ghMemDC) {
		SelectObject(ghMemDC,hOldBmp);
		DeleteDC(ghMemDC);
		ghMemDC = NULL;
	}
	/* удаляем BITMAP */
	if (hBmp) {
		DeleteObject(hBmp);
		hBmp = NULL;
	}
}

/* копирует поверхность для рисования на первичную поверхность */
void UpdateDraw(void)
{
	HDC hdc;

	/* получим контекст клиентской области окна */
	if ((hdc = GetDC(ghWndMain)) == NULL)
		return;
	/* скопируем изображение */
	BitBlt(hdc,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ghMemDC,0,0,SRCCOPY);
	/* освободим контекст окна */
	ReleaseDC(ghWndMain,hdc);
}
