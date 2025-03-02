/* input.c */

/* включение guids */
#define INITGUID
/* используем 8-ю версию DirectInput */
#define DIRECTINPUT_VERSION     0x0800

/* включение заголовков */
#include "global.h"
#include <dinput.h>
#include "input.h"

/* описание ввода */
GAMEINPUT input;

/* локальные переменные */
static LPDIRECTINPUT8 lpdi;                /* основной интерфейс DirectInput */
static LPDIRECTINPUTDEVICE8 lpdiKey;       /* интерфейс клавиатуры */
static LPDIRECTINPUTDEVICE8 lpdiMouse;     /* интерфейс мыши */

/* флаги сигнализирующие о нажатии клавиши */
static struct {
	int p;
} flags;

/* проверка нажатие кнопки */
#define DIKEYDOWN(data,n)      ((data)[(n)] & 0x80)

/* получить ввод данных */
int GameInput(void)
{
	UCHAR keyState[256];
	DIMOUSESTATE mouseState;
	HRESULT result;
	BOOL bExit = 0;

	/* очищаем структуру ввода */
	memset(&input,0,sizeof(input));

	/* получим данные клавиатуры */
	result = IDirectInputDevice8_GetDeviceState(lpdiKey,
		sizeof(keyState),keyState);
	/* клавиатура потеряла ввод */
	if (result == DIERR_INPUTLOST) {
		IDirectInputDevice8_Acquire(lpdiKey);
		bExit = 1;
	} else if (FAILED(result))
		bExit = 1;
	/* получим данные мыши */
	result = IDirectInputDevice8_GetDeviceState(lpdiMouse,
		sizeof(DIMOUSESTATE),&mouseState);
	/* мышь потеряла ввод */
	if (result == DIERR_INPUTLOST) {
		IDirectInputDevice8_Acquire(lpdiMouse);
		bExit = 1;
	} else if (FAILED(result))
		bExit = 1;

	/* проверим ошибку */
	if (bExit)
		return 1;

	/* обрабатываем события клавиатуры */
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

	/* обрабатываем события мыши */
	if (mouseState.rgbButtons[0]) input.lButton = 1;
	input.x = mouseState.lX;
	input.y = mouseState.lY;

	return 0;
}

/* захватить устройства ввода */
int AcquireInput(BOOL flags)
{
	if (flags) {
		/* получаем клавиатуру */
		if (lpdiKey) {
			if (FAILED(IDirectInputDevice8_Acquire(lpdiKey)))
				return 1;
		}
		/* захватываем мышь */
		if (lpdiMouse) {
			if (FAILED(IDirectInputDevice8_Acquire(lpdiMouse)))
				return 1;
		}
	} else {
		/* возвратим клавиатуру */
		if (lpdiKey)
			IDirectInputDevice8_Unacquire(lpdiKey);
		/* возвратим мышь */
		if (lpdiMouse)
			IDirectInputDevice8_Unacquire(lpdiMouse);
	}

	return 0;
}

/* инициализация модуля ввода данных */
int InitInput(void)
{
	/* создание интерфейса DirectInput */
	if (FAILED(DirectInput8Create(ghInst,DIRECTINPUT_VERSION,
		&IID_IDirectInput8,&lpdi,NULL)))
		return 1;

	/* создание устройства клавиатуры */
	if (FAILED(IDirectInput8_CreateDevice(lpdi,&GUID_SysKeyboard,
		&lpdiKey,NULL)))
		return 1;
	/* устанавливаем уровень взаимодействия для клавиатуры */
	if (FAILED(IDirectInputDevice8_SetCooperativeLevel(lpdiKey,
		ghWndMain,DISCL_FOREGROUND|DISCL_NONEXCLUSIVE)))
		return 1;
	/* определяем формат данных */
	if (FAILED(IDirectInputDevice8_SetDataFormat(lpdiKey,&c_dfDIKeyboard)))
		return 1;

	/* создаем устройство мыши */
	if (FAILED(IDirectInput8_CreateDevice(lpdi,&GUID_SysMouse,
		&lpdiMouse,NULL)))
		return 1;
	/* задаем уровень взаимодействия для мыши */
	if (FAILED(IDirectInputDevice8_SetCooperativeLevel(lpdiMouse,
		ghWndMain,DISCL_FOREGROUND|DISCL_EXCLUSIVE)))
		return 1;
	/* задаем формат данных */
	if (FAILED(IDirectInputDevice8_SetDataFormat(lpdiMouse,&c_dfDIMouse)))
		return 1;

	/* захватим устройства ввода */
	AcquireInput(TRUE);

	return 0;
}

/* освобождение модуля ввода данных */
void ReleaseInput(void)
{
	/* освободим устройства ввода */
	AcquireInput(FALSE);

	/* возвратим клавиатуру */
	if (lpdiKey) {
		IDirectInputDevice8_Release(lpdiKey);
		lpdiKey = NULL;
	}
	/* возвратим мышь */
	if (lpdiMouse) {
		IDirectInputDevice8_Release(lpdiMouse);
		lpdiMouse = NULL;
	}

	/* освободим интерфейс DirectInput */
	if (lpdi) {
		IDirectInput8_Release(lpdi);
		lpdi = NULL;
	}
}
