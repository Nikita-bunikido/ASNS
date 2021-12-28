/* asns - global header */

#if !defined (ASNS_HEADER)

#define ASNS_HEADER

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <windows.h>
#include <conio.h>

#define ASNS_VERSION    "0.1 \'demo\'"

#define C_SPACE     (unsigned char)32 /* empty char */
#define C_EMPTY     (unsigned char)0  /* layer trancperent char */

typedef struct {
    unsigned x;
    unsigned y;
} point;

enum modes {
    M_DRAW,
    M_ERASE,
    M_SELECTED
};

extern unsigned char **screen;
extern unsigned int SCREEN_W, SCREEN_H, WA_W, WA_H;
extern unsigned int current_layer;
extern unsigned char BRUSH;
extern bool work_modes[3];
extern point cursor;
extern unsigned char format;
extern bool saved;
extern unsigned long long saved_hash_summ;
extern bool area_moving;

#endif