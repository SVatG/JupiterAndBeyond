#ifndef __FIELDS_H__
#define __FIELDS_H__

#include <stdint.h>

struct FieldsData
{
	uint8_t palette[32],palette2[32],palette3[32],palette4[32];
};

void Fields();

#endif

