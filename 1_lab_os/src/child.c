#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_INPUT 512

int main() {
    char str_[MAX_INPUT];

    while (fgets(str_, MAX_INPUT, stdin) != NULL){
        if (isupper(str_[0])){
            printf("%s", str_);
        } else {
            fprintf(stderr, "error %s", str_);
        }
    }
    return 0;
}