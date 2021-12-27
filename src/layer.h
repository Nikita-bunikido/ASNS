/* layer struct, and functions
to work with layers         */

#include "header.h"

struct layer {
    unsigned sx;
    unsigned sy;
    unsigned char** data;
    bool visible;
};

/* new_layer (unsigned, unsigned) - creates new empty layer,
with size 'size_x', 'size_y' and returns it's adress */
struct layer* new_layer (unsigned size_x, unsigned size_y){
    struct layer* new_layer = (struct layer*)malloc(sizeof(struct layer));

    (*new_layer).sx      = size_x;
    (*new_layer).sy      = size_y;
    (*new_layer).visible = true;

    (*new_layer).data = (unsigned char**)malloc(size_y * sizeof(unsigned char*));
    for (int i = 0; i < (*new_layer).sy; i++){
        (*new_layer).data[i] = (unsigned char*)malloc(sizeof(unsigned char) * size_x);

        memset ((*new_layer).data[i], C_EMPTY, sizeof(unsigned char) * size_x);
    }
    return new_layer;
}

/* destroy_layer (struct layer*) - destroys layer 'lr', and
returns it's adress */
struct layer* destroy_layer (struct layer* lr){
    for (int i = 0; i < (*lr).sy; i++)
        free((*lr).data[i]);
    free((*lr).data);

    free(lr);
    return lr;
}

/* Push_layer(struct layer, int*, struct layer**) - pushes layer 'layer'
to the 'num' pos into 'stack', and increments 'num'*/
void push_layer (struct layer* layer, int* num, struct layer** stack){
    stack = (struct layer**)realloc(stack, *num * (sizeof(struct layer*)));
    stack[*num++] = layer;
}