/**
 * @file lex_json.h
 * @author Peter Fiala (fiala@hit.bme.hu)
 * @brief JSON lexing library
 * @version 0.1
 * @date 2020-12-20
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef LEX_JSON_H_INCLUDED
#define LEX_JSON_H_INCLDUED

#include <stdio.h>

/**
 * @brief token type identifiers
 */
typedef enum
{
	TOKEN_BRACKET_ARRAY_OPEN,	// [
	TOKEN_BRACKET_ARRAY_CLOSE,	// ]
	TOKEN_BRACKET_OBJECT_OPEN,	// {
	TOKEN_BRACKET_OBJECT_CLOSE, // }
	TOKEN_PUNCTUATOR_COLON,		// :
	TOKEN_PUNCTUATOR_COMMA,		// ,
	TOKEN_STRING,
	TOKEN_NUMBER,
	TOKEN_TRUE,
	TOKEN_FALSE,
	TOKEN_NULL
} token_type_t;

/**
 * @brief token structure
 * 
 * The token is stored as an identifier and a generic data.
 * The generic data is needed to store strings or numeric values
 */
typedef struct
{
	token_type_t type; ///<\brief the token type
	void *value;	   ///<\brief generic token data
	size_t line_cntr;  ///<\brief the line number where the token was parsed
} token_t;

/**
 * @brief linked list of tokens
 * 
 */
typedef struct tl
{
	token_t data;	 ///<\brief the token data
	struct tl *next; ///<\brief pointer to the next token or NULL at list end
} token_list_elem;
typedef token_list_elem *token_list; ///<\brief the token list pointer and list type

/**
 * @brief Read a token list from a file
 * 
 * @param fin The input file
 * @return a token list or NULL if could not read tokens
 */
token_list token_list_read_from_file(FILE *fin);

/**
 * @brief Print a token list to an output stream
 * 
 * @param tl The token list
 * @param fout The output stream
 */
void token_list_print(token_list tl, FILE *fout);

/**
 * @brief Delete a token list
 * 
 * @param tl The token list
 */
void token_list_delete(token_list tl);

/**
 * @brief Get the token object
 * 
 * @param head Pointer to the list element
 * @return Pointer to the stored token
 */
token_t const *get_token(token_list head);

#endif
