/* mayu plugin: sound - sound-related API  {{{1
 *
 * Copyright (C) 2011 emonkak <emonkak@gmail.com>
 */

#include <windows.h>
#include <mmsystem.h>
#include <math.h>




/* Misc.  {{{1 */

inline static BOOL
mixer_open(HMIXER* mixer)
{
	return mixerOpen(mixer, 0, 0, 0, MIXER_OBJECTF_HMIXER) == 0;
}


inline static BOOL
mixer_close(HMIXER mixer)
{
	return mixerClose(mixer) == 0;
}


inline static BOOL
mixer_get_cotrol(HMIXER mixer, MIXERCONTROL* mc, DWORD control_type)
{
	MIXERLINE ml;
	MIXERLINECONTROLS mlc;

	ZeroMemory(&ml, sizeof(ml));
	ml.cbStruct = sizeof(ml);
	ml.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

	if (mixerGetLineInfo((HMIXEROBJ)mixer, &ml,
			MIXER_GETLINEINFOF_COMPONENTTYPE | MIXER_OBJECTF_HMIXER) != 0)
		return FALSE;

	ZeroMemory(&mlc, sizeof(mlc));
	mlc.cbStruct = sizeof(mlc);
	mlc.dwLineID = ml.dwLineID;
	mlc.dwControlType = control_type;
	mlc.cControls = ml.cControls;
	mlc.cbmxctrl = sizeof(MIXERCONTROL);
	mlc.pamxctrl = mc;

	if (mixerGetLineControls((HMIXEROBJ)mixer, &mlc,
			MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER) != 0)
		return FALSE;

	return TRUE;
}


inline static UINT
mixer_get_volume(HMIXER mixer, DWORD control_id)
{
	MIXERCONTROLDETAILS mcd;
	MIXERCONTROLDETAILS_UNSIGNED mcdu;

	ZeroMemory(&mcd, sizeof(mcd));
	ZeroMemory(&mcdu, sizeof(mcdu));
	mcd.cbStruct = sizeof(mcd);
	mcd.dwControlID = control_id;
	mcd.cChannels = 1;
	mcd.cMultipleItems = 0;
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mcd.paDetails = &mcdu;

	mixerGetControlDetails((HMIXEROBJ)mixer, &mcd,
		MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);

	return mcdu.dwValue;
}


inline static BOOL
mixer_get_mute(HMIXER mixer, DWORD control_id)
{
	MIXERCONTROLDETAILS mcd;
	MIXERCONTROLDETAILS_BOOLEAN mcdb;

	ZeroMemory(&mcd, sizeof(mcd));
	ZeroMemory(&mcdb, sizeof(mcdb));
	mcd.cbStruct = sizeof(mcd);
	mcd.dwControlID = control_id;
	mcd.cChannels = 1;
	mcd.cMultipleItems = 0;
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mcd.paDetails = &mcdb;

	mixerGetControlDetails((HMIXEROBJ)mixer, &mcd,
		MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);

	return mcdb.fValue != 0;
}


inline static BOOL
mixer_set_volume(HMIXER mixer, DWORD control_id, UINT step)
{
	MIXERCONTROLDETAILS mcd;
	MIXERCONTROLDETAILS_UNSIGNED mcdu;

	ZeroMemory(&mcd, sizeof(mcd));
	ZeroMemory(&mcdu, sizeof(mcdu));
	mcd.cbStruct = sizeof(mcd);
	mcd.dwControlID = control_id;
	mcd.cChannels = 1;
	mcd.cMultipleItems = 0;
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mcd.paDetails = &mcdu;
	mcdu.dwValue = step;

	return mixerSetControlDetails((HMIXEROBJ)mixer, &mcd,
		MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE) == 0;
}


inline static BOOL
mixer_set_mute(HMIXER mixer, DWORD control_id, BOOL muted_p)
{
	MIXERCONTROLDETAILS mcd;
	MIXERCONTROLDETAILS_BOOLEAN mcdb;

	ZeroMemory(&mcd, sizeof(mcd));
	ZeroMemory(&mcdb, sizeof(mcdb));
	mcd.cbStruct = sizeof(mcd);
	mcd.dwControlID = control_id;
	mcd.cChannels = 1;
	mcd.cMultipleItems = 0;
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mcd.paDetails = &mcdb;
	mcdb.fValue = muted_p;

	return mixerSetControlDetails((HMIXEROBJ)mixer, &mcd,
		MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE) == 0;
}




/* Public Functions  {{{1 */

void WINAPI
play_sound(const char* param)
{
	PlaySound(param, NULL, SND_FILENAME | SND_SYNC);
}


void WINAPI
set_volume(const char* param)
{
	HMIXER mixer;
	MIXERCONTROL mc;
	int volume;
	int param_len;

	if (!mixer_open(&mixer))
		return;

	ZeroMemory(&mc, sizeof(mc));

	if (!mixer_get_cotrol(mixer, &mc, MIXERCONTROL_CONTROLTYPE_VOLUME)) {
		mixer_close(mixer);
		return;
	}

	param_len = strlen(param);
	if (param_len > 0) {
		volume = atoi(param);

		if (*(param + param_len - 1) == '%')
			volume = (int)round(volume * (mc.Bounds.lMaximum - mc.Bounds.lMinimum) / 100.0) + mc.Bounds.lMinimum;

		if (*param == '-' || *param == '+')
			volume += mixer_get_volume(mixer, mc.dwControlID);

		volume = max(mc.Bounds.lMinimum, min(volume, mc.Bounds.lMaximum));
		mixer_set_volume(mixer, mc.dwControlID, volume);
	}

	mixer_close(mixer);
}


void WINAPI
set_mute(const char* param)
{
	HMIXER mixer;
	MIXERCONTROL mc;
	BOOL muted_p;

	if (!mixer_open(&mixer))
		return;

	ZeroMemory(&mc, sizeof(mc));

	if (!mixer_get_cotrol(mixer, &mc, MIXERCONTROL_CONTROLTYPE_MUTE)) {
		mixer_close(mixer);
		return;
	}

	muted_p = (BOOL)atoi(param);

	mixer_set_mute(mixer, mc.dwControlID, muted_p);
	mixer_close(mixer);
}


void WINAPI
toggle_mute(const char* param)
{
	HMIXER mixer;
	MIXERCONTROL mc;
	BOOL muted_p;

	if (!mixer_open(&mixer))
		return;

	ZeroMemory(&mc, sizeof(mc));

	if (!mixer_get_cotrol(mixer, &mc, MIXERCONTROL_CONTROLTYPE_MUTE)) {
		mixer_close(mixer);
		return;
	}

	muted_p = mixer_get_mute(mixer, mc.dwControlID);
	mixer_set_mute(mixer, mc.dwControlID, (!muted_p));
	mixer_close(mixer);
}




/* __END__ {{{1 */
/* vim: foldmethod=marker
 */
