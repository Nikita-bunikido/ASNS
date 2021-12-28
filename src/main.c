/* ASNS */
#include <ctype.h>

#include "header.h"
#include "layer.h"
#include "pseudo-graphics.h"

#include "setup.h"
#include "state.h"
#include "output.h"

#define MAX_MEM_CELLS   5

#define DEBUG

/*swap(unsigned char*,unsigned char*) - swaps to counts */
void swap (unsigned char* x, unsigned char* y){
    *x = *x ^ *y;
    *y = *x ^ *y;
    *x = *x ^ *y;
}

/* is_count(const char*) return value 'true' if 'str' is
a string of digits, other ways - 'false'  */
bool is_count (const char* str){
    bool flag = true;

    for (int i = 0; str[i] != '\0'; i++){
        flag &= (isdigit(str[i]) & 01);
    }

    return flag;
}

/* move_layer(struct layer*, int) - moves layer data (ilayer) in direction (dir) */
void move_layer (struct layer* ilayer, int dir){
    switch (dir){
        case '\x48': /* up */
            for (int i = 0; i < ilayer->sy-1; i++)
                for (int j = 0; j < ilayer->sx; j++)
                    swap(&ilayer->data[i][j], &ilayer->data[i+1][j]);
            break;
        case '\x50': /* down */
            for (int i = ilayer->sy-1; i >= 1; i--)
                for (int j = 0; j < ilayer->sx; j++)
                    swap(&ilayer->data[i][j], &ilayer->data[i-1][j]);
            break;
        case '\x4B': /* left */
            for (int i = 0; i < ilayer->sy; i++)
                for (int j = 0; j < ilayer->sx-1; j++)
                    swap(&ilayer->data[i][j], &ilayer->data[i][j+1]);
            break;
        case '\x4D': /* right */
            for (int i = 0; i < ilayer->sy; i++)
                for (int j = ilayer->sx-1; j >= 1; j--)
                    swap(&ilayer->data[i][j], &ilayer->data[i][j-1]);
            break;
    }
}

/* Update_layers(int, struct layer*[]) - updates all layers with active instruments */
void Update_layers (int layers_num, struct layer* layers[], struct layer* select_layer, struct layer* tmp){

    if (work_modes[M_DRAW] && layers[current_layer]->visible)
        layers[current_layer]->data[cursor.y][cursor.x] = BRUSH;
    if (work_modes[M_ERASE] && layers[current_layer]->visible)
        layers[current_layer]->data[cursor.y][cursor.x] = C_EMPTY;
    if (work_modes[M_SELECTED] && !area_moving)
        select_layer->data[cursor.y][cursor.x] = 219;
}

/* move_cursor(point*, int) - moves cursor, with user input 'input'*/
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
    
    FILE* hist = history_setup("sys\\.hist");
    bool can_parse = false;
    char* working_path = Init(&argc, &argv, &can_parse);
    memset(work_modes, false, sizeof(work_modes));


    int layers_num = 1;
    struct layer** layers = malloc(sizeof(struct layer*));

    char user_char = '\0';

    if (can_parse == true){ //parsing
        Parse(get_format(working_path), working_path, &layers_num, layers);
    } else {
        layers[0] = new_layer(WA_W-2, WA_H-2);
    }
    
    #ifdef DEBUG
        printf("%d %d %d %d\n", WA_W, WA_H, SCREEN_W, SCREEN_H);
    #endif

    if (WA_W <= 0 || WA_H <= 0 || SCREEN_W <= 0 || SCREEN_H <= 0){
        fprintf(stdout, "asns.error: can\'t make screen or work area.\n");
        free(working_path);
        free(layers);
        exit(1);
    }
    

    Screen_setup();

    system("cls");

    /* selection mode */
    struct layer* selected = new_layer(WA_W-2, WA_H-2);
        
    /* temp layer */
    struct layer* temp = new_layer(WA_W-2, WA_H-2);

    /* memory */
    struct layer* mem[MAX_MEM_CELLS];
    for (int l = 0; l < MAX_MEM_CELLS; ++l)
        mem[l] = new_layer(WA_W-2, WA_H-2);


    saved_hash_summ = hash_summ(layers_num, layers);
    saved = true;

    unsigned long long last_hash_summ = hash_summ(layers_num, layers), current_step = 0x0ULL;

    do {
        
        saved = (saved_hash_summ == hash_summ(layers_num, layers));

        if (last_hash_summ != hash_summ(layers_num, layers)){
            last_hash_summ = hash_summ(layers_num, layers);
            history_write(hist, &current_step, layers_num, layers);
        }

        Update_layers(layers_num, layers, selected, temp);
        Update_screen(working_path, layers_num, layers, selected);
        Display_screen();
        
        user_char = getch();

        /* cursor move or area move */
        if (strchr("\x48\x50\x4B\x4D", user_char) != NULL){
            if(area_moving){
                move_layer(temp, user_char);
                move_layer(selected, user_char);
            } else
                move_cursor(&cursor, user_char);
            continue;
        }

        /* finish moving area */
        if (area_moving && user_char == 13){
            for (int i = 0; i < layers[current_layer]->sy; i++)
                for (int j = 0; j < layers[current_layer]->sx; j++)
                    if (temp->data[i][j] != C_EMPTY)
                        layers[current_layer]->data[i][j] = temp->data[i][j];
            area_moving = false;

            goto transform;
        }

        unsigned char tmp;
        
        switch (user_char){
        case 's':
            for (int i = 0; i < selected->sy; i++)
                for (int j = 0; j < selected->sx; j++)
                    selected->data[i][j] = C_EMPTY;

            work_modes[M_SELECTED] = !work_modes[M_SELECTED];
            if (work_modes[M_ERASE] && work_modes[M_SELECTED])
                work_modes[M_ERASE] = false;
            if (work_modes[M_DRAW] && work_modes[M_SELECTED])
                work_modes[M_DRAW] = false;

            if (work_modes[M_SELECTED]){
                /*
                fprintf(stdout, "Select mode:\n"
                "s - standart mode\n"
                "a - select all\n"
                "c - select characters\n"
                "r - rectangle selecting\n"
                "c - circle selecting\n> ");
                scanf("\n%c", &tmp);

                switch (tmp){
                case 's':
                    break;
                default:
                    fprintf(stdout, "selection error: unknown mode.\n");
                    getchar();
                    system("cls");
                    break;
                }*/
                
                fprintf(stdout, "selection mode activated.\n");
            } else
                system("cls");
            break;
        case 't':
            if (!work_modes[M_SELECTED]){
                fprintf(stdout, "transformation error: no select mode active.\n");
                getchar();
                system("cls");
                break;
            }

            transform:
            fprintf(stdout, "Transform selected area:\n"
            "f - replace (fill by one character)\n"
            "x - horizontal flip\n"
            "y - vertical flip\n"
            "d - delete selected area\n"
            "c - copy selected area\n"
            "u - cut selected area\n"
           /* "r - rotate selected area\n"*/
            "m - move selected area\n"
            "e - end of trandformation & deactivate select mode\n> ");

            scanf("\n%c", &tmp);

            int sx, sy, fx, fy;
            sx = sy = +0xFFFFF;
            fx = fy = -0xFFFFF;

            switch (tmp) {
            case 'f': /* fill area */
                fprintf(stdout, "fill character > ");
                scanf("\n%c", &tmp);
                for (int i = 0; i < selected->sy; i++)
                    for (int j = 0; j < selected->sx; j++)
                        if (selected->data[i][j] != C_EMPTY)
                            layers[current_layer]->data[i][j] = tmp;
                
                system("cls");
                break;
            case 'x': /* x flip */
                sx = sy = +0xFFFFF;
                fx = fy = -0xFFFFF;

                for (int i = 0; i < selected->sy; i++)
                    for (int j = 0; j < selected->sx; j++)
                        if (selected->data[i][j] != C_EMPTY){
                            if (j < sx) sx = j;
                            if (j > fx) fx = j;
                            if (i < sy) sy = i;
                            if (i > fy) fy = i;
                        }

                int size_x = fx - sx;

                for (int i = sy; i < fy+1; i++)
                    for (int j = sx; j < sx + size_x / 2; j++){
                        swap(&layers[current_layer]->data[i][j], &layers[current_layer]->data[i][fx - (j - sx)]);
                        swap(&selected->data[i][j], &selected->data[i][fx - (j - sx)]);
                    }
                system("cls");
                break;

            case 'y': /* y flip */
                sx = sy = +0xFFFFF;
                fx = fy = -0xFFFFF;

                for (int i = 0; i < selected->sy; i++)
                    for (int j = 0; j < selected->sx; j++)
                        if (selected->data[i][j] != C_EMPTY){
                            if (j < sx) sx = j;
                            if (j > fx) fx = j;
                            if (i < sy) sy = i;
                            if (i > fy) fy = i;
                        }

                int size_y = fy - sy;

                for (int i = sy; i < sy + size_y / 2 + 1; i++)
                    for (int j = sx; j < fx; j++){
                        swap(&layers[current_layer]->data[i][j], &layers[current_layer]->data[fy - (i - sy)][j]);
                        swap(&selected->data[i][j], &selected->data[fy - (i - sy)][j]);
                    }
                system("cls");
            
                break;
            case 'd': /* delete area */
                for (int i = 0; i < selected->sy; i++)
                    for (int j = 0; j < selected->sx; j++)
                        if (selected->data[i][j] != C_EMPTY)
                            layers[current_layer]->data[i][j] = C_EMPTY;
                system("cls");
                break;
            case 'u': /* cut and copy area to buffer */
                for (int i = 0; i < layers[current_layer]->sy; i++)
                    for (int j = 0; j < layers[current_layer]->sx; j++)
                        layers[current_layer]->data[i][j] = C_EMPTY;
            case 'c': /* copy area to buffer */
                system("cls");
                fprintf(stdout, "memory buffers:\n");
                for (int i = 0; i < MAX_MEM_CELLS; i++){
                    fprintf(stdout, "[%d] %8s - 0x%llx\n", i+1, mem[i]->visible ? "AVAIABLE" : "STORES", hash_summ(1, (struct layer*[]){mem[i]}));
                }
                fprintf(stdout, "Buffer > ");
                int bn;

                scanf("\n%d", &bn);

                if (!(bn > 0 && bn <= MAX_MEM_CELLS)){
                    fprintf(stdout, "Memory error: no such buffer to write.\n");
                    getchar();
                    getchar();
                    break;
                }

                --bn;

                mem[bn]->visible = false;
                for (int i = 0; i < mem[bn]->sy; i++)
                    for (int j = 0; j < mem[bn]->sx; j++)
                        mem[bn]->data[i][j] = (selected->data[i][j] != C_EMPTY) ? layers[current_layer]->data[i][j] : C_EMPTY;

                fprintf(stdout, "Successfully written to buffer [%d]\n", bn + 1);
                getchar();
                getchar();
                system("cls");
                break;
            case 'm': /* move area */
                area_moving = true;
                for (int i = 0; i < selected->sy; i++)
                    for (int j = 0; j < selected->sx; j++)
                        if (selected->data[i][j] != C_EMPTY){
                            temp->data[i][j] = layers[current_layer]->data[i][j];
                            layers[current_layer]->data[i][j] = C_EMPTY;
                        } else
                            temp->data[i][j] = C_EMPTY;
                system("cls");
                break;
            case 'e': /* exit from transform & selection mode */
                work_modes[M_SELECTED] = false;
                system("cls");
                break;
                

            default:
                fprintf(stdout, "transform error: unknown command.\n");
                getchar();
                system("cls");
                break;
            }
            break;
        case 'z': /* step back */
            if (current_step == 0){
                fprintf(stdout, "no changes yet, nothing to restore.\n");
                getchar();
                system("cls");
                break;
            }
            history_step_back(hist, "sys\\.hist", &current_step, &layers_num, layers);
            last_hash_summ = hash_summ(layers_num, layers);
            fprintf(stdout, "last change deleted.\n");
            getchar();
            system("cls");
            break;
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
            char ctmp[64];
            fprintf(stdout, "new brush\n\'-ascii\' to choose by code\n> ");
            scanf("\n%s", ctmp);

            if (!strcmp(ctmp, "-ascii")){
                unsigned code;
                
                fprintf(stdout, "ascii code > ");
                scanf("\n%u", &code);
                BRUSH = code;
                system("cls");
                break;
            }

            BRUSH = *ctmp;
            system("cls");
            break;
        case 'a': /* file saving */ {
            fprintf(stdout, "Save as or save [A/S]\n> ");
            scanf("\n%c", &tmp);

            if (tmp == 'S'){
                Save_file(layers_num, layers, working_path, get_format(working_path));
                saved = true;
                saved_hash_summ = hash_summ(layers_num, layers);
                system("cls");
                break;
            }

            char path[255U], ctmp[64];
            fprintf(stdout, "Path to save\n \'-standart\' to save into standart directory\n\'-none' to save into program directory\n> ");
            scanf("%s", ctmp);

            if (!strcmp(ctmp, "-standart"))
                strcpy(path, "saved\\");
            else if (!strcmp(ctmp, "-none"))
                strcpy(path, "");
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

            saved = true;
            saved_hash_summ = hash_summ(layers_num, layers);

            Save_file(layers_num, layers, path, tmp);
            fprintf(stdout, "Saved to \'%s\'.\n", path);
            fprintf(stdout, "Press [enter] to continue...");
            getchar();
            getchar();
            system("cls");
        } break;
        }
        
    } while (user_char != 'q');

    /* closing history file */
    fclose(hist);

    /* destroying layers */
    for (int i = 0; i < layers_num; i++)
        layers[i] = (destroy_layer (layers[i]), NULL);

    /* destroying screen */
    for (int i = 0; i < SCREEN_H; i++)
        free(screen[i]);
    free(screen);

    free(working_path);

    selected = (destroy_layer (selected), NULL);
    temp     = (destroy_layer (temp), NULL);
    
    for (int l = 0; l < MAX_MEM_CELLS; l++)
        mem[l] = (destroy_layer (mem[l]), NULL);
    screen = NULL;
    return 0;
}