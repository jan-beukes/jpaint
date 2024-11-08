#include <stdlib.h>
#include <assert.h>
#include "p_stack.h"

void pstack_push(PStack *stack, PVec2 value) {
    Node *new = malloc(sizeof(Node));
    new->value = value;
    new->next = stack->top;
    
    stack->top = new;
    stack->size++;
    return;
}

PVec2 pstack_pop(PStack *stack) {
    assert(stack->top != NULL);

    Node *prev = stack->top;
    PVec2 ret = prev->value;
    stack->top = prev->next;
    stack->size--;
    free(prev);
    return ret;
}
