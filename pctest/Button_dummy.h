#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdbool.h>

void InitializeUserButton();
void EnableUserButtonInterrupt();

static inline bool UserButtonState()
{
	return false;
}

#endif

