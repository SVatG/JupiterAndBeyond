#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <stdint.h>
#include <stdbool.h>

typedef void AudioCallbackFunction(void *context,int buffer);

void InitializeAudio();

void SetAudioOutputThroughDAC();

void AudioOn();
void AudioOff();

void SetAudioVolume(int volume);
int SetAudioFormat(int samplerate,int bitspersample,int channels);

void PlayAudioWithCallback(AudioCallbackFunction *callback,void *context);
void StopAudio();

void ProvideAudioBuffer(void *samples,int numsamples);
bool ProvideAudioBufferWithoutBlocking(void *samples,int numsamples);

void SetAudioOutputThroughDAC();
void MakeDACNoise();
void SetAudioDACValue(int16_t sample);

#endif
