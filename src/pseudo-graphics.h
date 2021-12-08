#include "header.h"

enum { UL, UR, DL, DR, GL, VR, DU, DD, DER, DEL };

static const unsigned char clui_shades[] = {
    176, 177,
    178, 219
};

static const unsigned char bls_w[2][10] = {
  { 218, 191,
    192, 217,
    196, 179,
    193, 194,
    195, 180 },
  { 201, 187,
    200, 188,
    205, 186,
    202, 203,
    204, 185 }
};

static const unsigned char bt_sh[] = { //buttons shade
    0xB7, 0xD4,
    0xB8, 0xD3
};

unsigned char get_border (int i, int j, int sx, int sy, bool width){
    if (i == 0    && j ==    0) return bls_w[width][UL];
    if (i == sx-1 && j ==    0) return bls_w[width][UR];
    if (i == 0    && j == sy-1) return bls_w[width][DL];
    if (i == sx-1 && j == sy-1) return bls_w[width][DR];

    if (i == 0 || i == sx-1) return bls_w[width][VR];
    if (j == 0 || j == sy-1) return bls_w[width][GL];

    return 32;
}