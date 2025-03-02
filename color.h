/* color.h */
#ifndef _COLOR_H_
#define _COLOR_H_

/* ������������ ������� */
#define RGB32BIT(r,g,b)   ((b)+((g)<<8)+((r)<<16))

/* ����� ��� ��������� ������� ������ */
/*
 * ��������� ��� ����� �������� ������, �.�. ��� ������
 * ������ � ������ �������� �����.
 */
#define CL_TRANSP                 1     /* ���������� */

#define CL_BLACK                  0     /* ������ */
#define CL_WHITE                  1     /* ����� */
#define CL_YELLOW                 2     /* ������ */
#define CL_ROSY                   3     /* ������� */
#define CL_RED                    4     /* ������� */
#define CL_BLUE                   5     /* ����� */
#define CL_GREEN                  6     /* ������� */

#define CL_YELLOW_BACK           11     /* ��� ������ */
#define CL_GREEN_BACK            12     /* ��� ������� */
#define CL_SKY_BACK              13     /* ��� ������� */
#define CL_WHITE_BACK            14     /* ��� ����� */
#define CL_SHADE_YELLOW_BACK     15     /* ��� ���� ������ */
#define CL_SHADE_GREEN_BACK      16     /* ��� ���� ������� */
#define CL_SHADE_SKY_BACK        17     /* ��� ���� ������� */
#define CL_SHADE_WHITE_BACK      18     /* ��� ���� ����� */

#define CL_SCREEN_BACK           19     /* ������ ��� ���� */

#define CL_MAX                   20     /* ���������� ������ */

/* ����� ��� ��������� �������� ���� */
#define CLM_BLACK                 0     /* ������ */
#define CLM_WHITE                 1     /* ����� */
#define CLM_SHADE_WHITE           2     /* ��������� ����� */
#define CLM_YELLOW                3     /* ������ */
#define CLM_SHADE_YELLOW          4     /* ��������� ������ */
#define CLM_ROSY                  5     /* ������� */
#define CLM_SHADE_ROSY            6     /* ��������� ������� */
#define CLM_RED                   7     /* ������� */
#define CLM_SHADE_RED             8     /* ��������� ������� */
#define CLM_BLUE                  9     /* ����� */
#define CLM_SHADE_BLUE           10     /* ��������� ����� */
#define CLM_GREEN                11     /* ������� */
#define CLM_SHADE_GREEN          12     /* ��������� ������� */

#define CLM_SCREEN_BACK          13     /* ������ ��� */

/*
 *  0 - 19 : ������������ ��� ��������� ����.
 * 20 - 29 : ������������ ��� ������� 'screen_first.bmp'.
 * 30 - 59 : ������������ ��� ������� 'screen_black.bmp'.
 * 60 - 79 : ������������ ��� ������� 'screen.bmp'.
 */
#define CLM_MAX                 100     /* ���������� ������ */

#endif  /* _COLOR_H_ */
