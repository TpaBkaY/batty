/* draw.c */
#include "global.h"
#include <windows.h>
#include "draw.h"

/* ����������� ��� ��������� */
UINT *ptImage;
/* �������� ��� ��������� � ������ */
HDC ghMemDC;

/* BITMAP ��� ��������� */
static HBITMAP hBmp,hOldBmp;

/* ������������� ������ ��������� ����� */
int InitDraw(void)
{
	BITMAPINFO bmpInfo;
	HDC hdc;

	/* �������� �������� � ������ */
	if ((hdc = GetDC(ghWndMain)) == NULL)
		return 1;
	ghMemDC = CreateCompatibleDC(hdc);
	ReleaseDC(ghWndMain,hdc);
	if (!ghMemDC)
		return 1;

	/* �������� BITMAP */
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

	/* ������������ BITMAP � ��������� */
	hOldBmp = SelectObject(ghMemDC,hBmp);
	if (!hOldBmp)
		return 1;

	/* ��������� ��������� ������ */
	SelectObject(ghMemDC,GetStockObject(ANSI_VAR_FONT));
	SetTextColor(ghMemDC,RGB(255,255,255));   /* ����� ���� */
	SetBkColor(ghMemDC,RGB(0,0,60));          /* ���� ������� ���� */

	return 0;
}

/* ��������������� ������ ��������� ����� */
void ReleaseDraw(void)
{
	/* ������� �������� � ������ */
	if (ghMemDC) {
		SelectObject(ghMemDC,hOldBmp);
		DeleteDC(ghMemDC);
		ghMemDC = NULL;
	}
	/* ������� BITMAP */
	if (hBmp) {
		DeleteObject(hBmp);
		hBmp = NULL;
	}
}

/* �������� ����������� ��� ��������� �� ��������� ����������� */
void UpdateDraw(void)
{
	HDC hdc;

	/* ������� �������� ���������� ������� ���� */
	if ((hdc = GetDC(ghWndMain)) == NULL)
		return;
	/* ��������� ����������� */
	BitBlt(hdc,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ghMemDC,0,0,SRCCOPY);
	/* ��������� �������� ���� */
	ReleaseDC(ghWndMain,hdc);
}
