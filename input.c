/* input.c */

/* ��������� guids */
#define INITGUID
/* ���������� 8-� ������ DirectInput */
#define DIRECTINPUT_VERSION     0x0800

/* ��������� ���������� */
#include "global.h"
#include <dinput.h>
#include "input.h"

/* �������� ����� */
GAMEINPUT input;

/* ��������� ���������� */
static LPDIRECTINPUT8 lpdi;                /* �������� ��������� DirectInput */
static LPDIRECTINPUTDEVICE8 lpdiKey;       /* ��������� ���������� */
static LPDIRECTINPUTDEVICE8 lpdiMouse;     /* ��������� ���� */

/* ����� ��������������� � ������� ������� */
static struct {
	int p;
} flags;

/* �������� ������� ������ */
#define DIKEYDOWN(data,n)      ((data)[(n)] & 0x80)

/* �������� ���� ������ */
int GameInput(void)
{
	UCHAR keyState[256];
	DIMOUSESTATE mouseState;
	HRESULT result;
	BOOL bExit = 0;

	/* ������� ��������� ����� */
	memset(&input,0,sizeof(input));

	/* ������� ������ ���������� */
	result = IDirectInputDevice8_GetDeviceState(lpdiKey,
		sizeof(keyState),keyState);
	/* ���������� �������� ���� */
	if (result == DIERR_INPUTLOST) {
		IDirectInputDevice8_Acquire(lpdiKey);
		bExit = 1;
	} else if (FAILED(result))
		bExit = 1;
	/* ������� ������ ���� */
	result = IDirectInputDevice8_GetDeviceState(lpdiMouse,
		sizeof(DIMOUSESTATE),&mouseState);
	/* ���� �������� ���� */
	if (result == DIERR_INPUTLOST) {
		IDirectInputDevice8_Acquire(lpdiMouse);
		bExit = 1;
	} else if (FAILED(result))
		bExit = 1;

	/* �������� ������ */
	if (bExit)
		return 1;

	/* ������������ ������� ���������� */
	if (DIKEYDOWN(keyState,DIK_K)) input.K = 1;
	if (DIKEYDOWN(keyState,DIK_L)) input.L = 1;
	if (DIKEYDOWN(keyState,DIK_SPACE)) input.SPACE = 1;
	if (DIKEYDOWN(keyState,DIK_P)) {
		if (!flags.p) {
			input.P = 1;
			flags.p = 1;
		}
	} else {
		flags.p = 0;
	}

	/* ������������ ������� ���� */
	if (mouseState.rgbButtons[0]) input.lButton = 1;
	input.x = mouseState.lX;
	input.y = mouseState.lY;

	return 0;
}

/* ��������� ���������� ����� */
int AcquireInput(BOOL flags)
{
	if (flags) {
		/* �������� ���������� */
		if (lpdiKey) {
			if (FAILED(IDirectInputDevice8_Acquire(lpdiKey)))
				return 1;
		}
		/* ����������� ���� */
		if (lpdiMouse) {
			if (FAILED(IDirectInputDevice8_Acquire(lpdiMouse)))
				return 1;
		}
	} else {
		/* ��������� ���������� */
		if (lpdiKey)
			IDirectInputDevice8_Unacquire(lpdiKey);
		/* ��������� ���� */
		if (lpdiMouse)
			IDirectInputDevice8_Unacquire(lpdiMouse);
	}

	return 0;
}

/* ������������� ������ ����� ������ */
int InitInput(void)
{
	/* �������� ���������� DirectInput */
	if (FAILED(DirectInput8Create(ghInst,DIRECTINPUT_VERSION,
		&IID_IDirectInput8,&lpdi,NULL)))
		return 1;

	/* �������� ���������� ���������� */
	if (FAILED(IDirectInput8_CreateDevice(lpdi,&GUID_SysKeyboard,
		&lpdiKey,NULL)))
		return 1;
	/* ������������� ������� �������������� ��� ���������� */
	if (FAILED(IDirectInputDevice8_SetCooperativeLevel(lpdiKey,
		ghWndMain,DISCL_FOREGROUND|DISCL_NONEXCLUSIVE)))
		return 1;
	/* ���������� ������ ������ */
	if (FAILED(IDirectInputDevice8_SetDataFormat(lpdiKey,&c_dfDIKeyboard)))
		return 1;

	/* ������� ���������� ���� */
	if (FAILED(IDirectInput8_CreateDevice(lpdi,&GUID_SysMouse,
		&lpdiMouse,NULL)))
		return 1;
	/* ������ ������� �������������� ��� ���� */
	if (FAILED(IDirectInputDevice8_SetCooperativeLevel(lpdiMouse,
		ghWndMain,DISCL_FOREGROUND|DISCL_EXCLUSIVE)))
		return 1;
	/* ������ ������ ������ */
	if (FAILED(IDirectInputDevice8_SetDataFormat(lpdiMouse,&c_dfDIMouse)))
		return 1;

	/* �������� ���������� ����� */
	AcquireInput(TRUE);

	return 0;
}

/* ������������ ������ ����� ������ */
void ReleaseInput(void)
{
	/* ��������� ���������� ����� */
	AcquireInput(FALSE);

	/* ��������� ���������� */
	if (lpdiKey) {
		IDirectInputDevice8_Release(lpdiKey);
		lpdiKey = NULL;
	}
	/* ��������� ���� */
	if (lpdiMouse) {
		IDirectInputDevice8_Release(lpdiMouse);
		lpdiMouse = NULL;
	}

	/* ��������� ��������� DirectInput */
	if (lpdi) {
		IDirectInput8_Release(lpdi);
		lpdi = NULL;
	}
}
