/* global program data */

#include "header.h"
unsigned char **screen = NULL;
/* screen size, working area size */
unsigned int SCREEN_W, SCREEN_H, WA_W, WA_H;

unsigned int current_layer = 0U;
unsigned char BRUSH = '#';

//work modes flags
bool work_modes[3] = {false};

//global program cursor
point cursor = {0, 0};

//file format global
unsigned char format = 't';

//file saved
bool saved = true;
unsigned long long saved_hash_summ = 0ULL;