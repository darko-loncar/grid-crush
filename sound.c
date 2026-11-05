#include "sound.h"
#include <SDL3/SDL.h>
#include <math.h>

#define SOUND_SAMPLE_RATE 22050

#define TAP_VOLUME 0.6f

static SDL_AudioStream *audio_stream;

bool sound_init(void) {
    SDL_AudioSpec spec = {
        .format = SDL_AUDIO_F32,
        .channels = 1,
        .freq = SOUND_SAMPLE_RATE
    };

    audio_stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!audio_stream) {
        SDL_Log("Could not open audio device: %s", SDL_GetError());
        return false;
    }

    if (!SDL_ResumeAudioStreamDevice(audio_stream)) {
        SDL_Log("Could not start audio device: %s", SDL_GetError());
        SDL_DestroyAudioStream(audio_stream);
        audio_stream = NULL;
        return false;
    }
    return true;
}

void sound_play_piece_lock(void) {
    if (!audio_stream) return;

    enum { SAMPLE_COUNT = 1212 };
    float samples[SAMPLE_COUNT];
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        float time = (float)i / SOUND_SAMPLE_RATE;
        float envelope = expf(-55.0f * time);
        float frequency = 190.0f - 900.0f * time;
        samples[i] = sinf(6.2831853f * frequency * time) * envelope * TAP_VOLUME;
    }

    if (!SDL_PutAudioStreamData(audio_stream, samples, sizeof(samples))) {
        SDL_Log("Could not queue piece-lock sound: %s", SDL_GetError());
    }
}

void sound_shutdown(void) {
    if (audio_stream) {
        SDL_DestroyAudioStream(audio_stream);
        audio_stream = NULL;
    }
}
