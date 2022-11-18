#ifndef SYMTABLE_H
#define SYMTABLE_H

#define SPACE_SIZE 4
#include <stdbool.h>

typedef struct node_t
{
  int key;
  int data;
  struct node_t *left;
  struct node_t *right;
} tree_node_t;

typedef tree_node_t *tree_node_ptr;

/*
 * tree_node_ptr tree_init() alocates root node of binary tree
 *
 * return tree_node_ptr root (NULL)
 */
tree_node_ptr tree_init(void);

/*
 *tree_node_ptr  tree_search() searches for node with given key
 *
 *return tree_node_ptr node or NULL if key does not exist
 */
tree_node_ptr tree_search(tree_node_ptr root, int key);

//! Do NOT use! use tree_insert instead.
tree_node_ptr create_node(int key, int data);

/*
 * tree_node_ptr tree_insert instert node into *root tree
 *
 * newNode->left = NULL; newNode->right = NULL;
 * return tree_node_ptr root;
 *
 * malloc failure will result in exit(99)
 */
tree_node_ptr tree_insert(tree_node_ptr root, int key, int data);

/*
 * tree_node_ptr tree_min return *node with smallest key
 *
 * return tree_node_ptr smallest_key
 */
tree_node_ptr tree_min(tree_node_ptr root);
/*
 * tree_node_ptr tree_delete deletes node of given key
 *
 * return tree_node_ptr root
 */
tree_node_ptr tree_delete(tree_node_ptr root, int key);

/*
 * void tree_dispose disposes
 *
 * Disposes binary tree and its alocated memory
 */
void tree_dispose(tree_node_ptr root);

/*
 * void TreePrint prints out the tree in console
 *
 */
void tree_print(tree_node_ptr root);
void tree_print_util(tree_node_ptr root, int spaces);

#endif