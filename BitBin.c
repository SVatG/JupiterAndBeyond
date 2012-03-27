#include "BitBin.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

static void InitializeBitBinChannel(BitBinChannel *channel,BitBinNote *notes);

static void MoveSongToNextSample(BitBinSong *self);
static void MoveSongToNextTick(BitBinSong *self);
static void MoveSongToNextRow(BitBinSong *self);

static void UpdateChannelAtRow(BitBinSong *self,BitBinChannel *channel);
static void UpdateChannelAtNonRowTick(BitBinSong *self,BitBinChannel *channel);
static void UpdateChannelAtSample(BitBinSong *self,BitBinChannel *channel);
static int32_t ChannelAmplitude(BitBinChannel *channel);
static int32_t SquareWave1_8(BitBinChannel *channel);
static int32_t SquareWave1_4(BitBinChannel *channel);
static int32_t SquareWave1_2(BitBinChannel *channel);
static int32_t SquareWave3_4(BitBinChannel *channel);
static int32_t TriangleWave(BitBinChannel *channel);
static int32_t Noise(BitBinChannel *channel);
static uint32_t Hash32(uint32_t val);

static uint32_t PhaseVelocityForNote(BitBinSong *self,int note);

static int8_t SineTable[48]=
{
	0,8,16,24,32,39,45,51,55,59,62,63,
	64,63,62,59,55,51,45,39,32,24,16,8,
	0,-8,-16,-24,-32,-39,-45,-51,-55,-59,-62,-63,
	-64,-63,-62,-59,-55,-51,-45,-39,-32,-24,-16,-8,
};

void InitializeBitBinSong(BitBinSong *self,const uint32_t *phasetable,int numchannels,BitBinNote **notes)
{
	self->currentsample=0xffffffff;
	self->nexttick=0;
	self->samplespertick=phasetable[0]/50;

	self->currenttick=0xffffffff;
	self->nextrow=0;
	self->ticksperrow=3;

	self->currentrow=0xffffffff;

	self->phasetable=&phasetable[1];

	self->numchannels=numchannels;
	for(int i=0;i<self->numchannels;i++) InitializeBitBinChannel(&self->channels[i],notes[i]);
}

static void InitializeBitBinChannel(BitBinChannel *channel,BitBinNote *notes)
{
	channel->note=0;
	channel->waveform=0;
	channel->effect=0;

	channel->phase=0;
	channel->phasedelta=0;

	channel->currentvolume=0;
	channel->requestedvolume=0;
	channel->mastervolume=64;

	memset(channel->memory,0,sizeof(channel->memory));

	channel->notes=notes;
}



int16_t NextBitBinSample(BitBinSong *self)
{
	MoveSongToNextSample(self);

	int32_t amplitude=0;
	for(int i=0;i<self->numchannels;i++)
	{
		amplitude+=ChannelAmplitude(&self->channels[i]);
	}

	amplitude>>=12+3;
	if(amplitude>32767) return 32767;
	else if(amplitude<-32768) return -32768;
	else return amplitude;
}

void RenderBitBinSamples(BitBinSong *self,int numsamples,int16_t *samples)
{
	for(int i=0;i<numsamples;i++) *samples++=NextBitBinSample(self);
}

static void MoveSongToNextSample(BitBinSong *self)
{
	self->currentsample++;
	if(self->currentsample==self->nexttick)
	{
		MoveSongToNextTick(self);
		self->nexttick+=self->samplespertick;
	}

	for(int i=0;i<self->numchannels;i++) UpdateChannelAtSample(self,&self->channels[i]);
}

static void MoveSongToNextTick(BitBinSong *self)
{
	self->currenttick++;
	if(self->currenttick==self->nextrow)
	{
		MoveSongToNextRow(self);
		self->nextrow+=self->ticksperrow;
		return;
	}

	for(int i=0;i<self->numchannels;i++) UpdateChannelAtNonRowTick(self,&self->channels[i]);
}

static void MoveSongToNextRow(BitBinSong *self)
{
	self->currentrow++;
	if(self->currentrow==128) self->currentrow=0;

	for(int i=0;i<self->numchannels;i++) UpdateChannelAtRow(self,&self->channels[i]);
}




static void UpdateChannelAtRow(BitBinSong *self,BitBinChannel *channel)
{
	int note=channel->notes[self->currentrow].note;
	int waveform=channel->notes[self->currentrow].waveform;
	int volume=channel->notes[self->currentrow].volume;
	int effect=channel->notes[self->currentrow].effect;
	int parameter=channel->notes[self->currentrow].parameter;

	if(note==0xfe)
	{
		channel->requestedvolume=0;
	}
	else
	{
		if(note!=0xff)
		{
			if(effect=='G') channel->portadest=note<<4;
			else channel->note=note<<4;
		}

		if(waveform!=0xff)
		{
			channel->waveform=waveform;
			channel->phase=0;
			channel->requestedvolume=64;
		}

		if(volume!=0xff)
		{
			channel->requestedvolume=volume;
		}

		if(effect!=0xff)
		{
			if(parameter==0) parameter=channel->memory[effect-'A'];
			else channel->memory[effect-'A']=parameter;
		}

		channel->effect=effect;
		channel->parameter=parameter;
	}

	// Always reset vibrato if the current effect is not vibrato.
	if(effect!='H')
	{
		channel->vibrato=0;
		channel->vibratophase=0;
	}

	int lo=parameter&0x0f;
	int hi=(parameter&0xf0)>>4;

	switch(channel->effect)
	{
		case 'A':
			self->ticksperrow=parameter;
		break;

		case 'D': 
			if(hi==0x0f)
			{
				if(lo==0) channel->requestedvolume+=15;
				else channel->requestedvolume-=lo;
			}
			else if(lo==0x0f)
			{
				if(hi==0) channel->requestedvolume-=15;
				else channel->requestedvolume+=hi;
			}

			if(channel->requestedvolume<0) channel->requestedvolume=0;
			if(channel->requestedvolume>64) channel->requestedvolume=64;
		break;

		case 'H':
			channel->vibrato=lo*SineTable[channel->vibratophase]>>6;
			channel->vibratophase+=hi;
			if(channel->vibratophase>=48) channel->vibratophase-=48;
		break;

		case 'J':
			channel->vibrato=0;
			channel->vibratophase=1;
		break;

		case 'M':
		   channel->mastervolume=parameter;
		break;
	}

	// Don't ramp volume on note starts.
	if(note!=0xff) channel->currentvolume=channel->requestedvolume;

	channel->phasedelta=PhaseVelocityForNote(self,channel->note+channel->vibrato);
}

static void UpdateChannelAtNonRowTick(BitBinSong *self,BitBinChannel *channel)
{
	if(channel->note==0xff || channel->effect==0xff) return;

	int parameter=channel->parameter;
	int lo=parameter&0x0f;
	int hi=(parameter&0xf0)>>4;

	switch(channel->effect)
	{
		case 'D': 
			if(hi==0) channel->requestedvolume-=lo;
			else if(lo==0) channel->requestedvolume+=hi;

			if(channel->requestedvolume<0) channel->requestedvolume=0;
			if(channel->requestedvolume>64) channel->requestedvolume=64;
		break;

		case 'E':
			channel->note-=parameter;
		break;

		case 'F':
			channel->note+=parameter;
		break;

		case 'G':
			if(channel->portadest>channel->note+parameter) channel->note+=parameter;
			else if(channel->portadest<channel->note-parameter) channel->note-=parameter;
			else channel->note=channel->portadest;
		break;

		case 'H':
			channel->vibrato=lo*SineTable[channel->vibratophase]>>6;
			channel->vibratophase+=hi;
			if(channel->vibratophase>=48) channel->vibratophase-=48;
		break;

		case 'J':
			switch(channel->vibratophase%3)
			{
				case 0: channel->vibrato=0; break;
				case 1: channel->vibrato=hi; break;
				case 2: channel->vibrato=lo; break;
			}
			channel->vibratophase++;
		break;
	}

	channel->phasedelta=PhaseVelocityForNote(self,channel->note+channel->vibrato);
}

static void UpdateChannelAtSample(BitBinSong *self,BitBinChannel *channel)
{
	if((self->currentsample&7)==0) // TODO: Should be sample rate dependent.
	{
		if(channel->currentvolume<channel->requestedvolume) channel->currentvolume++;
		else if(channel->currentvolume>channel->requestedvolume) channel->currentvolume--;
	}

	channel->phase+=channel->phasedelta;
}

static int32_t ChannelAmplitude(BitBinChannel *channel)
{
	if(channel->currentvolume==0) return 0;
	if(channel->mastervolume==0) return 0;

	int32_t sample=0;
	switch(channel->waveform)
	{
		case 0: sample=SquareWave1_8(channel); break;
		case 1: sample=SquareWave1_4(channel); break;
		case 2: sample=SquareWave1_2(channel); break;
		case 3: sample=SquareWave3_4(channel); break;
		case 4: sample=TriangleWave(channel); break;
		case 5: sample=Noise(channel); break;
	}

	return sample*channel->currentvolume*channel->mastervolume;
}

static int32_t SquareWave1_8(BitBinChannel *channel)
{
	if((channel->phase&0x70000)==0) return 32767;
	else return -32768;
}

static int32_t SquareWave1_4(BitBinChannel *channel)
{
	if((channel->phase&0x60000)==0) return 32767;
	else return -32768;
}

static int32_t SquareWave1_2(BitBinChannel *channel)
{
	if((channel->phase&0x40000)==0) return 32767;
	else return -32768;
}

static int32_t SquareWave3_4(BitBinChannel *channel)
{
	if((channel->phase&0x60000)!=0x60000) return 32767;
	else return -32768;
}

static int32_t TriangleWave(BitBinChannel *channel)
{
	uint32_t phase=channel->phase<<12;
	uint32_t trianglewave=((phase<<1)^((int32_t)phase)>>31);
	int32_t signedwave=trianglewave-0x80000000;
	return (signedwave>>16);
}

static int32_t Noise(BitBinChannel *channel)
{
	return Hash32(channel->phase>>16)&1?32767:-32768;
}

static uint32_t Hash32(uint32_t val)
{
	val^=val>>16;
	val^=61;
	val+=val<<3;
	val^=val>>4;
	val*=0x27d4eb2d;
	val^=val>>15;
	return val;
}

static uint32_t PhaseVelocityForNote(BitBinSong *self,int note)
{
	unsigned int octave=(unsigned int)note/(12*16);
	unsigned int index=(unsigned int)note%(12*16);

	return self->phasetable[index]>>(10-octave);
}
