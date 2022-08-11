/**
 * @file lex_json.c
 * @author Peter FIala (fiala@hit.bme.hu)
 * @brief implementation of lex_json
 * @version 0.1
 * @date 2020-12-23
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "lex_json.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @brief print a token to an output stream
 * 
 * @param token the token
 * @param fout the output stream
 */
static void token_print(token_t token, FILE *fout)
{
	fprintf(fout, "line: %lu: ", token.line_cntr);
	switch (token.type)
	{
	case TOKEN_BRACKET_ARRAY_OPEN:
		fprintf(fout, "[");
		break;
	case TOKEN_BRACKET_ARRAY_CLOSE:
		fprintf(fout, "]");
		break;
	case TOKEN_BRACKET_OBJECT_OPEN:
		fprintf(fout, "{");
		break;
	case TOKEN_BRACKET_OBJECT_CLOSE:
		fprintf(fout, "}");
		break;
	case TOKEN_PUNCTUATOR_COLON:
		fprintf(fout, ":");
		break;
	case TOKEN_PUNCTUATOR_COMMA:
		fprintf(fout, ",");
		break;
	case TOKEN_STRING:
		fprintf(fout, "%s", (char const *)token.value);
		break;
	case TOKEN_NUMBER:
		fprintf(fout, "%f", *(double *)token.value);
		break;
	case TOKEN_TRUE:
		fprintf(fout, "TRUE");
		break;
	case TOKEN_FALSE:
		fprintf(fout, "FALSE");
		break;
	case TOKEN_NULL:
		fprintf(fout, "NULL");
		break;
	}
}

void token_list_print(token_list tl, FILE *fout)
{
	while (tl != NULL)
	{
		token_print(tl->data, fout);
		fprintf(fout, "\n");
		tl = tl->next;
	}
}

/**
 * @brief read a number from a string
 * 
 * @param str the string to read from
 * @param number[out] the number is stored here
 * @return char const* pointer to the first uninpterpreted character in the string
 */
static char const *read_number(char const *str, double *number)
{
	char const *s = str;

	double num = 0.0;
	int sign = 0;

	// read integer part
	if (*s == '-')
	{
		sign = 1;
		s++;
	}
	if (*s >= '1' && *s <= '9') // first digit cannot be zero
	{
		num = *s - '0';
		s++;
	}
	while (*s >= '0' && *s <= '9') // remaining digits
	{
		num *= 10;
		num += *s - '0';
		s++;
	}
	// fractional part
	if (*s == '.')
	{
		s++;
		double d = 1.0;
		while (*s >= '0' && *s <= '9')
		{
			d /= 10;
			num += (*s - '0') * d;
			s++;
		}
	}
	if (sign == 1)
		num = -num;
	// exponent part
	if (*s == 'e' || *s == 'E')
	{
		int exponent = 0, expsign = 0;
		s++;

		if (*s == '-')
		{
			expsign = 1;
			s++;
		}
		else if (*s == '+')
		{
			expsign = 0;
			s++;
		}
		while (*s >= '0' && *s <= '9')
		{
			exponent *= 10;
			exponent += *s - '0';
			s++;
		}
		if (expsign == 1)
			exponent = -exponent;
		num = num * pow(10.0, exponent);
	}

	*number = num;
	return s;
}

/**
 * @brief Read next token from a string
 * 
 * if a token could be interpreted it is returned as side effect and a pointer
 * is returned to the first uninterpreted character.
 * If the buffer end has been reached NULL is returned.
 * For the case of parsing error the original character pointer is returned.
 * 
 * @param str The input string
 * @param tok[out] The interpreted token
 * @return Pointer to the first uninterpreted character or NULL if the character buffer is empty
 */
static char const *read_next_token(char const *str, token_t *tok)
{
	// save the input to report errors
	char const *save = str;

	// skip white spaces
	while (*str != '\0' && isspace(*str))
		str++;

	// end of input has been reached, no more tokens
	if (*str == '\0')
		return NULL;

	// try to interpret single-character tokens
	struct
	{
		token_type_t tok;
		char c;
	} single_character_tokens[] = {
		{TOKEN_PUNCTUATOR_COMMA, ','},
		{TOKEN_PUNCTUATOR_COLON, ':'},
		{TOKEN_BRACKET_ARRAY_OPEN, '['},
		{TOKEN_BRACKET_ARRAY_CLOSE, ']'},
		{TOKEN_BRACKET_OBJECT_OPEN, '{'},
		{TOKEN_BRACKET_OBJECT_CLOSE, '}'},
		{0, '\0'}};
	for (int i = 0; single_character_tokens[i].c != '\0'; i++)
	{
		if (*str == single_character_tokens[i].c)
		{
			tok->type = single_character_tokens[i].tok;
			tok->value = NULL;
			return str + 1;
		}
	}

	// try to interpret keyword tokens
	struct
	{
		token_type_t tok;
		char *str;
	} keywords[] = {
		{TOKEN_TRUE, "true"},
		{TOKEN_FALSE, "false"},
		{TOKEN_NULL, "null"},
		{0, NULL}};
	for (int i = 0; keywords[i].str != NULL; i++)
	{
		size_t k = strlen(keywords[i].str);
		if (strncmp(str, keywords[i].str, k) == 0 && !isalnum(str[k]) && str[k] != '_')
		{
			tok->type = keywords[i].tok;
			tok->value = NULL;
			return str + k;
		}
	}

	// try to interpret strings
	if (*str == '\"')
	{
		char const *end = str + 1;
		while (*end != '\0' && *end != '\"')
		{
			// skip quote escape sequences
			if (*end == '\\' && *(end + 1) == '\"')
				end += 2;
			else
				end++;
		}
		if (*end == '\0')
			fprintf(stderr, "Error parsing string");
		else
			end++;
		size_t n = end - str;
		tok->value = malloc(n - 2 + 1);
		char *v = (char *)tok->value;
		strncpy(v, str + 1, n - 2);
		v[n - 2] = '\0';
		tok->type = TOKEN_STRING;
		return end;
	}

	// try to interpret number
	double number;
	char const *ret = read_number(str, &number);
	if (ret != str)
	{
		tok->type = TOKEN_NUMBER;
		tok->value = malloc(sizeof(double));
		*(double *)(tok->value) = number;
		return ret;
	}

	return save;
}

token_list token_list_read_from_file(FILE *fin)
{
	enum
	{
		MAX_LINE = 16*1024
	};
	char linebuffer[MAX_LINE];
	size_t line_cntr = 0;
	token_list_elem sentinel = {{0}, NULL};
	token_list last = &sentinel;
	while (fgets(linebuffer, MAX_LINE, fin) != NULL)
	{
		line_cntr++;

		token_t tok;
		char const *buf = linebuffer;
		char const *end;
		while ((end = read_next_token(buf, &tok)) != NULL)
		{
			if (end == buf)
			{
				token_list_delete(sentinel.next);
				return NULL;
			}
			buf = end;
			tok.line_cntr = line_cntr;
			last->next = malloc(sizeof(token_list_elem));
			last->next->data = tok;
			last->next->next = NULL;
			last = last->next;
		}
	}
	return sentinel.next;
}

void token_list_delete(token_list tl)
{
	while (tl != NULL)
	{
		token_list p = tl;
		tl = tl->next;
		free(p);
	}
}

token_t const *get_token(token_list head)
{
	if (head == NULL)
		return NULL;
	return &head->data;
}
