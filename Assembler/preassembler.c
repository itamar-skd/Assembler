#define _GNU_SOURCE
#include <string.h>
#include "preassembler.h"
#include "ext_vars.h"
#include "utils.h"

char* preassembler()
{
	if (!is_assembly_file(filename))
		handle_error(ERROR_FILE_NOT_ASSEMBLY, filename, 0);

	FILE* input_file = fopen(filename, "r");
	if (input_file == NULL)
		handle_error(ERROR_FILE_INVALID, filename, 0);

	char* spread_filename = change_file_extension(filename, "as", "am");

	FILE* spread_file = fopen(spread_filename, "w");
	if (!spread_file)
		handle_error(ERROR_FILE_CANT_WRITE, spread_filename, 0);

	char* line = (char*)malloc(MAX_LINE_SIZE + 1 /* null terminator */);
	if (!line)
		handle_error(ERROR_ALLOCATION_FAILED, NULL, 0);

	memset(line, '0', MAX_LINE_SIZE);
	line[MAX_LINE_SIZE] = '\0';

	size_t len = 30;
	ssize_t line_length;
	int line_number = 0;

	char* line_to_read = line;

	boolean macro_flag = FALSE;
	char* macro_name = NULL;

	while ((line_length = getline(&line_to_read, &len, input_file)) != -1)
	{
		line_number++;

		skip_spaces(&line_to_read);
		read_line_preassembler(spread_file, line, line_number, &macro_flag, &macro_name);
	}

	if (macro_flag)
		handle_error(ERROR_MACRO_UNTERMINATED, macro_name, line_number);

	fclose(input_file);
	fclose(spread_file);
	return spread_filename;
}

void read_line_preassembler(FILE* spread_file, char* line, const int line_number, boolean* macro_flag, char** macro_name)
{
	if (!spread_file || !line || do_ignore_line(&line))
		return;

	int line_length = strlen(line);
	char* temp = line;

	const int mcro_keyword_length = strlen("mcro");
	const int endmcro_keyword_length = strlen("endmcro");

	// Check for macro
	if (strncmp(temp, "mcro", mcro_keyword_length) == 0)
	{
		if (*macro_flag)
			handle_error(ERROR_MACRO_UNTERMINATED, *macro_name, line_number);

		*macro_flag = TRUE;
		temp += mcro_keyword_length;
		skip_spaces(&temp);

		Error err = get_macro_name(temp, macro_name);
		if (err != NO_ERROR)
			handle_error(ERROR_MACRO_INVALID, *macro_name, line_number);

		return;
	}

	if (strncmp(temp, "endmcro", endmcro_keyword_length) == 0)
	{
		if (!(*macro_flag))
		{
			handle_error(ERROR_MACRO_UNSTARTED, NULL, line_number);
		}

		*macro_flag = FALSE;
		free(*macro_name);
		*macro_name = NULL;
		return;
	}

	if (*macro_flag)
	{
		insert_macro_line(temp, *macro_name);
		return;
	}

	macroPtr temp_macro;
	if ((temp_macro = get_macro(temp)))
	{
		int macro_name_length = strlen(temp_macro->name);
		while (temp_macro && strncmp(temp_macro->name, temp, macro_name_length) == 0)
		{
			fwrite(temp_macro->line, sizeof(char), strlen(temp_macro->line), spread_file);
			temp_macro = temp_macro->next;
		}
	}
	else
	{
		fwrite(temp, sizeof(char), line_length, spread_file);
	}
}