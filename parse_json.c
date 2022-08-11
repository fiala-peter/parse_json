/**
 * @file parse_json.c
 * @author Peter Fiala (fiala@hit.bme.hu)
 * @brief implemenetation of parse_json
 * @version 0.1
 * @date 2020-12-23
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "parse_json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Create a new syntax tree node
 * 
 * @param type The node identifier 
 * @param data The generic data, NULL if none
 * @return A newly allocated syntax tree node with no children
 */
static syntax_tree syntax_tree_node_create(syntax_type_t type, void *data)
{
	syntax_tree t = malloc(sizeof(syntax_tree_elem));
	if (t == NULL)
		return NULL;
	t->type = type;
	t->data = data;
	t->children = NULL;
	return t;
}

/**
 * @brief Returns the number of child nodes of a syntax tree node
 * 
 * @param tree Pointer to the tree node
 * @return The number of children nodes
 */
static size_t syntax_tree_num_children(syntax_tree tree)
{
	size_t i = 0;
	for (syntax_tree *c = tree->children; c != NULL && *c != NULL; c++)
		i++;
	return i;
}

void syntax_tree_delete(syntax_tree root)
{
	if (root == NULL)
		return;
	for (syntax_tree *c = root->children; c != NULL && *c != NULL; c++)
		syntax_tree_delete(*c);
	free(root->children);
	free(root->data);
}

void syntax_tree_add_child(syntax_tree tree, syntax_tree child)
{
	size_t n = syntax_tree_num_children(tree);
	size_t new_n = n + 1;
	tree->children = realloc(tree->children, (new_n + 1) * sizeof(syntax_tree_elem));
	tree->children[new_n - 1] = child;
	tree->children[new_n] = NULL;
}

static void syntax_tree_print_level(syntax_tree tree, FILE *fout, int depth)
{
	if (tree == NULL)
		return;
	for (size_t i = 0; i < depth; i++)
		fputc('\t', fout);
	switch (tree->type)
	{
	case syntax_object:
		fprintf(fout, "OBJECT: ");
		break;
	case syntax_array:
		fprintf(fout, "ARRAY: ");
		break;
	case syntax_pair:
		fprintf(fout, "PAIR: ");
		break;
	case syntax_string:
		fprintf(fout, "STRING: ");
		fprintf(fout, " %s", (char const *)tree->data);
		break;
	case syntax_elements:
		fprintf(fout, "ELEMENTS");
		break;
	case syntax_members:
		fprintf(fout, "MEMBERS");
		break;
	case syntax_true:
		fprintf(fout, "TRUE");
		break;
	case syntax_false:
		fprintf(fout, "FALSE");
		break;
	case syntax_null:
		fprintf(fout, "NULL");
		break;
	case syntax_number:
		fprintf(fout, "NUMBER: ");
		fprintf(fout, " %f", *(double const *)tree->data);
		break;
	}
	fputc('\n', fout);
	for (syntax_tree *c = syntax_tree_first_child(tree); c != NULL; c = syntax_tree_next_sibling(c))
		syntax_tree_print_level(*c, fout, depth + 1);
}

void syntax_tree_print(syntax_tree tree, FILE *fout)
{
	syntax_tree_print_level(tree, fout, 0);
}

syntax_tree syntax_tree_copy(syntax_tree root)
{
	if (root == NULL)
		return NULL;
	syntax_tree t = syntax_tree_node_create(root->type, root->data);
	for (syntax_tree *c = syntax_tree_first_child(root); c != NULL; c = syntax_tree_next_sibling(c))
		syntax_tree_add_child(t, syntax_tree_copy(*c));
	return t;
}

syntax_tree syntax_tree_get_field(syntax_tree object, char const *fieldname)
{
	if (object == NULL || object->type != syntax_object)
		return NULL;
	for (syntax_tree *c = object->children; c != NULL && *c != NULL; c++)
	{
		syntax_tree field = (*c)->children[0];
		syntax_tree value = (*c)->children[1];
		if (field->data != NULL && strcmp(field->data, fieldname) == 0)
			return value;
	}
	return NULL;
}

static syntax_tree parse_array(token_list tl, token_list *end);

static syntax_tree parse_object(token_list tl, token_list *end);

/**
 * @brief Clone a string by making a dynamic copy
 * 
 * @param str The input string
 * @return Pointer to the cloned string
 */
char *strclone(char const *str)
{
	char *s = malloc(strlen(str)+1);
	return strcpy(s, str);
}

/**
 * @brief Parse a string node
 * 
 * @param tl Pointer to the token list
 * @param[out] end Pointer to the first uninterpreted element of the token list
 * @return The interpreted syntax tree or NULL if could not interpret
 */
static syntax_tree parse_string(token_list tl, token_list *end)
{
	if (tl == NULL || tl->data.type != TOKEN_STRING)
		return NULL;
	*end = tl->next;
	return syntax_tree_node_create(syntax_string, tl->data.value);
}

/**
 * @brief Clone a double number by making a dynamic copy
 * 
 * @param d The input number
 * @return pointer to the dynamically copied instance
 */
double *number_clone(double const *d)
{
	if (d == NULL)
		return NULL;
	double *ret = malloc(sizeof(double));
	*ret = *d;
	return ret;
}

/**
 * @brief Parse a number node
 * 
 * @param tl Pointer to the token list
 * @param[out] end Pointer to the first uninterpreted element of the token list
 * @return The interpreted syntax tree or NULL if could not interpret
 */
static syntax_tree parse_number(token_list tl, token_list *end)
{
	if (tl == NULL || tl->data.type != TOKEN_NUMBER)
		return NULL;
	*end = tl->next;
	return syntax_tree_node_create(syntax_number, number_clone(tl->data.value));
}

/**
 * @brief Parse a true node
 * 
 * @param tl Pointer to the token list
 * @param[out] end Pointer to the first uninterpreted element of the token list
 * @return The interpreted syntax tree or NULL if could not interpret
 */
static syntax_tree parse_true(token_list tl, token_list *end)
{
	if (tl == NULL || tl->data.type != TOKEN_TRUE)
		return NULL;
	*end = tl->next;
	return syntax_tree_node_create(syntax_true, NULL);
}

/**
 * @brief Parse a false node
 * 
 * @param tl Pointer to the token list
 * @param[out] end Pointer to the first uninterpreted element of the token list
 * @return The interpreted syntax tree or NULL if could not interpret
 */
static syntax_tree parse_false(token_list tl, token_list *end)
{
	if (tl == NULL || tl->data.type != TOKEN_FALSE)
		return NULL;
	*end = tl->next;
	return syntax_tree_node_create(syntax_false, NULL);
}

/**
 * @brief Parse a null node
 * 
 * @param tl Pointer to the token list
 * @param[out] end Pointer to the first uninterpreted element of the token list
 * @return The interpreted syntax tree or NULL if could not interpret
 */
static syntax_tree parse_null(token_list tl, token_list *end)
{
	if (tl == NULL || tl->data.type != TOKEN_NULL)
		return NULL;
	*end = tl->next;
	return syntax_tree_node_create(syntax_null, NULL);
}

/**
 * @brief Parse a value node
 * 
 * @param tl Pointer to the token list
 * @param[out] end Pointer to the first uninterpreted element of the token list
 * @return The interpreted syntax tree or NULL if could not interpret
 */
static syntax_tree parse_value(token_list tl, token_list *end)
{
	typedef syntax_tree (*parsing_function_t)(token_list, token_list *);
	parsing_function_t pfs[] = {parse_object, parse_array, parse_true, parse_false, parse_null, parse_string, parse_number, NULL};
	syntax_tree st;
	for (int i = 0; pfs[i] != NULL; i++)
		if ((st = pfs[i](tl, end)) != NULL)
			return st;
	return NULL;
}

/**
 * @brief Parse a pair node
 * 
 * @param tl Pointer to the token list
 * @param[out] end Pointer to the first uninterpreted element of the token list
 * @return The interpreted syntax tree or NULL if could not interpret
 */
static syntax_tree parse_pair(token_list tl, token_list *end)
{
	if (tl == NULL)
		return NULL;
	*end = tl;

	if (tl->data.type != TOKEN_STRING)
		return NULL;
	char const *fieldname = tl->data.value;
	tl = tl->next;
	if (tl == NULL || tl->data.type != TOKEN_PUNCTUATOR_COLON)
		return NULL;
	tl = tl->next;
	token_list e;
	syntax_tree value = parse_value(tl, &e);
	if (value == NULL)
		return NULL;
	*end = e;

	syntax_tree pair = syntax_tree_node_create(syntax_pair, NULL);
	syntax_tree_add_child(pair, syntax_tree_node_create(syntax_string, strclone(fieldname)));
	syntax_tree_add_child(pair, value);
	return pair;
}

/**
 * @brief Parse an elements node
 * 
 * @param tl Pointer to the token list
 * @param[out] end Pointer to the first uninterpreted element of the token list
 * @return The interpreted syntax tree or NULL if could not interpret
 */
static syntax_tree parse_elements(token_list tl, token_list *end)
{
	if (tl == NULL)
		return NULL;
	*end = tl;

	token_list t = tl;
	token_list e;
	syntax_tree v = parse_value(t, &e);
	if (v == NULL)
		return NULL;
	t = e;

	syntax_tree elements = syntax_tree_node_create(syntax_elements, NULL);
	syntax_tree_add_child(elements, v);

	while (t != NULL && get_token(t)->type == TOKEN_PUNCTUATOR_COMMA)
	{
		t = t->next;
		syntax_tree v = parse_value(t, &e);
		if (v == NULL)
		{
			syntax_tree_delete(elements);
			return NULL;
		}
		syntax_tree_add_child(elements, v);
		t = e;
	}

	*end = e;
	return elements;
}

/**
 * @brief Parse a members node
 * 
 * @param tl Pointer to the token list
 * @param[out] end Pointer to the first uninterpreted element of the token list
 * @return The interpreted syntax tree or NULL if could not interpret
 */
static syntax_tree parse_members(token_list tl, token_list *end)
{
	if (tl == NULL)
		return NULL;
	*end = tl;

	token_list t = tl;
	token_list e;
	syntax_tree p = parse_pair(t, &e);
	if (p == NULL)
		return NULL;
	t = e;

	syntax_tree members = syntax_tree_node_create(syntax_members, NULL);
	syntax_tree_add_child(members, p);

	while (t != NULL && get_token(t)->type == TOKEN_PUNCTUATOR_COMMA)
	{
		t = t->next;
		p = parse_pair(t, &e);
		if (p == NULL)
		{
			syntax_tree_delete(members);
			return NULL;
		}
		syntax_tree_add_child(members, p);
		t = e;
	}

	*end = e;
	return members;
}

/**
 * @brief Parse an array node
 * 
 * @param tl Pointer to the token list
 * @param[out] end Pointer to the first uninterpreted element of the token list
 * @return The interpreted syntax tree or NULL if could not interpret
 */
static syntax_tree parse_array(token_list tl, token_list *end)
{
	if (tl == NULL)
		return NULL;
	*end = tl;

	token_list t = tl;
	if (get_token(t) == NULL || get_token(t)->type != TOKEN_BRACKET_ARRAY_OPEN)
		return NULL;
	t = t->next;

	token_list e;
	syntax_tree elements = parse_elements(t, &e);
	t = e;

	if (get_token(t) == NULL || get_token(t)->type != TOKEN_BRACKET_ARRAY_CLOSE)
	{
		syntax_tree_delete(elements);
		return NULL;
	}
	t = t->next;

	syntax_tree st = syntax_tree_node_create(syntax_array, NULL);
	if (elements != NULL)
	{
		for (syntax_tree *c = elements->children; c != NULL && *c != NULL; c++)
		{
			syntax_tree_add_child(st, *c);
			*c = NULL;
		}
		syntax_tree_delete(elements);
	}

	*end = t;
	return st;
}

/**
 * @brief Parse an object node
 * 
 * @param tl Pointer to the token list
 * @param[out] end Pointer to the first uninterpreted element of the token list
 * @return The interpreted syntax tree or NULL if could not interpret
 */
static syntax_tree parse_object(token_list tl, token_list *end)
{
	if (tl == NULL)
		return NULL;
	*end = tl;

	token_list t = tl;
	if (get_token(t) == NULL || get_token(t)->type != TOKEN_BRACKET_OBJECT_OPEN)
		return NULL;
	t = t->next;

	token_list e;
	syntax_tree members = parse_members(t, &e);
	t = e;

	if (get_token(t) == NULL || get_token(t)->type != TOKEN_BRACKET_OBJECT_CLOSE)
	{
		syntax_tree_delete(members);
		return NULL;
	}
	t = t->next;

	syntax_tree st = syntax_tree_node_create(syntax_object, NULL);
	if (members != NULL)
	{
		for (syntax_tree *c = members->children; c != NULL && *c != NULL; c++)
		{
			syntax_tree_add_child(st, *c);
			*c = NULL;
		}
		syntax_tree_delete(members);
	}

	*end = t;
	return st;
}

syntax_tree parse_json(token_list tl, token_list *end)
{
	*end = tl;
	if (tl == NULL)
		return NULL;
	// try to parse an array
	syntax_tree s = parse_array(tl, end);
	if (s != NULL)
		return s;
	// try to parse an object
	s = parse_object(tl, end);
	if (s != NULL)
		return s;
	return NULL;
}

syntax_tree *syntax_tree_first_child(syntax_tree root)
{
	if (root == NULL)
		return NULL;
	if (root->children == NULL)
		return NULL;
	return root->children;
}

syntax_tree *syntax_tree_next_sibling(syntax_tree *child)
{
	syntax_tree *next = child + 1;
	if (*next == NULL)
		return NULL;
	return next;
}
