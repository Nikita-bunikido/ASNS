#include "header.h"
#include "layer.h"
#include "pseudo-graphics.h"

unsigned char **screen = NULL;
unsigned int SCREEN_W, SCREEN_H, WA_W, WA_H;

unsigned int current_layer = 0U;

typedef struct {
    unsigned x;
    unsigned y;
} point;

struct {
    bool drawing;
} work_modes = {false};

point cursor = {0, 0};

void print_strings (int num, ...){
    va_list p;
    va_start(p, num);

    for (int i = 0; i < num; i++)
        printf("%s", va_arg(p, char*));
    va_end(p);
}

bool in_screen (point p){
    return p.x < SCREEN_W && p.y < SCREEN_H;
}

bool in_working_area (point p){
    return p.x < WA_W-1 && p.y < WA_H-1 && p.x > 0 && p.y > 0;
}

void print_string (const char* string, point begin){
    const char* p = string;
    int offset = 0;

    while (*p) {
        if (in_screen((point){begin.x + offset, begin.y}))
            screen[begin.y][begin.x + offset] = *p++;
        ++offset;
    }
}

void Init_screen (int *argc, char *(*argv[])){

    bool default_size = !strcmp((*argv)[1], "-default");

    if (*argc < 3 && !default_size){
        printf("usage: asns <w.a. width> <w.a. height>\n");
        exit(0);
    }

    signed int safew = atoi((*argv)[1]), 
               safeh = atoi((*argv)[2]);

    if (!default_size){
    #define MIN_AREA_SIZE   (point){10,10}

    if (safew <= 0){
        fprintf(stderr, "asns.error: lower or equal to \'0\' w.a. width: %d.\nProcess terminated.\n", safew);
        exit(1);
    } else if (safeh <= 0){
        fprintf(stderr, "asns.error: lower or equal to \'0\' w.a. height: %d.\nProcess terminated.\n", safeh);
        exit(1);
    } else if (safew <= MIN_AREA_SIZE.x){
        fprintf(stderr, "asns.error: lower than min size \'%d\' w.a. width: %d\nProcess terminated.\n", MIN_AREA_SIZE.x, safew);
        exit(1);
    } else if (safeh <= MIN_AREA_SIZE.y){
        fprintf(stderr, "asns.error: lower than min size \'%d\' w.a. height: %d\nProcess terminated.\n", MIN_AREA_SIZE.y, safeh);
        exit(1);
    }
    #undef MIN_AREA_SIZE

    } /* if default sizes */
    else {
        safew = 13U;
        safeh = 11U;
    }

    /* setting screen sizes as work area sizes with offsets */
    SCREEN_W = (WA_W = (unsigned int)safew) + 25;
    SCREEN_H = (WA_H = (unsigned int)safeh);
    
    /* allocate dynamic 'screen' array */
    screen = (unsigned char**)malloc(sizeof(unsigned char*) * SCREEN_H);

    for (int k = 0; k < SCREEN_H; k++){
        screen[k] = malloc(sizeof(unsigned char) * (SCREEN_W + 1));

        memset(screen[k], C_SPACE, SCREEN_W);
        screen[k][SCREEN_W] = '\0';
    }

    /* work area drawing */
    for (int i = 0; i < WA_H; i++){
        for (int j = 0; j < WA_W; j++){
            screen[i][j] = get_border(j, i, WA_W, WA_H, true);
        }
    }

    /* separator */
    screen[2][WA_W-1] = 0xC7U;
    for (int i = WA_W; i < SCREEN_W; i++)
        screen[2][i] = 196U;

    /* size bar strings printing */
    void print_string (const char* string, point begin);

    print_string ("asns v0.1", (point){WA_W + 1, 0U});
    print_string ("test.txt* - ASCII text", (point){WA_W + 1, 1U});
}

void Draw_working_area (int num_of_layers, struct layer* layers[]){
    for (int i = 1; i < WA_H-1; i++)
        for (int j = 1; j < WA_W-1; j++)
            screen[i][j] = C_SPACE;
    
    for (int k = 0; k < num_of_layers; k++){

        for (int i = 1; i < layers[k]->sy+1; i++)
            for (int j = 1; j < layers[k]->sx+1; j++){
                if (in_working_area ((point){j, i}) && layers[k]->data[i-1][j-1] != C_EMPTY){
                    screen[i][j] = layers[k]->data[i-1][j-1];
                }
            }
    }
}

void Update_screen (int layers_num, struct layer* layers[]){
    Draw_working_area(layers_num, layers);

    /* cursor drawing */
    screen[cursor.y+1][cursor.x+1] = 219;
}

void Display_screen (){
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){0,0});
    for (int i = 0; i < SCREEN_H; i++)
        print_strings(2, screen[i], "\n");
}

void move_cursor (point *cur, int input){
    switch (input){
        case '\x48': /* up */
            if (in_working_area ((point){cur->x+1, cur->y-1+1}))
                cur->y--;
            break;
        case '\x50': /* down */
            if (in_working_area ((point){cur->x+1, cur->y+1+1}))
                cur->y++;
            break;
        case '\x4B': /* left */
            if (in_working_area ((point){cur->x-1+1, cur->y+1}))
                cur->x--;
            break;
        case '\x4D': /* right */
            if (in_working_area ((point){cur->x+1+1, cur->y+1}))
                cur->x++;
            break;
    }
}

int main (int argc, char* argv[]){
    system("cls");
    Init_screen(&argc, &argv);

    struct layer* main_layer = new_layer(WA_W-2, WA_H-2);
    
    char user_char = '\0';

    do {
        Update_screen(1, (struct layer*[]){main_layer});
        Display_screen();
        
        user_char = getch();

        if (strchr("\x48\x50\x4B\x4D", user_char) != NULL)
            move_cursor(&cursor, user_char);

        
    } while (user_char != 'q');

    /* destroying layers */
    main_layer = (destroy_layer (main_layer), NULL);

    /* destroying screen */
    for (int i = 0; i < SCREEN_H; i++)
        free(screen[i]);
    free(screen);

    screen = NULL;
    return 0;
}