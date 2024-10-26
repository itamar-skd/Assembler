#define _GNU_SOURCE
#include "second_pass.h"
#include "structs.h"
#include "ext_vars.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>

void second_pass(const char* filename)
{
	FILE* input_file = fopen(filename, "r");
	if (input_file == NULL)
		handle_error(ERROR_FILE_INVALID, filename, 0);

	int ic = 0;

	char* line = (char*)malloc(MAX_LINE_SIZE + 1 /* null terminator */);
	memset(line, '0', MAX_LINE_SIZE);
	line[MAX_LINE_SIZE] = '\0';

	size_t len = 30;
	ssize_t line_length;
	int line_number = 0;

	char* line_to_read = line;
	while ((line_length = getline(&line_to_read, &len, input_file)) != -1)
	{
		line_number++;

		if (do_ignore_line(&line_to_read)) // This also removes whitespaces from the start
			continue;

		read_line_second_pass(line_to_read, line_number, &ic);
	}

	fclose(input_file);
}

void read_line_second_pass(char* line, const int line_number, int* ic)
{
	if (!line || !ic)
		return;

	char* temp = line;
	int line_length = strlen(line);

	// Check for label
	int label_length = 0;
	if ((label_length = has_label(temp, line_length)) != -1)
	{
		// If there's a label, skip it.
		temp += label_length + 1;
		line_length -= label_length + 1;

		skip_spaces(&temp);
	}

	// Check for an entry directive.
	// All other directives were handled in the first-pass.
	if (*temp == '.')
	{
		temp++;
		line_length--;

		if (strncmp(temp, "entry", strlen("entry")) == 0)
		{
			if (!read_directive(&temp, &line_length, "entry"))
			{
				handle_error(ERROR_DIRECTIVE_UNSEPARATED, NULL, line_number);
			}

			read_entry(temp, line_number);
		}

		return;
	}

	char command_name[MAX_COMMAND_LENGTH + 1 /* null terminator */];
	memset(command_name, '\0', MAX_COMMAND_LENGTH);

	int i;
	for (i = 0; *temp != ' ' && *temp != '\t' && *temp != '\n' && *temp != '\0' && i < MAX_COMMAND_LENGTH; temp++, line_length--, i++)
		command_name[i] = *temp;

	command_name[i] = '\0';

	OpCode opcode;
	if ((opcode = is_command(command_name)) == UNKNOWN_COMMAND)
		handle_error(ERROR_COMMAND_UNKNOWN, command_name, line_length);

	*ic += read_command_second_pass(opcode, temp, line_number, *ic);
}

void write_to_commands(char* word, int ic)
{
	if (!word || ic >= strlen(command_instructions))
		return;

	int target_index = ic * 12;

	memcpy(&(command_instructions[target_index]), word, strlen(word));
}

void read_entry(char* entries, const int line_number)
{
	char* entries_iterator = entries;

	char label[MAX_LABEL_SIZE + 1 /* null terminator */];
	memset(label, '0', MAX_LABEL_SIZE);
	label[MAX_LABEL_SIZE] = '\0';

	int label_counter = 0;
	boolean keep_alive = TRUE;

	while (keep_alive)
	{
		if (*entries_iterator == ' ' || *entries_iterator == '\t')
		{
			entries_iterator++;
			continue;
		}

		if (*entries_iterator == ',' || *entries_iterator == '\n' || *entries_iterator == '\0')
		{
			label[label_counter] = '\0';
			if (is_valid_label(label))
			{
				labelPtr label_temp;
				if (!(label_temp = set_label_as_entry(label)))
				{
					handle_error(ERROR_LABEL_UNDEFINED, label, line_number);
				}

				label_counter = 0;
				write_entries_to_file(label, label_temp->address);
			}
			else
				handle_error(ERROR_LABEL_INVALID, label, line_number);

			if (*entries_iterator == '\0' || *entries_iterator == '\n')
				keep_alive = FALSE;
		}
		else
		{
			label[label_counter] = *entries_iterator;
			label_counter++;
		}

		entries_iterator++;
	}
}

int read_command_second_pass(const OpCode opcode, char* operands, const int line_number, int ic)
{
	// Immediate values and registers as operands were already handled in the first pass.
	// In the second pass, we're only looking for labels as operands.

	char* temp = operands;

	char* source_operand = NULL;
	OperandMethod source_operand_method;
	char* target_operand = NULL;
	OperandMethod target_operand_method;

	int num_operands = get_num_operands(opcode);
	if (num_operands > 0)
	{
		target_operand = (char*)malloc(MAX_LINE_SIZE + 1 /* null terminator */);
		if (!target_operand)
			handle_error(ERROR_ALLOCATION_FAILED, NULL, line_number);

		memset(target_operand, '0', MAX_LINE_SIZE);
		target_operand[MAX_LINE_SIZE] = '\0';

		if (num_operands == 2)
		{
			source_operand = (char*)malloc(MAX_LINE_SIZE + 1 /* null terminator */);
			if (!source_operand)
				handle_error(ERROR_ALLOCATION_FAILED, NULL, line_number);

			memset(source_operand, '0', MAX_LINE_SIZE);
			source_operand[MAX_LINE_SIZE] = '\0';
		}
	}

	// Look for the source operand
	if (source_operand)
	{
		int counter = 0;
		while (*temp != ',' && *temp != '\n' && *temp != '\0')
		{
			if (*temp != ' ' && *temp != '\t')
			{
				source_operand[counter] = *temp;
				counter++;
			}

			temp++;
		}

		source_operand[counter] = '\0';

		if (is_number(source_operand) != INT_MIN)
		{
			source_operand_method = IMMEDIATE_METHOD;
		}
		if (is_valid_label(source_operand))
		{
			source_operand_method = DIRECT_METHOD;

			char source_word[12 + 1 /* null terminator */];
			memset(source_word, '0', 12);
			source_word[12] = '\0';

			boolean is_extern;
			Error err;

			if ((err = is_label_extern(source_operand, &is_extern)) == NO_ERROR)
			{
				if (!is_extern)
				{
					memcpy(&(source_word[10]), "10", strlen("10"));

					int source_address = get_label_address(source_operand);
					char address_binary[10 + 1 /* null terminator */];
					memset(address_binary, '0', 10);
					address_binary[10] = '\0';

					char* address_binary_ptr = address_binary_ptr;
					int_to_binary(&address_binary_ptr, source_address, 10, FALSE);
					memcpy(source_word, address_binary, 10);
				}
				else
				{
					memcpy(&(source_word[10]), "01", strlen("01")); // Resembles external variable
					write_external_to_file(source_operand, ic + 1);
				}
			}
			else
			{
				handle_error(ERROR_LABEL_UNDEFINED, source_operand, line_number);
			}

			write_to_commands(source_word, ic + 1);
		}
		else if (is_register(source_operand))
		{
			source_operand_method = REGISTER_DIRECT_METHOD;
		}
		else
		{
			handle_error(ERROR_OPERAND_INVALID, source_operand, line_number);
		}

		// Skip over the comma
		temp++;

		free(source_operand);
	}

	// Look for the target operand
	if (target_operand)
	{
		int counter = 0;
		while (*temp != ',' && *temp != '\n' && *temp != '\0')
		{
			if (*temp != ' ' && *temp != '\t')
			{
				target_operand[counter] = *temp;
				counter++;
			}

			temp++;
		}

		target_operand[counter] = '\0';

		if (is_number(target_operand) != INT_MIN)
		{
			target_operand_method = IMMEDIATE_METHOD;
		}
		else if (is_valid_label(target_operand))
		{
			target_operand_method = DIRECT_METHOD;

			char target_word[12 + 1 /* null terminator */];
			memset(target_word, '0', 12);
			target_word[12] = '\0';

			boolean is_extern;
			Error err;

			if ((err = is_label_extern(target_operand, &is_extern)) == NO_ERROR)
			{
				if (!is_extern)
				{
					memcpy(&(target_word[10]), "10", strlen("10"));

					int target_address = get_label_address(target_operand);
					char address_binary[10 + 1 /* null terminator */];
					memset(address_binary, '0', 10);
					address_binary[10] = '\0';

					char* address_binary_ptr = address_binary;
					int_to_binary(&address_binary_ptr, target_address, 10, FALSE);
					memcpy(target_word, address_binary, 10);
				}
				else
				{
					memcpy(&(target_word[10]), "01", strlen("01")); // Resembles external variable
					write_external_to_file(target_operand, ic + (source_operand ? 2 : 1));
				}
			}
			else
			{
				handle_error(ERROR_LABEL_UNDEFINED, target_operand, line_number);
			}

			write_to_commands(target_word, ic + (num_operands == 1 ? 1 : 2));
		}
		else if (is_register(target_operand))
		{
			target_operand_method = REGISTER_DIRECT_METHOD;
		}
		else
		{
			handle_error(ERROR_OPERAND_INVALID, target_operand, line_number);
		}

		free(target_operand);
	}

	int num_statements = 1;

	if (num_operands > 0)
	{
		num_statements++;

		if (num_operands == 2)
		{
			if (source_operand_method != REGISTER_DIRECT_METHOD || target_operand_method != REGISTER_DIRECT_METHOD)
			{
				num_statements++;
			}
		}
	}

	return num_statements;
}