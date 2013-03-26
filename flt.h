const struct { int32_t a; int32_t b; int32_t c; } coeffs[] = {
	#include "fltgen/filters.inc"
};



// Simple 2-poles resonant filter
void SetupFilter(BitBinSong *self, int cutoff, int resonance) 
//----------------------------------------------------------------------------------------
{

/*	int fs = gdwMixingFreq;
	u32 fg, fb0, fb1;

	// x is 16.16
    u32 x =  ((uint64_t)fs * _it_cutoff_table[cutoff]) >> 16;
    // y is 0.32
    u32 y = _it_resonance_table[resonance];
    // 16.16
    u32 x2 = ((uint64_t)x * x) >> 16;
    // 16.16
    u32 xy = ((uint64_t)x * y) >> 32;
    y >>= 16; // make y 16.16 now
     fg = 0xFFFFFFFF / (x2 + xy + y); //  1*S*S / (a*S)  = S*1/a (fg is still 16.16)
     fb0 = (fg * (2*x2 + xy + (uint64_t)y - 0x00010000)) >> 16;
	 fb1 = 0x00010000 - fg - fb0;
	*/
	
	self->c = cutoff * 0x101;
	//self->c = pow( 0.5, (128 - cutoff)/16.0 ) * 0xFFFF;
	self->r = 0x7FFF; 


	self->nFilter_A0 = coeffs[cutoff].a; 
	self->nFilter_B0 =  coeffs[cutoff].b;
	self->nFilter_B1 =  coeffs[cutoff].c;
	
	//fprintf(stdout, "%d %d %d\n", self->nFilter_A0, self->nFilter_B0, self->nFilter_B1);

}
