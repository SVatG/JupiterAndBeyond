#include "Starfield.h"
#include "VGA.h"
#include "LED.h"
#include "Button.h"
#include "Random.h"

#include "Graphics/Bitmap.h"
#include "Graphics/RLEBitmap.h"
#include "Graphics/Drawing.h"

#include "VectorLibrary/VectorFixed.h"
#include "VectorLibrary/MatrixFixed.h"
#define Viewport(x,w,s) (imul(idiv((x),(w))+IntToFixed(1),IntToFixed((s)/2)))

#include <stdint.h>
#include <string.h>

#include "Blobs.h"

#include "Utils.h"


#include "Graphics/RLEBitmap.h"

#include "Metablobs.h"
#include "Global.h"

#include "BitBin.h"

#define Viewport(x,w,s) (imul(idiv((x),(w))+IntToFixed(1),IntToFixed((s)/2)))

extern const RLEBitmap EndLogo;

void Metablobs()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,320*200);
	memset(framebuffer2,0,320*200);

	const uint8_t *blobImg[6]={
		BLOB1,
		BLOB2,
		BLOB3,
		BLOB4,
		BLOB5,
		BLOB6,
	};
	
	Bitmap frame1,frame2;
	InitializeBitmap(&frame1,320,200,320,framebuffer1);
	InitializeBitmap(&frame2,320,200,320,framebuffer2);

        SetVGAScreenMode320x200_60Hz(framebuffer1);
	
	int frame=0;

        srand(999);
        ivec3_t blobs[4];

        int32_t star_waitstates[300];
        for(int i = 0; i < 300; i++) {
            int32_t rv = rand();
            data.metablobs.stars[i] = ivec3(imul(isin(rv),rand()%F(2)), imul(icos(rv),rand()%F(2)), F(20));
            star_waitstates[i] = iabs(rand() % 200)+27;
        }
        int32_t startframe = VGAFrame;
        while(CurrentBitBinRow(songp) < 256)
	{
                int32_t rotcnt = VGAFrame - startframe;
        
                uint8_t ballpal[255];
                if(rotcnt < 50) {
                    int32_t divider = (8-(rotcnt/8));
                    for(int i = 0; i < 256; i++) {
                        if(i < 64) {
                            ballpal[i] = RawRGB(0,(i/16)/divider,(i/16)/divider);
                        }
                        else if(i < 128) {
                            ballpal[i] = RawRGB(0,((128-i)/16)/divider,((128-i)/16)/(divider/2));
                        }
                        else if(i < 192) {
                            ballpal[i] = RawRGB(((i-128)/16)/divider,0,((i-128)/32)/(divider/2));
                        }
                        else {
                            ballpal[i] = RawRGB(((i-128)/16)/divider,((i-192)/8)/divider,3/(divider/2));
                        }
                    }
                }
                else {
                    for(int i = 0; i < 256; i++) {
                        if(i < 64) {
                            ballpal[i] = RawRGB(0,i/16,i/16);
                        }
                        else if(i < 128) {
                            ballpal[i] = RawRGB(0,(128-i)/16,(128-i)/16);
                        }
                        else if(i < 192) {
                            ballpal[i] = RawRGB((i-128)/16,0,(i-128)/32);
                        }
                        else {
                            ballpal[i] = RawRGB((i-128)/16,(i-192)/8,3);
                        }
                    }
                }
		WaitVBL();
                
		Bitmap *currframe;
		if(frame&1) { currframe=&frame2; SetFrameBuffer(framebuffer1); }
		else { currframe=&frame1; SetFrameBuffer(framebuffer2); }

		ClearBitmap(currframe);
                uint8_t* pixels = currframe->pixels;
                
                // Projection matrix
                imat4x4_t proj = imat4x4diagonalperspective(IntToFixed(45),idiv(IntToFixed(320),IntToFixed(200)),4096,IntToFixed(400));

//                 // Background background
//                 for(int i = -20; i < 20; i++) {
//                     DrawLine(currframe, 0, (600/10)*i, 320/2-5, (60/10)*i + 70, 40);
//                     DrawLine(currframe, 319, (600/10)*i, 320/2+5, (60/10)*i + 70, 40);
//                 }
//                 int idiff = 0;
//                 for(int i = 0; i < 10; i++) {
//                     idiff += i*7;
//                     DrawLine(currframe, 320/2+5+idiff, 0, 320/2+5+idiff, 199, 40);
//                     DrawLine(currframe, 320/2-5-idiff, 0, 320/2-5-idiff, 199, 40);
//                 }
                
                // Background
                for(int i = 0; i < 300; i++) {
                    if(star_waitstates[i] < 0) {
                        if(rotcnt < 130) {
                            data.metablobs.stars[i].z -= F(0.9);
                        }
                        else {
                            data.metablobs.stars[i].z -= F(0.4);                
                        }
                        if(data.metablobs.stars[i].z <= F(1)) {
                            int32_t rv = rand();
                            data.metablobs.stars[i] = ivec3(imul(isin(rv),rand()%F(2)), imul(icos(rv),rand()%F(2)), F(20));
                        }
                        ivec4_t starpos = ivec4(F(data.metablobs.stars[i].x), F(data.metablobs.stars[i].y), F(data.metablobs.stars[i].z), F(1));
                        starpos = imat4x4transform(proj, starpos);
                        int32_t sx = FixedToInt(Viewport(starpos.x,starpos.w,320));
                        int32_t sy = FixedToInt(Viewport(starpos.y,starpos.w,200));
                        if(sx < 320 && sx >= 0 && sy < 200 && sy >= 0) {
                            pixels[320*sy+sx] = 255;
                        }
                        else {
                            int32_t rv = rand();
                            ivec3(imul(isin(rv),rand()%F(2)), imul(icos(rv),rand()%F(2)), F(20));
                        }
                    }
                    else {
                        star_waitstates[i]--;
                    }
                }

                

                int32_t dist;
                if(rotcnt < 130) {
                    dist = 130 - rotcnt;
                }
                else if(CurrentBitBinRow(songp) > 248) {
                    dist = (CurrentBitBinRow(songp)-255);
                }
                else {
                    dist = 0;
                }
                
                // Modelview matrix
                ivec3_t eye = ivec3(
                    imul(isin(rotcnt<<4),IntToFixed(30+3*dist)),
                    imul(icos(rotcnt<<2),IntToFixed(30+3*dist)),
                    imul(icos(rotcnt<<4),IntToFixed(30+3*dist))
                );
                ivec3_t look = ivec3(IntToFixed(0), IntToFixed(0), IntToFixed(0));
                imat4x4_t modelview = imat4x4lookat(
                    eye,
                    look,
                    ivec3(IntToFixed(0), IntToFixed(1), IntToFixed(0))
                );

                // MVP matrix
                imat4x4_t mvp = imat4x4mul(proj, modelview);

                for(int i = 0; i < 4; i++) {
                    blobs[i].x = idiv(isin(rotcnt<<(i+1)),F(2.5));
                    blobs[i].y = idiv(isin(rotcnt<<(i)),F(2.5));
                    blobs[i].z = idiv(icos(rotcnt<<(3-i)),F(2.5));
                    blobs[i].x = ((blobs[i].x + F(2)) % F(4)) - F(2);
                    blobs[i].y = ((blobs[i].y + F(2)) % F(4)) - F(2);
                    blobs[i].z = ((blobs[i].z + F(2)) % F(4)) - F(2);
                }
                
                int blobidx = 0;
                
                for(int x = -10; x <= 10; x+=2) {
                    for(int y = -10; y <= 10; y+=2) {
                        for(int z = -10; z <= 10; z+=2) {
                            int32_t blobval = 0;
                            for(int i = 0; i < 4; i++) {
                                ivec3_t d = ivec3sub(blobs[i], ivec3(F(x)>>4,F(y)>>4,F(z)>>4));
                                int32_t r = ivec3dot(d,d);
                                blobval = blobval + (r < F(0.5)) * (isq(r) - r + F(0.25));
                            }
                            blobval = F(0.2) - blobval;
                            if(blobval < 0) {
                                ivec4_t blobpos = ivec4(F(x), F(y), F(z), F(1));
                                blobpos = imat4x4transform(mvp, blobpos);
                                data.metablobs.blobs[blobidx].x = FixedToInt(Viewport(blobpos.x,blobpos.w,320));
                                data.metablobs.blobs[blobidx].y = FixedToInt(Viewport(blobpos.y,blobpos.w,200));
                                int32_t blobv = FixedToInt(iabs(idiv(blobpos.z,blobpos.w))<<3);
                                blobv = imin(blobv,5);
                                data.metablobs.blobs[blobidx].blob = blobImg[blobv];
                                blobidx++;
                            }
                        }
                    }
                }
		
                for( int i = 0; i < blobidx; i++ ) {
			for( int x = 0; x < 19; x++ ) {
				for( int y = 0; y < 19; y++ ) {
					int fx = data.metablobs.blobs[i].x+x-9;
					int fy = data.metablobs.blobs[i].y+y-9;
					if(fx < 0 || fx >= 320 || fy < 0 || fy >= 200) continue;
					int idx = fx+fy*320;
					uint32_t things = pixels[idx];
					things += data.metablobs.blobs[i].blob[x+y*19];
					things = things > 0xFF ? 0xFF : things;
					pixels[idx] = things;
				}
			}
		}

		
                for( int x = 0; x < 320; x++ ) {
                        for( int y = 0; y < 200; y++ ) {
                                pixels[x+y*320] = ballpal[pixels[x+y*320]];
                        }
                }
		
		frame++;
	}
}

