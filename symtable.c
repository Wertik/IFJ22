#include "symtable.h"
#include <stdio.h>
#include <stdlib.h>

TreeNode *Treeinit()
{
    return NULL;
}

TreeNode *CreateNode(int key, int data)
{
    TreeNode *root = (TreeNode *)malloc(sizeof(TreeNode));

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

TreeNode *TreeInsert(TreeNode *root, int key, int data)
{
    if (root == NULL)
    {
        return CreateNode(key, data);
    }
    else
    {
        if (key < root->key)
        {
            root->left = TreeInsert(root->left, key, data);
        }
        else if (key > root->key)
        {
            root->right = TreeInsert(root->right, key, data);
        }
        else
        {
            root->data = data;
            root->key = key;
        }
    }
    return root;
}

TreeNode *TreeMin(TreeNode *root)
{

    if (root == NULL)
    {
        return NULL;
    }
    return (root->left == NULL) ? root : TreeMin(root->left);
}

TreeNode *TreeDelete(TreeNode *root, int key)
{
    if (root == NULL)
    {
        return NULL;
    }
    else
    {
        if (key < root->key)
        {
            root->left = TreeDelete(root->left, key);
            return root;
        }
        else if (key > root->key)
        {
            root->right = TreeDelete(root->right, key);
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

                TreeNode *min = TreeMin(root->right);
                root->data = min->data;
                root->key = min->key;
                root->right = TreeDelete(root->right, min->key);
                return root;
            }
            else
            {

                TreeNode *child = (root->left == NULL) ? root->right : root->left;
                free(root);
                return child;
            }
        }
    }
}

void TreeDispose(TreeNode *root)
{
    int key = root->key;
    while (root != NULL)
    {
        root = TreeDelete(root, key);
        if (root != NULL)
        {
            key = root->key;
        }
    }
}

TreeNode *TreeSearch(TreeNode *root, int key)
{

    if (root == NULL)
        return NULL;

    if (key < root->key)
    {
        return TreeSearch(root->left, key);
    }
    else if (key > root->key)
    {
        return TreeSearch(root->right, key);
    }
    else
    {
        return root;
    }
}

void TreePrintUtil(TreeNode *root, int space)
{
    if (root == NULL)
    {
        return;
    }

    space += SPACE_SIZE;

    TreePrintUtil(root->right, space);

    printf("\n");
    for (int i = SPACE_SIZE; i < space; i++)
        printf(" ");
    printf("%d,%d<", root->key, root->data);

    TreePrintUtil(root->left, space);
}
void TreePrint(TreeNode *root)
{
    printf("root\n");
    printf("↓");
    TreePrintUtil(root, 0);
    printf("\n ------------------------------------------------------------ \n");
}