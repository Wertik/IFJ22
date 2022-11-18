#include <stdio.h>
#include <stdlib.h>
#include "symtable.h"

int main()
{

    TreeNode *root = NULL; // TreeNode *root = TreeInit(); should be use instead but for some reason function is "Undefined"
    root = TreeInsert(root, 5, 5);
    root = TreeInsert(root, 4, 4);
    root = TreeInsert(root, 6, 6);
    root = TreeInsert(root, 8, 8);
    root = TreeInsert(root, 7, 7);
    root = TreeInsert(root, 19, 19);
    root = TreeInsert(root, 3, 3);
    root = TreeInsert(root, 13, 13);
    root = TreeInsert(root, 9, 9);
    root = TreeInsert(root, 1, 1);
    root = TreeInsert(root, 2, 2);

    TreePrint(root);
    root = TreeDelete(root, 9);
    root = TreeDelete(root, 19);
    TreePrint(root);
    root = TreeDelete(root, 8);
    TreePrint(root);
    root = TreeInsert(root, 1, 4);
    TreePrint(root);

    TreeNode *k = TreeSearch(root, 2);
    TreeNode *h = TreeSearch(root, 567890);
    printf("key: %d, value: %d \n", k->key, k->data);
    if (h == NULL)
        printf("NULL \n");

    TreeDispose(root);
    return 0;
}
