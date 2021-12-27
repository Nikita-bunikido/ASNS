#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool is_count (const char* str){
    bool flag = true;

    for (int i = 0; str[i] != '\0'; i++){
        flag &= (isdigit(str[i]) & 01);
    }

    return flag;
}

int main(){
    char s[20];

    while (strcmp(s, "-exit")){
        memset(s, 0, 20);
        scanf("\n%s", s);
        printf("%s - %s\n", s, is_count(s) ? "digit" : "string");
    }
}