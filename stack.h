#ifndef STACK_H
#define STACK_H

typedef struct item_t
{
  int data;
  struct item_t *next;
} StackItem;

StackItem *StackInit(void);
StackItem *StackPush(StackItem *stack, int data);
StackItem *StackPop(StackItem *stack);
int StackTop(StackItem *stack);
void StackDispose(StackItem *stack);

#endif