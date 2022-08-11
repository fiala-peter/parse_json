/**
 * @file parse_json.h
 * @author Peter Fiala (fiala@hit.bme.hu)
 * @brief JSON parsing library
 * @version 0.1
 * @date 2020-12-20
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef PARSE_JSON_H_INCLUDED
#define PARSE_JSON_H_INCLUDED

#include "lex_json.h"
#include <stdio.h>

/** @brief Syntax tree element indentifiers */
typedef enum
{
	syntax_string,
	syntax_number,
	syntax_pair,
	syntax_elements,
	syntax_members,
	syntax_array,
	syntax_object,
	syntax_true,
	syntax_false,
	syntax_null
} syntax_type_t;

/** @brief Tree type storing the syntax tree */
typedef struct st
{
	syntax_type_t type;		///<\brief the tree node type
	void *data;				///<\brief generic data stored in the tree node
	struct st **children;	///<\brief array of pointers to children tree nodes
} syntax_tree_elem;
typedef syntax_tree_elem *syntax_tree;	///<\brief the tree pointer and tree type

/**
 * @brief Parse a json file
 * 
 * @param tl Pointer to the token list
 * @param[out] end Pointer to the first uninterpreted element of the token list
 * @return The interpreted syntax tree or NULL if could not interpret
 */
syntax_tree parse_json(token_list tl, token_list *end);

/**
 * @brief Delete a syntax tree
 * 
 * @param root The root pointer
 */
void syntax_tree_delete(syntax_tree root);

/**
 * @brief Add a child to a syntax tree node
 * The element is not copied, just linked as the last child
 * 
 * @param tree Pointer to the tree node
 * @param child Pointer to the child node
 */
void syntax_tree_add_child(syntax_tree tree, syntax_tree child);

/**
 * @brief Print a syntax tree to an ouput stream
 * 
 * @param tree Pointer to the root node
 * @param fout Output stream
 */
void syntax_tree_print(syntax_tree tree, FILE *fout);

/**
 * @brief Deep copy of a syntax tree
 * 
 * @param root Pointer to the root node
 * @return A newly allocated copy of the tree
 */
syntax_tree syntax_tree_copy(syntax_tree root);

/**
 * @brief Return first child of a syntax_tree node
 * 
 * @param root Pointer to the node
 * @return Pointer to the first child tree or NULL
 */
syntax_tree *syntax_tree_first_child(syntax_tree root);

/**
 * @brief Return next sibling of a syntax tree node
 * 
 * @param child Pointer to the syntax tree
 * @return Pointer to the next sibling tree or NULL
 */
syntax_tree *syntax_tree_next_sibling(syntax_tree *child);

/**
 * @brief Get a specific field of and object node
 * 
 * @param object Pointer to the syntax tree node
 * @param fieldname The searched field name
 * @return Pointer to the value node or NULL if the field does not exist
 */
syntax_tree syntax_tree_get_field(syntax_tree object, char const *fieldname);


#endif // PARSE_JSON_H_INCLUDED
