#include <nds.h>
#include "drv_nds_hw7.h"

static NDS_HW_IPC* ipc = NULL;

void NDS_HW7_Init(NDS_HW_IPC* _ipc)
{
	ipc = _ipc;
}

void NDS_HW7_Exit()
{
	ipc = NULL;
}

void NDS_HW7_PlayStart()
{
}

void NDS_HW7_PlayStop()
{
	int i;
	for(i = 0; i < NDS_HW_MAXVOICES; i++) {
		SCHANNEL_CR(i) = 0;
	}
}

void NDS_HW7_Reset()
{
}

void NDS_HW7_VoiceUpdate(u8 voice, u8 changes)
{
	NDS_HW_VOICE* v = ipc->voices + voice;

	if(changes & NDS_HW_CHANGE_VOLUME) {
		SCHANNEL_VOL(voice) = v->volume >> 1;
	}

	if(changes & NDS_HW_CHANGE_PANNING) {
		SCHANNEL_PAN(voice) = v->panning >> 1;
	}

	if(changes & NDS_HW_CHANGE_FREQUENCY) {
		SCHANNEL_TIMER(voice) = SOUND_FREQ((s32)v->frequency);
	}
	
	if(changes & NDS_HW_CHANGE_STOP) {
		SCHANNEL_CR(voice) = 0;
	}

	if(changes & NDS_HW_CHANGE_START) {
		SCHANNEL_SOURCE(voice) = (u32)ipc->samples[v->handle];
		SCHANNEL_REPEAT_POINT(voice) = v->loopstart >> ((v->flags & SF_16BITS) ? 1 : 2);
		if(v->flags & SF_LOOP)
			SCHANNEL_LENGTH(voice) = (v->loopend - v->loopstart) >> ((v->flags & SF_16BITS) ? 1 : 2);
		else
			SCHANNEL_LENGTH(voice) = v->length >> ((v->flags & SF_16BITS) ? 1 : 2);
		SCHANNEL_CR(voice) =
			SOUND_VOL(v->volume >> 1) |
			SOUND_PAN(v->panning >> 1) |
			((v->flags & SF_LOOP) ? SOUND_REPEAT : SOUND_ONE_SHOT) |
			((v->flags & SF_16BITS) ? SOUND_FORMAT_16BIT : SOUND_FORMAT_8BIT) |
			SCHANNEL_ENABLE;
	}
}
