/* ASNS */
#include <ctype.h>

#include "header.h"
#include "layer.h"
#include "pseudo-graphics.h"

#include "setup.h"
#include "state.h"
#include "output.h"

#define DEBUG

/* is_count(const char*) return value 'true' if 'str' is
a string of digits, other ways - 'false'  */
bool is_count (const char* str){
    bool flag = true;

    for (int i = 0; str[i] != '\0'; i++){
        flag &= (isdigit(str[i]) & 01);
    }

    return flag;
}

/* Update_layers(int, struct layer*[]) - updates all layers with active instruments */
void Update_layers (int layers_num, struct layer* layers[]){
    
    if (work_modes[M_DRAW] && layers[current_layer]->visible)
        layers[current_layer]->data[cursor.y][cursor.x] = BRUSH;
    if (work_modes[M_ERASE] && layers[current_layer]->visible)
        layers[current_layer]->data[cursor.y][cursor.x] = C_EMPTY;
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

    saved_hash_summ = hash_summ(layers_num, layers);

    saved = true;

    unsigned long long last_hash_summ = hash_summ(layers_num, layers), current_step = 0x0ULL;

    do {
        
        saved = (saved_hash_summ == hash_summ(layers_num, layers));

        if (last_hash_summ != hash_summ(layers_num, layers)){
            last_hash_summ = hash_summ(layers_num, layers);
            history_write(hist, &current_step, layers_num, layers);
        }

        Update_layers(layers_num, layers);
        Update_screen(working_path, layers_num, layers);
        Display_screen();
        
        user_char = getch();

        if (strchr("\x48\x50\x4B\x4D", user_char) != NULL){
            move_cursor(&cursor, user_char);
            continue;
        }

        unsigned char tmp;
        

        switch (user_char){
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

    screen = NULL;
    return 0;
}