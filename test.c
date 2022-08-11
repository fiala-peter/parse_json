#include "parse_json.h"

#include <stdlib.h>

enum
{
	MAX_FILE = 1 << 25
};


int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stdout, "Usage: %s input_json\n", argv[0]);
		return 0;
	}

	char *fname = argv[1];
	FILE *fin = fopen(fname, "r");
	if (fin == NULL)
	{
		fprintf(stderr, "Could not open file %s\n", fname);
		return 1;
	}
	token_list tl = token_list_read_from_file(fin);
	fclose(fin);

	token_list_print(tl, stdout);

	token_list end;
	syntax_tree st = parse_json(tl, &end);
	if (end != NULL)
		token_list_print(end, stdout);
		
	syntax_tree_print(st, stdout);

	syntax_tree_delete(st);
	token_list_delete(tl);

	return 0;
}
