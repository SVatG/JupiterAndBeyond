
#include "greetstars.h"
#include "Random.h"
#include "Global.h"


#define WIDTH 320
#define HEIGHT 200


void greetstars_init(){
    greetstars_t* stars_a=data.greets.stars_a;
    greetstars_t* stars_b=data.greets.stars_b;
    for(int i=0; i<GREETSTARS_COUNT; ++i){
        stars_a[i].x = RandomInteger()%(320<<8);
        stars_a[i].y = RandomInteger()%(200<<8);
        stars_a[i].delta = (RandomInteger()%500)-100;
        stars_b[i].x = RandomInteger()%(320<<8);
        stars_b[i].y = RandomInteger()%(200<<8);
        stars_b[i].delta = (RandomInteger()%500)-400;
    }
}
void drawstars(Bitmap* dest, greetstars_t stars[], int colour){
    for(int i=0; i<GREETSTARS_COUNT; ++i){
        stars[i].x += stars[i].delta;
        stars[i].y += stars[i].delta/4;
        if(stars[i].x >= (WIDTH<<8)){
            stars[i].x = 0;
        }
        if(stars[i].x < 0){
            stars[i].x = (WIDTH-1)<<8;
        }
        if(stars[i].y >= (HEIGHT<<8)){
            stars[i].y = 0;
        }
        if(stars[i].y < 0){
            stars[i].y = (HEIGHT-1)<<8;
        }

        dest->pixels[(stars[i].x>>8) + WIDTH*(stars[i].y>>8)] = colour;
    }
}

void greetstars_draw(Bitmap* dest){
    drawstars(dest, data.greets.stars_a, (7<<2)+2);
    drawstars(dest, data.greets.stars_b, (7<<5)+2);

}


