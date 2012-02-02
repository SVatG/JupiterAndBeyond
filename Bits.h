#ifndef __BITS_H__
#define __BITS_H__

static inline uint32_t InterleaveZeros(uint32_t val)
{
	val=(val|(val<<8))&0x00ff00ff;
	val=(val|(val<<4))&0x0f0f0f0f;
	val=(val|(val<<2))&0x33333333;
	val=(val|(val<<1))&0x55555555;
	return val;
}

static inline uint32_t SetBits(uint32_t original,uint32_t bits,int state)
{
	if(!state) return original&~bits;
	else return original|bits;
}

static inline uint32_t SetDoubleBits(uint32_t original,uint16_t bits,int state)
{
	uint32_t mask=InterleaveZeros(bits);
	return (original&~(mask*3))|(mask*state);
}

#endif
