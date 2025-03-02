/* sound.c */

/* включение guids */
#define INITGUID
/* версия 8.0 */
#define DIRECTSOUND_VERSION     0x0800  /* Version 8.0 */

#include "global.h"
#include <mmreg.h>
#include <dsound.h>
#include "sound.h"
#include "resource.h"

/* локальные переменные */
static LPDIRECTSOUND lpds;
static LPDIRECTSOUNDBUFFER lpdsBuff;

/* играть звук */
GAMEPLAY play;

/* воспроизвести звуки */
int GameSound(void)
{
	if (play.brik) {
		play.brik = 0;
		IDirectSoundBuffer_Play(lpdsBuff,0,0,0);
	}
	if (play.stone) {
		play.stone = 0;
		IDirectSoundBuffer_Play(lpdsBuff,0,0,0);
	}
	if (play.wall) {
		play.wall = 0;
//		IDirectSoundBuffer_Play(lpdsBuff,0,0,0);
	}
	if (play.bang) {
		play.bang = 0;
	}
	if (play.swerve) {
		play.swerve = 0;
	}
	if (play.destroy) {
		play.destroy = 0;
	}
	if (play.transform) {
		play.transform = 0;
	}
	if (play.bonus) {
		play.bonus = 0;
	}
	if (play.rocket) {
		play.rocket = 0;
	}

	return 0;
}

/* загрузим звук из ресурса в буфер */
static int LoadSample(void)
{
	DSBUFFERDESC dsbd;
	WAVEFORMATEX pcmwf;
	HRSRC hrSrc;
	HGLOBAL hRes;
	UCHAR *lpBuff,
		*audio_ptr_1 = NULL,
		*audio_ptr_2 = NULL;
	DWORD dwSize,
		audio_length_1 = 0,
		audio_length_2 = 0;

	/* найдем ресурс звукового файла */
	if ((hrSrc = FindResource(NULL,MAKEINTRESOURCE(IDR_WAVE1),_T("wave"))) == NULL)
		return 1;
	/* загрузим ресурс в память */
	if ((hRes = LoadResource(NULL,hrSrc)) == NULL)
		return 1;
	/* получим указатель на загруженный ресурс */
	if ((lpBuff = LockResource(hRes)) == NULL)
		return 1;
	/* получим размер звукового файла */
	dwSize = *((DWORD *)(lpBuff+0x28));

	/* создадим дополнительный аудиобуфер */
	memset(&pcmwf,0,sizeof(pcmwf));
	pcmwf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.nChannels = 1;
	pcmwf.nSamplesPerSec = 11025;
	pcmwf.nBlockAlign = 1;
	pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
	pcmwf.wBitsPerSample = 8;

	INIT_DIRECT_STRUCT(dsbd);
	dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE;
	dsbd.dwBufferBytes = dwSize;
	dsbd.lpwfxFormat = &pcmwf;
	if (FAILED(IDirectSound_CreateSoundBuffer(lpds,&dsbd,&lpdsBuff,NULL)))
		return 1;

	/* блокируем аудиобуфер */
	if (FAILED(IDirectSoundBuffer_Lock(lpdsBuff,0,dwSize,&audio_ptr_1,&audio_length_1,
		&audio_ptr_2,&audio_length_2,DSBLOCK_ENTIREBUFFER)))
		return 1;

	/* копируем первый сегмент */
	memcpy(audio_ptr_1,lpBuff+0x2c,audio_length_1);
	/* копируем второй сегмент */
	memcpy(audio_ptr_2,lpBuff+0x2c+audio_length_1,audio_length_2);

	/* разблокируем буфер */
	if (FAILED(IDirectSoundBuffer_Unlock(lpdsBuff,audio_ptr_1,audio_length_1,
		audio_ptr_2,audio_length_2)))
		return 1;

	return 0;
}

/* инициализация модуля */
int InitSound(void)
{
	/* получим интерфейс DirectSound */
	if (FAILED(DirectSoundCreate(NULL,&lpds,NULL)))
		return 1;
	/* установим уровень взаимодействия */
	if (FAILED(IDirectSound_SetCooperativeLevel(lpds,ghWndMain,DSSCL_NORMAL)))
		return 1;
	/* загрузим звук из ресурса в буфер */
	if (LoadSample())
		return 1;

	return 0;
}

/* деинициализация модуля */
void ReleaseSound(void)
{
	/* освободим аудиобуфер */
	if (lpdsBuff)
		IDirectSoundBuffer_Release(lpdsBuff);
	/* освободим интерфейс */
	if (lpds)
		IDirectSound_Release(lpds);
}
