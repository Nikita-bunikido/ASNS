/* program initialization and startup 
files save and parse */

#include "header.h"

/* Init(int*, char *(*[]), bool*) - checks arguments, set flags to working modes*/
char* Init (int *argc, char *(*argv[]), bool* can_parse){

    char* return_path = malloc(256);
    
    if (*argc < 2){
        fprintf(stdout, "asns.error: no input files.\n");
        exit(1);
    }

    if (*argc > 2){
        fprintf(stdout, "asns.info: usage: asns <file_name>\n");
        exit(1);
    }

    if (!strcmp((*argv)[true], "-version")){
        fprintf(stdout, "ASNS (asns) v%s.\nCreated by Nikita Donskov.\n2021\n", ASNS_VERSION);
        exit(0);
    }

    strcpy(return_path, (*argv)[true]);
    FILE* file = fopen(return_path, "r");

    /* can't open file */
    if (file == NULL){
        *can_parse = false;
        fprintf(stdout, "asns.warning: file \'%s\' does not exist.\nDo you want to create new file? [Y/N]\n> ", return_path);
        
        fclose(file);
        char choose = getchar();
        if (choose == 'Y') {

            signed int safeh, safew;
            fprintf(stdout, "w.a. width > ");
            scanf("\n%d", &safew);
            fprintf(stdout, "w.a. height > ");
            scanf("\n%d", &safeh);

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
            
            file = fopen(return_path, "w");
            
            fprintf(file, "ASNS_EMPTY_FILE");
            /* setting screen sizes as work area sizes with offsets */
            SCREEN_W = (WA_W = (unsigned int)safew) + 25;
            SCREEN_H = (WA_H = (unsigned int)safeh);
    
            } else { fprintf(stdout, "asns.info: process terminated.\n"); exit(0); }
    } else 
        *can_parse = true;
    fclose(file);

    return return_path;
}

/* hash_summ(int, struct layer*[]) - returns
value of hash summ of layers. 'num' - number of layers */
unsigned long long hash_summ (int num, struct layer* layers[]){
    unsigned long long result = 0x0ULL;

    result += (num * 9173ULL) - 10283ULL;
    for (int k = 0; k < num; k++){
        result += layers[k]->visible * 5;
        for (int i = 0; i < layers[k]->sy; i++)
            for(int j = 0; j < layers[k]->sx; j++)
                result += layers[k]->data[i][j] * 708ULL;
    }

    return result % 1000000000ULL;
}

/* Save_file (int, struct layer*[], const char, unsigned char) - saves
file to 'save_path' with 'format' format option. */
void Save_file (int num, struct layer* layers[], const char* save_path, unsigned char format){
    unsigned long long hash_summ(int, struct layer*[]);
    
    if (num <= 0)
        return;  //no layers

    FILE* save;
    switch (format){
    case 'a': /* asns format */
        save = fopen(save_path, "wb");

        fwrite(&num, sizeof(num), 1, save);

        for (int k = 0; k < num; k++){
            fwrite(&layers[k]->visible, sizeof(bool),1, save);
            fwrite(&layers[k]->sx, sizeof(unsigned), 1, save);
            fwrite(&layers[k]->sy, sizeof(unsigned), 1, save);
        
        for (int i = 0; i < layers[k]->sy; i++)
            for (int j = 0; j < layers[k]->sx; j++){
                fwrite(&layers[k]->data[i][j], sizeof(unsigned char), 1, save);
            }
        }
        fclose(save);
        break;
    case 't': /* ascii text format */
        save = fopen(save_path, "w");

        _standart_ascii_save:
        for (int i = 0; i < layers[0]->sy; i++){
            for (int j = 0; j < layers[0]->sx; j++){
                int tmp = num;
                unsigned char result = layers[tmp-1]->data[i][j];

                while (tmp > 0 && (layers[tmp-1]->data[i][j] == C_EMPTY || layers[tmp-1]->visible == false)){
                    result = layers[tmp-1]->data[i][j];
                    tmp--;
                }
                if (result == C_EMPTY)
                    result = C_SPACE;
                fprintf(save, "%c", result);
            }
            fprintf(save, "\n");
        }
        fclose(save);
        break;
    case 'e': /* texture format */
        save = fopen(save_path, "w");
        fprintf(save, "0x%llx: %u %u\n", hash_summ (num, layers), layers[0]->sy, layers[0]->sx);

        goto _standart_ascii_save;

        break;
    case 'g': /* animated texture format */
        fprintf(stdout, "asns.info:animated texture format (.texg) isn't avaiable in this asns version.\nSave process terminated.\n");
        break;
    case 'o': /* user format */
        save = fopen(save_path, "w");
        goto _standart_ascii_save;
        break;
    }
}

/* Parse(const unsigned char, char*, int*, struct layer*[])
- reads file with format 'format' by 'path'. Writes data to 'layers'
and 'num' */
void Parse(const unsigned char format, char* parse_path, int* num, struct layer* layers[]){
    
    FILE* parsed_file = NULL;

    switch (format){
    case 'a': /* asns format */
        parsed_file = fopen(parse_path, "r");
        char buf[256];

        fgets(buf, 256, parsed_file);

        if (!strcmp(buf, "ASNS_EMPTY_FILE")){
            fprintf(stdout, "asns.error: can\'t parse empty file.\n");
            free(layers);
            free(parse_path);
            exit(1);
        }

        fclose(parsed_file);

        parsed_file = fopen(parse_path, "rb");

        if(fread(num, sizeof(int), 1, parsed_file) <= 0){
            fprintf(stdout, "asns.error: file is empty\\can\'t read from file.\n");
            free(layers);
            free(parse_path);
            exit(1);
        }

        if(*num <= 0){
            fprintf(stdout, "asns.error: zero layers.\n");
            free(layers);
            free(parse_path);
            exit(1);
        }

        int k;
        for (k = 0; k < *num; k++){
            signed int tmp_w, tmp_h;
            bool tmp_vis;
            fread(&tmp_vis,   sizeof(bool), 1, parsed_file);
            fread(&tmp_w, sizeof(unsigned), 1, parsed_file);
            fread(&tmp_h, sizeof(unsigned), 1, parsed_file);

            layers[k] = new_layer(tmp_w, tmp_h);
            
        for (int i = 0; i < layers[k]->sy; i++)
            for (int j = 0; j < layers[k]->sx; j++){
                fread(&layers[k]->data[i][j], sizeof(unsigned char), 1, parsed_file);
            }
        }
        fclose(parsed_file);

        if (layers[0]->sx + 2 <= 10 || layers[0]->sy + 2 <= 10){
            printf("asns.error: can\'t parse file because it's too small: [%d, %d].\n", layers[0]->sx+2, layers[0]->sy+2);
            free(layers);
            free(parse_path);
            exit(1);
        }

        /* setting screen sizes as work area sizes with offsets */
        SCREEN_W = (WA_W = (unsigned int)layers[0]->sx + 2) + 25;
        SCREEN_H = (WA_H = (unsigned int)layers[0]->sy + 2);
    
        break;
    default:
        fprintf(stdout, "asns.warning: can't parse file - unknown format.\n");
    }
    return;
}

/* Screen_setup() - mallocing screen array, draws ui base on screen */
void Screen_setup(){
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

    print_string ("asns 0.1", (point){WA_W + 1, 0U});
    return;
}