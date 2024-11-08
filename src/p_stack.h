#define pstack_empty(_PSTACK) (_PSTACK.size == 0)  

typedef struct {
    int x;
    int y;
} PVec2;

typedef struct Node{
    PVec2 value;
    struct Node *next;
} Node;

typedef struct {
    Node *top;
    int size;
} PStack;

void pstack_push(PStack *stack, PVec2 value);
PVec2 pstack_pop(PStack *stack);