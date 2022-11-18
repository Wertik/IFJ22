#ifndef PARSER_H
#define PARSER_H

#include "array.h"
#include "symtable.h"

tree_node_ptr parse(array_ptr tokens);

#endif