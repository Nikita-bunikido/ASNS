/* functions to work with screen and output */

#include "header.h"

/* in_screen(point) retutn value 'true' if point
is in screen field, other ways - 'false' */
bool in_screen (point p){
    return p.x < SCREEN_W && p.y < SCREEN_H;
}

/* in_working_area(point) return value 'true' if point
is in working area, other ways - 'false' */
bool in_working_area (point p){
    return p.x < WA_W-1 && p.y < WA_H-1 && p.x > 0 && p.y > 0;
}

/* get_format(const char*) - returns format of file 'path'*/
unsigned char get_format (const char* path){
    while (*path++ != '.');
    --path;

    if (!strcmp(path, ".asns"))
        return 'a';
    else if (!strcmp(path, ".txt"))
        return 't';
    else if (!strcmp(path, ".tex"))
        return 'e';
    else if (!strcmp(path, ".texg"))
        return 'g';
    else
        return 'o';
}

/* get_string(unsigned char) - returns index of string with
file format 'format'*/
unsigned int get_string (unsigned char format){
    char* formats = "atego";
    return strchr(formats, format) - formats;
}

/* print_strings(int, ...) - prints strings.
num - number of strings
arguments - char*[] */
void print_strings (int num, ...){
    va_list p;
    va_start(p, num);

    for (int i = 0; i < num; i++)
        printf("%s", va_arg(p, char*));
    va_end(p);
}

/* print_string(const char*, point) - prints string 'string' in
'begin' position on screen array */
void print_string (const char* string, point begin){
    bool in_screen(point);

    const char* p = string;
    int offset = 0;

    while (*p) {
        if (in_screen((point){begin.x + offset, begin.y}))
            screen[begin.y][begin.x + offset] = *p++;
        else
            break;
        ++offset;
    }
}

/* Draw_working_area(int, struct layer*[]) - draws all layers on screen
array. 'num_of_layers' - number of layers will draw */
void Draw_working_area (int num_of_layers, struct layer* layers[], struct layer* select_layer){
    for (int i = 1; i < WA_H-1; i++)
        for (int j = 1; j < WA_W-1; j++)
            screen[i][j] = C_SPACE;
    
    for (int k = 0; k < num_of_layers; k++){
        if (layers[k]->visible != true)
            continue;
        for (int i = 1; i < layers[k]->sy+1; i++)
            for (int j = 1; j < layers[k]->sx+1; j++){
                if (in_working_area ((point){j, i}) && layers[k]->data[i-1][j-1] != C_EMPTY){
                    screen[i][j] = layers[k]->data[i-1][j-1];
                }
            }
    }
    if (work_modes[M_SELECTED]){
        for (int i = 1; i < select_layer->sy+1; i++)
            for (int  j = 1; j < select_layer->sx+1; j++){
                if (in_working_area ((point){j, i}) && select_layer->data[i-1][j-1] != C_EMPTY){
                    screen[i][j] = select_layer->data[i-1][j-1];
                }
            }
    }
}

/* Update_side_bar() - updates sidebar strings and states */
void Update_side_bar (const char* print_path){

    char file_state[256];

    unsigned char get_format (const char* path);
    unsigned int  get_string (unsigned char format);
    
    char* tmp_path = print_path;
    while (*print_path++ != '.')
        ;
    while (*print_path-- != '\\' && *print_path != '/' && print_path >= tmp_path )
        ;
    print_path += (*(print_path + 1) == '\\') ? 2 : 1;

    strcpy(file_state, print_path);
    strcat(file_state, !saved ? "*" : " ");
    strcat(file_state, (char*[]){"- ASNS source", "- ASCII text", "- Texture", "- Gif texture", "- Unknown"}[get_string(get_format(print_path))]);

    print_string (file_state, (point){WA_W + 1, 1U});
    
    /* coordinates */
    print_string ("[X  Y  ]", (point){WA_W + 1, 3U});
    char x[32], y[32];
    itoa(cursor.x, x, 10);
    itoa(cursor.y, y, 10);
    print_string (x, (point){WA_W + 3, 3U});
    print_string (y, (point){WA_W + 6, 3U});

    /* modes */
    for (int i = 0; i < 3; i++){
        print_string((char*[]){"[ ] - draw \' \'", "[ ] - erase", "[ ] - select"}[i], (point){WA_W + 1, 4U + i});
        print_string (work_modes[i] ? "^" : " ", (point){WA_W + 2, 4U + i});
    }

    /* modes + */
    print_string((char[]){BRUSH, '\0'}, (point){WA_W + 13, 4U});
}

/* Display_screen() - outputs screen array into real screen */
void Display_screen (){
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){0,0});
    for (int i = 0; i < SCREEN_H; i++)
        print_strings(2, screen[i], "\n");
}

/* Update_screen(const char*, int, struct layer*[]) - updates sidebar and working area, draws 'path' */
void Update_screen (const char* path, int layers_num, struct layer* layers[], struct layer* select_layer){
    Draw_working_area(layers_num, layers, select_layer);
    Update_side_bar(path);

    /* cursor drawing */
    if (!work_modes[M_SELECTED])
        screen[cursor.y+1][cursor.x+1] = 219;
}