#ifndef __METABLOBS_H__
#define __METABLOBS_H__

void Metablobs();

#define numBlobs 400

typedef struct {
	int32_t x;
	int32_t y;
	const uint8_t* blob;
} Blob;

struct MetablobsData {
	Blob blobs[numBlobs];
        ivec3_t stars[300];
};

#endif
