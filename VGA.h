#ifndef __VGA_H__
#define __VGA_H__

#include <stdint.h>

void InitializeVGA(uint8_t *framebuffer);
void WaitVBL();

#endif
