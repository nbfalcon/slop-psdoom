// i_sound_sdl3.c - SDL3 sound backend for psdoom
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "z_zone.h"
#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "doomdef.h"
#include "sounds.h"

extern int numChannels;
extern boolean nosound;

#define SAMPLECOUNT     512
#define SAMPLERATE      11025

static SDL_AudioStream *audio_stream = NULL;

static int lengths[NUMSFX];

static signed short mixbuffer[SAMPLECOUNT * 2];

static unsigned int channelstep[NUM_CHANNELS];
static unsigned int channelstepremainder[NUM_CHANNELS];
static unsigned char *channels[NUM_CHANNELS];
static unsigned char *channelsend[NUM_CHANNELS];
static int channelstart[NUM_CHANNELS];
static int channelhandles[NUM_CHANNELS];
static int channelids[NUM_CHANNELS];
static int steptable[256];
static int vol_lookup[128 * 256];
static int *channelleftvol_lookup[NUM_CHANNELS];
static int *channelrightvol_lookup[NUM_CHANNELS];

void *getsfx(char *sfxname, int *len)
{
    unsigned char *sfx;
    unsigned char *paddedsfx;
    int i;
    int size;
    int paddedsize;
    char name[20];
    int sfxlump;

    sprintf(name, "ds%s", sfxname);

    if (W_CheckNumForName(name) == -1)
        sfxlump = W_GetNumForName("dspistol");
    else
        sfxlump = W_GetNumForName(name);

    size = W_LumpLength(sfxlump);

    sfx = (unsigned char *)W_CacheLumpNum(sfxlump, PU_STATIC);

    paddedsize = ((size - 8 + (SAMPLECOUNT - 1)) / SAMPLECOUNT) * SAMPLECOUNT;

    paddedsfx = (unsigned char *)Z_Malloc(paddedsize + 8, PU_STATIC, (void *)0);

    memcpy(paddedsfx, sfx, size);
    for (i = size; i < paddedsize + 8; i++)
        paddedsfx[i] = 128;

    Z_Free(sfx);

    *len = paddedsize;

    return (void *)(paddedsfx + 8);
}

int addsfx(int sfxid, int volume, int step, int seperation)
{
    static unsigned short handlenums = 0;
    int i;
    int rc = -1;
    int oldest = gametic;
    int oldestnum = 0;
    int slot;
    int rightvol;
    int leftvol;

    if (sfxid == sfx_sawup
        || sfxid == sfx_sawidl
        || sfxid == sfx_sawful
        || sfxid == sfx_sawhit
        || sfxid == sfx_stnmov
        || sfxid == sfx_pistol)
    {
        for (i = 0; i < numChannels; i++) {
            if (channels[i] && (channelids[i] == sfxid)) {
                channels[i] = 0;
                break;
            }
        }
    }

    for (i = 0; (i < numChannels) && (channels[i]); i++) {
        if (channelstart[i] < oldest) {
            oldestnum = i;
            oldest = channelstart[i];
        }
    }

    if (i == numChannels)
        slot = oldestnum;
    else
        slot = i;

    channels[slot] = (unsigned char *)S_sfx[sfxid].data;
    channelsend[slot] = channels[slot] + lengths[sfxid];
    channelhandles[slot] = rc = handlenums++;
    channelstep[slot] = step;
    channelstepremainder[slot] = 0;
    channelstart[slot] = gametic;

    seperation += 1;
    leftvol = volume - ((volume * seperation * seperation) >> 16);
    seperation = seperation - 257;
    rightvol = volume - ((volume * seperation * seperation) >> 16);

    if (rightvol < 0) rightvol = 0;
    if (rightvol > 127) rightvol = 127;
    if (leftvol < 0) leftvol = 0;
    if (leftvol > 127) leftvol = 127;

    channelleftvol_lookup[slot] = &vol_lookup[leftvol * 256];
    channelrightvol_lookup[slot] = &vol_lookup[rightvol * 256];
    channelids[slot] = sfxid;

    return rc;
}

void I_SetChannels(void)
{
    int i, j;
    int *steptablemid = steptable + 128;

    for (i = -128; i < 128; i++)
        steptablemid[i] = (int)(pow(2.0, (i / 64.0)) * 65536.0);

    for (i = 0; i < 128; i++) {
        for (j = 0; j < 256; j++) {
            vol_lookup[i * 256 + j] = ((i * (j - 128) * 256) / 127) << 2;
        }
    }
}

void I_SetSfxVolume(int volume)
{
    snd_SfxVolume = volume;
}

int I_GetSfxLumpNum(sfxinfo_t *sfx)
{
    char namebuf[9];
    sprintf(namebuf, "ds%s", sfx->name);
    return W_GetNumForName(namebuf);
}

int I_StartSound(int id, int vol, int sep, int pitch, int priority)
{
    (void)priority;
    if (nosound)
        return 0;

    id = addsfx(id, vol, steptable[pitch], sep);
    return id;
}

void I_StopSound(int handle)
{
    for (int i = 0; i < numChannels; i++) {
        if (channelhandles[i] == handle) {
            channels[i] = 0;
            break;
        }
    }
}

int I_SoundIsPlaying(int handle)
{
    for (int i = 0; i < numChannels; i++) {
        if (channelhandles[i] == handle)
            return 1;
    }
    return 0;
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
    (void)pitch;
    for (int i = 0; i < numChannels; i++) {
        if (channelhandles[i] == handle) {
            sep += 1;
            int leftvol = vol - ((vol * sep * sep) >> 16);
            sep = sep - 257;
            int rightvol = vol - ((vol * sep * sep) >> 16);

            if (rightvol < 0) rightvol = 0;
            if (rightvol > 127) rightvol = 127;
            if (leftvol < 0) leftvol = 0;
            if (leftvol > 127) leftvol = 127;

            channelleftvol_lookup[i] = &vol_lookup[leftvol * 256];
            channelrightvol_lookup[i] = &vol_lookup[rightvol * 256];
            break;
        }
    }
}

void I_UpdateSound(void)
{
    if (nosound || !audio_stream)
        return;

    int queued = SDL_GetAudioStreamQueued(audio_stream);
    if (queued > (int)(SAMPLECOUNT * 4 * sizeof(signed short)))
        return;

    signed short *leftout = mixbuffer;
    signed short *rightout = mixbuffer + 1;
    signed short *leftend = mixbuffer + SAMPLECOUNT * 2;

    while (leftout < leftend) {
        int dl = 0;
        int dr = 0;

        for (int chan = 0; chan < numChannels; chan++) {
            if (channels[chan]) {
                unsigned int sample = *channels[chan];
                dl += channelleftvol_lookup[chan][sample];
                dr += channelrightvol_lookup[chan][sample];

                channelstepremainder[chan] += channelstep[chan];
                channels[chan] += channelstepremainder[chan] >> 16;
                channelstepremainder[chan] &= 65535;

                if (channels[chan] >= channelsend[chan])
                    channels[chan] = 0;
            }
        }

        if (dl > 0x7fff) *leftout = 0x7fff;
        else if (dl < -0x8000) *leftout = -0x8000;
        else *leftout = (signed short)dl;

        if (dr > 0x7fff) *rightout = 0x7fff;
        else if (dr < -0x8000) *rightout = -0x8000;
        else *rightout = (signed short)dr;

        leftout += 2;
        rightout += 2;
    }

    SDL_PutAudioStreamData(audio_stream, mixbuffer, sizeof(mixbuffer));
}

void I_SubmitSound(void)
{
    I_UpdateSound();
}

void I_ShutdownSound(void)
{
    if (audio_stream) {
        SDL_DestroyAudioStream(audio_stream);
        audio_stream = NULL;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void I_InitSound(void)
{
    if (M_CheckParm("-nosound")) {
        nosound = true;
        return;
    }

    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        fprintf(stderr, "SDL_InitSubSystem(AUDIO) failed: %s; disabling sound\n", SDL_GetError());
        nosound = true;
        return;
    }

    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;
    spec.freq = SAMPLERATE;

    audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!audio_stream) {
        fprintf(stderr, "SDL_OpenAudioDeviceStream failed: %s; disabling sound\n", SDL_GetError());
        nosound = true;
        return;
    }

    SDL_ResumeAudioStreamDevice(audio_stream);

    I_SetChannels();

    for (int i = 1; i < NUMSFX; i++) {
        if (!S_sfx[i].link) {
            S_sfx[i].data = getsfx(S_sfx[i].name, &lengths[i]);
        } else {
            S_sfx[i].data = S_sfx[i].link->data;
            lengths[i] = lengths[(S_sfx[i].link - S_sfx) / sizeof(sfxinfo_t)];
        }
    }
}

// Stubbed music subsystem
void I_InitMusic(void) {}
void I_ShutdownMusic(void) {}
void I_SetMusicVolume(int volume) { (void)volume; }
void I_SetCdVolume(int volume) { (void)volume; }
void I_PauseSong(int handle) { (void)handle; }
void I_ResumeSong(int handle) { (void)handle; }
int I_RegisterSong(void *data) { (void)data; return 1; }
void I_PlaySong(int handle, int looping) { (void)handle; (void)looping; }
void I_StopSong(int handle) { (void)handle; }
void I_UnRegisterSong(int handle) { (void)handle; }
