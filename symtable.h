#ifndef SYMTABLE_H
#define SYMTABLE_H
#include <stdbool.h>

typedef struct node_t
{
    int key;
    int data;
    struct tnode *left;
    struct tnode *right;
} TreeNode;

/*
 * TreeNode * TreeInit() alocates root node of binary tree
 *
 * return TreeNode *root (NULL)
 */
TreeNode *TreeInit();

/*
 *TreeNode * TreeSearch() searches for node with given key
 *
 *return TreeNode *node or NULL if key does not exist
 */
TreeNode *TreeSearch(TreeNode *root, int key);

/*!!!!!Do NOT use!!!! use TreeInsert instead. */
TreeNode *CreateNode(int key, int data);

/* 
 * TreeNode *TreeInsert instert node into *root tree
 *
 * newNode->left = NULL; newNode->right = NULL;
 * return TreeNode *root;
 * 
 * malloc failure will result in exit(1)
 */
TreeNode *TreeInsert(TreeNode *root, int key, int data);

/*
 * TreeNode *TreeMin return *node with smallest key
 *
 * return TreeNode *smallest_key
  */
TreeNode *TreeMin(TreeNode *root);
/*
* TreeNode *TreeDelete deletes node of given key
*
* return TreeNode *root
*/
TreeNode *TreeDelete(TreeNode *root, int key);

/*
* void TreeDispose disposes 
*
* Disposes binary tree and its alocated memory
*/
void TreeDispose(TreeNode *root);
#endif