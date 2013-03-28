#include "Voxelscape.h"
#include "VGA.h"
#include "Random.h"
#include "Utils.h"
#include "Global.h"

#include "Graphics/Bitmap.h"
#include "Graphics/Drawing.h"

#include "Cockpit.h"

#include <string.h>

extern uint8_t HeightMap[256*256];

static void DrawSimpleColumnDownwards(uint8_t *pixels,int length,uint8_t c)
{
	for(int i=0;i<length;i++)
	{
		*pixels=c;
		pixels+=320;
	}
}

static void DrawMultiColumnDownwards(uint8_t *pixels,int length,int h,int dh,uint8_t *palette)
{
	for(int i=0;i<length;i++)
	{
		*pixels=palette[(h>>13)&7];
		h+=dh;
		pixels+=320;
	}
}

static void DrawSimpleColumnUpwards(uint8_t *pixels,int length,uint8_t c)
{
	for(int i=0;i<length;i++)
	{
		*pixels=c;
		pixels-=320;
	}
}

static void DrawMultiColumnUpwards(uint8_t *pixels,int length,int h,int dh,uint8_t *palette)
{
	for(int i=0;i<length;i++)
	{
		*pixels=palette[(h>>13)&7];
		h+=dh;
		pixels-=320;
	}
}

void Voxelscape()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,320*200);
	memset(framebuffer2,0,320*200);

	SetVGAScreenMode320x200_60Hz(framebuffer1);


        Bitmap frame1,frame2;
        InitializeBitmap(&frame1,320,200,320,framebuffer1);
        InitializeBitmap(&frame2,320,200,320,framebuffer2);
        
	int frame=0;
        while(CurrentBitBinRow(songp) < 928)
	{
		WaitVBL();

		int t=VGAFrameCounter();

		uint8_t *destination;
                Bitmap* currframe;
		if(frame&1)
		{
			destination=framebuffer2;
			SetFrameBuffer(framebuffer1);
                        currframe=&frame2;
		}
		else
		{
			destination=framebuffer1;
			SetFrameBuffer(framebuffer2);
                        currframe=&frame1;
		}
		frame^=1;

		uint8_t toppalette[8],bottompalette[8];
		for(int i=0;i<8;i++)
		{
			int j=7-i;
			toppalette[i]=RawRGB(i,i,i/2);
			bottompalette[i]=RawRGB(j,j*j/7,j*j*j*j/(7*7*7)/2);
		}


		#define NumberOfStrips 48
		#define Perspective 4

		int32_t a=t*4;
		int32_t sin_a=isin(a);
		int32_t cos_a=icos(a);
		int32_t u0=0;
		int32_t v0=t*Fix(5);

		for(int i=0;i<320;i++) { data.voxel.toplimity[i]=-1; data.voxel.bottomlimity[i]=200; }

		for(int i=0;i<NumberOfStrips;i++)
		{
			//if(i>6 && (i&1)) continue;

			int32_t z=Fix(i*i/4+1);
			//int32_t z=Fix((i+1)*8);
			int32_t rz=idiv(Fix(1024000/Perspective),z);

			int32_t du=Perspective*imul(z,-sin_a)/320;
			int32_t dv=Perspective*imul(z,cos_a)/320;
			int32_t u=u0+imul(z,cos_a)-du*320/2;
			int32_t v=v0+imul(z,sin_a)-dv*320/2;

			for(int x=0;x<320;x++)
			{
				if(data.voxel.toplimity[x]>=data.voxel.bottomlimity[x]) { u+=du; v+=dv; continue; }

				int bottomh=HeightMap[((u>>5)&0xff00)|((v>>13)&0xff)];
				/*int offset=((u>>5)&0xff00)|((v>>13)&0xff);
				int bottomh1=HeightMap[offset];
				int bottomh2=HeightMap[offset+1];
				int bottomh3=HeightMap[offset+256];
				int bottomh4=HeightMap[offset+257];
				int ufrac=u&0x1fff;
				int vfrac=v&0x1fff;
				int bottomh5=bottomh1+((bottomh2-bottomh1)*vfrac>>13);
				int bottomh6=bottomh3+((bottomh4-bottomh3)*vfrac>>13);
				int bottomh=bottomh5+((bottomh6-bottomh5)*ufrac>>13);*/

				//int bottomh=(isin(u/256)+isin(v/256)+Fix(2))>>6;
				int bottomy=100+FixedToInt(imul(bottomh,rz));

				if(bottomy<data.voxel.bottomlimity[x] && i!=0)
				{
					int starth=bottomh;
					int endh=data.voxel.bottomlasth[x];
					int startc=(starth>>5)&7;
					int endc=(endh>>5)&7;

					if(startc==endc)
					{
						DrawSimpleColumnDownwards(destination+x+bottomy*320,
						data.voxel.bottomlimity[x]-bottomy,bottompalette[endc]);
					}
					else
					{
						int dh=((endh-starth)<<8)/(data.voxel.bottomlasty[x]-bottomy);

						DrawMultiColumnDownwards(destination+x+bottomy*320,
						data.voxel.bottomlimity[x]-bottomy,starth<<8,dh,bottompalette);
					}

					data.voxel.bottomlimity[x]=bottomy;
				}
				data.voxel.bottomlasty[x]=bottomy;
				data.voxel.bottomlasth[x]=bottomh;

				int toph=HeightMap[((u>>6)&0xff00)|((v>>14)&0xff)];
				int topy=100+FixedToInt(imul(-toph-40,rz/2));

				if(topy>data.voxel.toplimity[x] && i!=0)
				{
					int starth=toph;
					int endh=data.voxel.toplasth[x];
					int startc=(starth>>5)&7;
					int endc=(endh>>5)&7;

					if(startc==endc)
					{
						DrawSimpleColumnUpwards(destination+x+topy*320,
						topy-data.voxel.toplimity[x],toppalette[endc]);
					}
					else
					{
						int dh=((endh-starth)<<8)/(topy-data.voxel.toplasty[x]);

						DrawMultiColumnUpwards(destination+x+topy*320,
						topy-data.voxel.toplimity[x],starth<<8,dh,toppalette);
					}

					data.voxel.toplimity[x]=topy;
				}
				data.voxel.toplasty[x]=topy;
				data.voxel.toplasth[x]=toph;

				u+=du;
				v+=dv;
			}
		}

		for(int x=0;x<320;x++)
		{
			if(data.voxel.toplimity[x]<data.voxel.bottomlimity[x])
			{
				DrawSimpleColumnDownwards(destination+x+(data.voxel.toplimity[x]+1)*320,
				data.voxel.bottomlimity[x]-data.voxel.toplimity[x]-1,0);
			}
		}
                
		DrawRLEBitmap(currframe, &cockpit, 0, 0);
	}

	while(UserButtonState());
}
