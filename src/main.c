#include "header.h"
#include "layer.h"
#include "pseudo-graphics.h"

enum modes {
    M_DRAW,
    M_ERASE
};

unsigned char **screen = NULL;
unsigned int SCREEN_W, SCREEN_H, WA_W, WA_H;

unsigned int current_layer = 0U;
unsigned char BRUSH = '#';

typedef struct {
    unsigned x;
    unsigned y;
} point;

bool work_modes[2] = {false};

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

    
    bool default_size;
    if (*argc > 1)
        default_size = !strcmp((*argv)[1], "-default");

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

void Update_side_bar (){
    print_string ("test.txt* - ASCII text", (point){WA_W + 1, 1U});
    
    /* coordinates */
    print_string ("[X  Y  ]", (point){WA_W + 1, 3U});
    char x[32], y[32];
    itoa(cursor.x, x, 10);
    itoa(cursor.y, y, 10);
    print_string (x, (point){WA_W + 3, 3U});
    print_string (y, (point){WA_W + 6, 3U});

    /* modes */
    for (int i = 0; i < 2; i++){
        print_string((char*[]){"[ ] - draw \' \'", "[ ] - erase"}[i], (point){WA_W + 1, 4U + i});
        print_string (work_modes[i] ? "^" : " ", (point){WA_W + 2, 4U + i});
    }

    /* modes + */
    print_string((char[]){BRUSH, '\0'}, (point){WA_W + 13, 4U});
}

void Update_screen (int layers_num, struct layer* layers[]){
    Draw_working_area(layers_num, layers);
    Update_side_bar();

    /* cursor drawing */
    screen[cursor.y+1][cursor.x+1] = 219;

}

void Update_layers (int layers_num, struct layer* layers[]){
    
    if (work_modes[M_DRAW])
        layers[current_layer]->data[cursor.y][cursor.x] = BRUSH;
    if (work_modes[M_ERASE])
        layers[current_layer]->data[cursor.y][cursor.x] = C_EMPTY;
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

void push_layer (struct layer* layer, int* num, struct layer** stack){
    stack = (struct layer**)realloc(stack, *num * (sizeof(struct layer*)));
    stack[*num++] = layer;
}

int main (int argc, char* argv[]){
    system("cls");
    Init_screen(&argc, &argv);
    memset(work_modes, false, sizeof(work_modes));


    int layers_num = 1;
    struct layer** layers = malloc(sizeof(struct layer*));
    layers[0] = new_layer(WA_W-2, WA_H-2);

    char user_char = '\0';

    do {
        Update_layers(1, layers);
        Update_screen(1, layers);
        Display_screen();
        
        user_char = getch();

        if (strchr("\x48\x50\x4B\x4D", user_char) != NULL){
            move_cursor(&cursor, user_char);
            continue;
        }

        unsigned char tmp;

        switch (user_char){
        case 'd': /* drawing mode switch */
            work_modes[M_DRAW] = !work_modes[M_DRAW];
            if (work_modes[M_ERASE] && work_modes[M_DRAW])
                work_modes[M_ERASE] = false;
            break;
        case 'e': /* erase mode switch */
            work_modes[M_ERASE] = !work_modes[M_ERASE];
            if (work_modes[M_ERASE] && work_modes[M_DRAW])
                work_modes[M_DRAW] = false;
            break;
        case 'b': /* brush setting */
            fprintf(stdout, "new brush > ");
            scanf("\n%c", &tmp);

            BRUSH = tmp;
            system("cls");
            break;
        case 'a': /* file saving */ {
            char path[255U], ctmp[64];
            fprintf(stdout, "Path to save > ");
            scanf("%s", ctmp);

            if (!strcmp(ctmp, "standart"))
                strcpy(path, "saved\\");
            else
                strcpy(path, ctmp);

            fprintf(stdout, "File name > ");
            scanf("%s", ctmp);
            strcat(path, ctmp);

            fprintf(stdout, "File format:\na - .asns (ASNS STANDART FORMAT)\nt - .txt (ASCII TEXT)\ne - .tex (TEXTURE)\ng - .texg (ANIMATED TEXTURE)\no - other format.\n> ");
            scanf("\n%c", &tmp);

            switch (tmp) {
            case 'a':
                strcat(path, ".asns");
                break;
            case 't':
                strcat(path, ".txt");
                break;
            case 'e':
                strcat(path, ".tex");
                break;
            case 'g':
                strcat(path, ".texg");
                break;
            case 'o':
                fprintf(stdout, "Your format > ");
                scanf("%s", ctmp);

                strcat(path, ctmp);
            }

            fprintf(stdout, "Saving to \'%s\'.\n", path);
            Sleep(1500);
            system(" cls" + 1);
        } break;
        }
        
    } while (user_char != 'q');

    /* destroying layers */
    for (int i = 0; i < layers_num; i++)
    layers[i] = (destroy_layer (layers[i]), NULL);

    /* destroying screen */
    for (int i = 0; i < SCREEN_H; i++)
        free(screen[i]);
    free(screen);

    screen = NULL;
    return 0;
}