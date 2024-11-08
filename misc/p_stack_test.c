#include <stdio.h>
#include "p_stack.h"

int main() {
    PStack stack = {0};

    int count = 0;
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            if (i % 2 == 0 && j % 2 == 0) {
                count++;
                pstack_push(&stack, (PVec2){i, j});
                printf("pushed (%d, %d)\n", i, j);
            }
        }
    }
    while(!pstack_empty(stack)) {
        PVec2 vec = pstack_pop(&stack);
        printf("POPPED (%d, %d)\n", vec.x, vec.y);
    }
    printf("Size:%d\n", stack.size);
}