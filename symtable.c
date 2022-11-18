#include "symtable.h"
#include <stdio.h>
#include <stdlib.h>

tree_node_ptr tree_init()
{
    return NULL;
}

tree_node_ptr create_node(int key, int data)
{
    tree_node_ptr root = (tree_node_ptr)malloc(sizeof(tree_node_t));

    root->data = data;
    root->key = key;
    root->left = NULL;
    root->right = NULL;

    if (root == NULL)
    {
        fprintf(stderr, "create_node - data: malloc fail.\n");
        exit(99);
    }
    return root;
}

tree_node_ptr tree_insert(tree_node_ptr root, int key, int data)
{
    if (root == NULL)
    {
        return create_node(key, data);
    }
    else
    {
        if (key < root->key)
        {
            root->left = tree_insert(root->left, key, data);
        }
        else if (key > root->key)
        {
            root->right = tree_insert(root->right, key, data);
        }
        else
        {
            root->data = data;
            root->key = key;
        }
    }
    return root;
}

tree_node_ptr tree_min(tree_node_ptr root)
{

    if (root == NULL)
    {
        return NULL;
    }
    return (root->left == NULL) ? root : tree_min(root->left);
}

tree_node_ptr tree_delete(tree_node_ptr root, int key)
{
    if (root == NULL)
    {
        return NULL;
    }
    else
    {
        if (key < root->key)
        {
            root->left = tree_delete(root->left, key);
            return root;
        }
        else if (key > root->key)
        {
            root->right = tree_delete(root->right, key);
            return root;
        }
        else
        {
            if (root->right == NULL && root->left == NULL) // no children
            {
                free(root);
                return NULL;
            }
            else if (root->right != NULL && root->left != NULL) // both children
            {

                tree_node_ptr min = tree_min(root->right);
                root->data = min->data;
                root->key = min->key;
                root->right = tree_delete(root->right, min->key);
                return root;
            }
            else
            {

                tree_node_ptr child = (root->left == NULL) ? root->right : root->left;
                free(root);
                return child;
            }
        }
    }
}

void tree_dispose(tree_node_ptr root)
{
    int key = root->key;
    while (root != NULL)
    {
        root = tree_delete(root, key);
        if (root != NULL)
        {
            key = root->key;
        }
    }
}

tree_node_ptr tree_search(tree_node_ptr root, int key)
{

    if (root == NULL)
        return NULL;

    if (key < root->key)
    {
        return tree_search(root->left, key);
    }
    else if (key > root->key)
    {
        return tree_search(root->right, key);
    }
    else
    {
        return root;
    }
}

void tree_print_util(tree_node_ptr root, int space)
{
    if (root == NULL)
    {
        return;
    }

    space += SPACE_SIZE;

    tree_print_util(root->right, space);

    printf("\n");
    for (int i = SPACE_SIZE; i < space; i++)
        printf(" ");
    printf("%d,%d<", root->key, root->data);

    tree_print_util(root->left, space);
}

void tree_print(tree_node_ptr root)
{
    printf("root\n");
    printf("↓");
    tree_print_util(root, 0);
    printf("\n ------------------------------------------------------------ \n");
}