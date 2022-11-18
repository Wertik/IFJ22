#include <stdio.h>
#include <stdlib.h>
#include "symtable.h"

int main()
{
    tree_node_t *root = tree_init();
    root = tree_insert(root, 5, 5);
    root = tree_insert(root, 4, 4);
    root = tree_insert(root, 6, 6);
    root = tree_insert(root, 8, 8);
    root = tree_insert(root, 7, 7);
    root = tree_insert(root, 19, 19);
    root = tree_insert(root, 3, 3);
    root = tree_insert(root, 13, 13);
    root = tree_insert(root, 9, 9);
    root = tree_insert(root, 1, 1);
    root = tree_insert(root, 2, 2);

    tree_print(root);
    root = tree_delete(root, 9);
    root = tree_delete(root, 19);
    tree_print(root);
    root = tree_delete(root, 8);
    tree_print(root);
    root = tree_insert(root, 1, 4);
    tree_print(root);

    tree_node_t *k = tree_search(root, 2);
    tree_node_t *h = tree_search(root, 567890);
    printf("key: %d, value: %d \n", k->key, k->data);
    if (h == NULL)
        printf("NULL \n");
    tree_dispose(root);
    return 0;
}
