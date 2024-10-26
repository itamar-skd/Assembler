#define _GNU_SOURCE
#include <string.h>
#include <limits.h>
#include <stddef.h>

#include "first_pass.h"
#include "structs.h"
#include "ext_vars.h"
#include "utils.h"

void first_pass(const char* filename)
{
	FILE* input_file = fopen(filename, "r");
	if (input_file == NULL)
		handle_error(ERROR_FILE_INVALID, filename, 0);

	int ic = 0, dc = 0;

	char* line = (char*)malloc(MAX_LINE_SIZE + 1 /* null terminator */);
	if (!line)
		handle_error(ERROR_ALLOCATION_FAILED, NULL, 0);

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

		read_line_first_pass(line_to_read, &ic, &dc, line_number);
	}

	labelPtr tempLabel = firstLabel;
	while (tempLabel != NULL)
	{
		if (tempLabel->is_data == TRUE)
			tempLabel->address += ic;

		tempLabel = tempLabel->next;
	}

	fclose(input_file);
} /* first_pass */

void read_line_first_pass(const char* line, int* ic, int* dc, const int line_number)
{
	if (!line || !ic || !dc) return;

	char* temp = line;
	int line_length = strlen(line);

	char label[MAX_LABEL_SIZE + 1 /* null terminator */];
	memset(label, '0', MAX_LABEL_SIZE);
	label[MAX_LABEL_SIZE] = '\0';

	int label_length = 0;
	boolean label_flag = FALSE;

	// Instruction format:
	// 1.	No operands:
	//	[OPTIONAL LABEL]: [.(DIRECTIVE) | (OPCODE)]
	// 
	// 2.	1 operand:
	// [OPTIONAL LABEL]: [.(DIRECTIVE) | (OPCODE)] [(SOURCE OPERAND) | (TARGET OPERAND)]
	//
	// 3.	2 operands:
	// [OPTIONAL LABEL]: [.(DIRECTIVE) | (OPCODE)] [SOURCE OPERAND],[TARGET OPERAND]

	// Check for a label
	if ((label_length = has_label(temp, line_length)) != -1)
	{
		strncpy(label, temp, label_length);
		label[label_length] = '\0';

		// Advance to the next field
		temp += label_length + 1;
		line_length -= label_length + 1;

		skip_spaces(&temp);
		label_flag = TRUE;
	}

	// Check if the next field is a directive.
	// Entries are handled at the second-pass.
	if (*temp == '.')
	{
		temp++;
		line_length--;

		// read_directive checks the directive's validity and skips the line to the presumable placement of the operands
		// reminder of a directive structure:
		// (OPTIONAL LABEL): .(directive) (operands)

		// read_(directive name) handles the directive's instruction list

		Error directive_unseparated_err;

		if (strncmp(temp, "entry", strlen("entry")) == 0)
		{
			return;
		}
		else if (strncmp(temp, "extern", strlen("extern")) == 0)
		{
			if (!read_directive(&temp, &line_length, "extern"))
			{
				handle_error(ERROR_DIRECTIVE_UNSEPARATED, NULL, line_number);
			}

			read_extern(temp, line_number);
		}
		else if (strncmp(temp, "data", strlen("data")) == 0)
		{
			Error label_error;
			if ((label_error = add_label(label, *dc, FALSE, TRUE, FALSE)) != NO_ERROR)
			{
				handle_error(label_error, label, line_number);
			}

			if (!read_directive(&temp, &line_length, "data"))
			{
				handle_error(ERROR_DIRECTIVE_UNSEPARATED, NULL, line_number);
			}

			*dc += read_data(temp, line_number);
		}
		else if (strncmp(temp, "string", strlen("string")) == 0)
		{
			Error label_error;
			if ((label_error = add_label(label, *dc, FALSE, TRUE, FALSE)) != NO_ERROR)
			{
				handle_error(label_error, label, line_number);
			}

			if (!read_directive(&temp, &line_length, "string"))
			{
				handle_error(ERROR_DIRECTIVE_UNSEPARATED, NULL, line_number);
			}

			*dc += read_string(temp, line_number);
		}
		else
		{
			// Unknown Directive.
			char directive[MAX_LINE_SIZE + 1 /* null terminator */];
			memset(directive, '0', MAX_LINE_SIZE);
			directive[MAX_LINE_SIZE] = '\0';

			int counter = 0;
			while (*temp != ' ' && *temp != '\t' && *temp != '\n' && *temp != '\0')
			{
				directive[counter] = *temp;
				counter++;
				temp++;
			}

			directive[counter] = '\0';

			handle_error(ERROR_DIRECTIVE_UNKNOWN, directive, line_number);
		}

		return;
	}
	// Otherwise, it's a command.

	// From this point on, we assume that the line references a command and not a directive.

	if (label_flag)
	{
		Error label_error;
		if ((label_error = add_label(label, *ic, TRUE, FALSE, FALSE)) != NO_ERROR)
		{
			handle_error(label_error, label, line_number);
		}
	}

	char command_name[MAX_COMMAND_LENGTH + 1 /* null terminator */];

	memset(command_name, '0', MAX_COMMAND_LENGTH);
	command_name[MAX_COMMAND_LENGTH] = '\0';

	int i;
	for (i = 0; *temp != ' ' && *temp != '\t' && *temp != '\n' && *temp != '\0' && i < MAX_COMMAND_LENGTH; temp++, line_length--, i++)
		command_name[i] = *temp;

	command_name[i] = '\0';
	OpCode opcode;
	if ((opcode = is_command(command_name)) == UNKNOWN_COMMAND)
		handle_error(ERROR_COMMAND_UNKNOWN, command_name, line_number);

	skip_spaces(&temp);

	*ic += read_command(opcode, temp, line_number);
} /* read_line_first_pass */

boolean insert_new_command(const char* data, const int num_instructions)
{
	if (!data || strlen(data) > 12 * num_instructions)
		return;

	const int data_length = strlen(data);
	const int new_instructions_length = strlen(command_instructions) + data_length + 1 /* null terminator */;
	char* new_ptr = realloc(command_instructions, new_instructions_length);
	if (!new_ptr) {
		return FALSE;
	}

	command_instructions = new_ptr;

	strcat(command_instructions, data);
} /* allocate_new_instruction */

void insert_new_data(const char* data)
{
	if (strlen(data) != 12)
		return;

	int current_data_size = strlen(data_instructions);
	data_instructions = (char*)realloc(data_instructions, sizeof(char) * (current_data_size + 12 /* word length */ + 1 /* null terminator */));

	strcat(data_instructions, data);
}

int read_command(const OpCode command_type, const char* data, const int line_number)
{
	int num_statements = 1;

	char* temp = data;
	int length = strlen(data);

	//char source_operand[LABEL_SIZE + 1 /* null terminator */];
	char* target_operand = NULL;
	OperandMethod target_operand_method;
	char* source_operand = NULL;
	OperandMethod source_operand_method;

	int num_operands = get_num_operands(command_type);

	if (num_operands >= 1)
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

	int command_length = 12 * (num_operands + 1);
	char* command = (char*)malloc(command_length + 1 /* null terminator */);
	if (!command)
		handle_error(ERROR_ALLOCATION_FAILED, NULL, line_number);

	memset(command, '0', command_length);
	command[command_length] = '\0';

	// A, R, E - The first word will always be absolute.
	strncpy(&command[10], "00", strlen("00"));

	// OPCODE - A command's index in the command table.
	char opcode[4 + 1 /* null terminator */];
	memset(opcode, '0', 4);
	opcode[4] = '\0';

	char* opcode_ptr = opcode;
	int_to_binary(&opcode_ptr, command_type, 4, FALSE);
	strncpy(&command[3], opcode, strlen(opcode));

	int num_immediate = 0;
	int num_register = 0;

	if (source_operand)
	{
		int i = 0;
		while (*temp != ',' && *temp != '\n' && *temp != '\0')
		{
			if (*temp != ' ' && *temp != '\t')
			{
				source_operand[i] = *temp;
				i++;
			}

			temp++;
			length--;
		}

		source_operand[i] = '\0';

		if ((num_immediate = is_number(source_operand)) != INT_MIN)
		{
			source_operand_method = IMMEDIATE_METHOD;
			strncpy(&(command[0]), "001", strlen("001"));

			char immediate_binary[10 + 1 /* null terminator */];
			memset(&immediate_binary, '0', 10);
			immediate_binary[10] = '\0';

			char* immediate_binary_ptr = immediate_binary;
			if (!int_to_binary(&immediate_binary_ptr, num_immediate, 10, FALSE))
			{
				handle_error(ERROR_DATA_INVALID_INTEGER, source_operand, line_number);
			}
			strncpy(&(command[12]), immediate_binary_ptr, 10);
		}
		else if (is_valid_label(source_operand))
		{
			source_operand_method = DIRECT_METHOD;
			strncpy(&(command[0]), "011", strlen("001"));
		}
		else if ((num_register = is_register(source_operand)) != -1)
		{
			source_operand_method = REGISTER_DIRECT_METHOD;
			strncpy(&(command[0]), "101", strlen("001"));

			char register_binary[5 + 1 /* null terminator */];
			memset(&register_binary, '0', 5);
			register_binary[5] = '\0';

			char* register_binary_ptr = register_binary;
			int_to_binary(&register_binary_ptr, num_register, 5, FALSE);
			strncpy(&(command[12]), register_binary_ptr, 5);
		}
		else {
			handle_error(ERROR_OPERAND_INVALID, source_operand, line_number);
		}

		// Skip over the comma
		temp++;
		length--;

		free(source_operand);
	}

	if (target_operand)
	{
		int i = 0;
		while (*temp != ',' && *temp != '\n' && *temp != '\0')
		{
			if (*temp != ' ' && *temp != '\t')
			{
				target_operand[i] = *temp;
				i++;
			}

			length--;
			temp++;
		}

		target_operand[i] = '\0';

		if (strncmp(target_operand, "-5", strlen("-5")) == 0)
		{
			int a = 5;
		}

		if ((num_immediate = is_number(target_operand)) != INT_MIN)
		{
			target_operand_method = IMMEDIATE_METHOD;
			strncpy(&(command[7]), "001", strlen("001"));

			char immediate_binary[10 + 1 /* null terminator */];
			memset(&immediate_binary, '0', 10);
			immediate_binary[10] = '\0';

			int target_index = num_operands == 1 ? 12 : 24;
			char* immediate_binary_ptr = immediate_binary;
			if (!int_to_binary(&immediate_binary_ptr, num_immediate, 10, FALSE))
			{
				handle_error(ERROR_DATA_INVALID_INTEGER, target_operand, line_number);
			}

			strncpy(&(command[target_index]), immediate_binary_ptr, 10);
		}
		else if (is_valid_label(target_operand))
		{
			target_operand_method = DIRECT_METHOD;
			strncpy(&(command[7]), "011", strlen("001"));
		}
		else if ((num_register = is_register(target_operand)) != -1)
		{
			target_operand_method = REGISTER_DIRECT_METHOD;
			strncpy(&(command[7]), "101", strlen("001"));

			char register_binary[5 + 1 /* null terminator */];
			memset(&register_binary, '0', 5);
			register_binary[5] = '\0';

			char* register_binary_ptr = register_binary;
			int_to_binary(&register_binary_ptr, num_register, 5, FALSE);

			int target_index = 5;
			// If there is only one operand OR both operands are registers, write the target register to the first word.
			if (num_operands == 1 || source_operand_method == REGISTER_DIRECT_METHOD)
			{
				if (num_operands == 2)
				{
					// Since the words should be merged, the last word should be removed.
					command_length -= 12;
					command = (char*)realloc(command, command_length + 1 /* null terminator */);
					command[command_length] = '\0';
				}

				target_index += 12;
			}
			else // Write it to the second word.
			{
				target_index += 24;
			}

			strncpy(&(command[target_index]), register_binary_ptr, 5);
		}
		else
		{
			handle_error(ERROR_OPERAND_INVALID, target_operand, line_number);
		}

		free(target_operand);
	}

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

	insert_new_command(command, num_statements);

	if (command)
		free(command);

	return num_statements;
} /* read_command */

void read_extern(const char* externs, const int line_number)
{
	const int line_length = strlen(externs);

	char* temp = externs;
	char current_label[MAX_LABEL_SIZE + 1 /* null terminator */];
	memset(current_label, '0', MAX_LABEL_SIZE);
	current_label[MAX_LABEL_SIZE] = '\0';

	int current_label_length = 0;
	int current_iterator = 0;
	for (int i = 0; i < line_length; i++)
	{
		if (*temp == ' ' || *temp == '\t' || *temp == '\0')
		{
			temp++;
			continue;
		}

		if (*temp == ',' || i == line_length - 1)
		{
			Error label_error;
			current_label[current_iterator] = '\0';
			if ((label_error = add_label(current_label, -1, FALSE, FALSE, TRUE)) != NO_ERROR)
			{
				handle_error(label_error, current_label, line_number);
			}

			current_label_length = 0;
			current_iterator = 0;
		}
		else
		{
			current_label[current_iterator++] = *temp;
			current_label_length++;
		}

		temp++;
	}
} /* read_extern */

int read_data(const char* data, const int line_number)
{
	const int line_length = strlen(data);
	char* temp = data;

	int data_size = 0;
	int num_data = 0;
	for (int i = 0; i < line_length; i++)
	{
		if (*temp == ',' || i == line_length - 1)
			num_data++;

		temp++;
	}

	temp = data;
	int data_lengths[num_data];
	memset(data_lengths, 0, sizeof(data_lengths));

	int counter = 0;
	for (int i = 0; i < line_length; i++)
	{
		if (*temp != ',' && *temp != ' ' && *temp != '\t' && *temp != '\n' && *temp != '\0')
		{
			if (isdigit(*temp) || (data_lengths[counter] == 0 && *temp == '-'))
			{
				data_lengths[counter]++;
			}
			else
			{
				handle_error(ERROR_DATA_INVALID_INPUT, NULL, line_number);
			}
		}

		if (*temp == ',' || i == (line_length - 1))
		{
			if (data_lengths[counter] == 0)
			{
				handle_error(ERROR_DATA_INVALID_INPUT, NULL, line_number);
			}

			counter++;
		}

		temp++;
	}

	temp = data;
	counter = 0;
	int num_counter = 0;
	int data_vec[num_data];
	char* data_str = malloc(data_lengths[num_counter] + 1 /* null terminator */);
	if (!data_str)
		handle_error(ERROR_ALLOCATION_FAILED, NULL, line_number);

	memset(data_str, '0', data_lengths[num_counter]);
	data_str[data_lengths[num_counter]] = '\0';

	char binary_stream[12 + 1 /* null terminator */];
	memset(binary_stream, '0', 12);
	binary_stream[12] = '\0';

	char* endptr;
	for (int i = 0; i < line_length; i++)
	{
		if (*temp != ',' && *temp != ' ' && *temp != '\t' && *temp != '\n' && *temp != '\0')
		{
			data_str[counter] = *temp;
			counter++;
		}

		if (*temp == ',' || i == (line_length - 1))
		{
			data_vec[num_counter] = strtol(data_str, &endptr, 10);
			if (*endptr != '\0')
			{
				handle_error(ERROR_DATA_INVALID_INPUT, NULL, line_number);
			}

			char* binary_ptr = binary_stream;

			if (!int_to_binary(&binary_ptr, data_vec[num_counter], 12, TRUE))
			{
				handle_error(ERROR_DATA_INVALID_INTEGER, data_str, line_number);
			}

			insert_new_data(binary_stream);

			// Reset the binary stream to prepare it for the next datum
			memset(binary_stream, '0', 12);
			data_size++;

			counter = 0;
			num_counter++;
			if (num_counter != num_data)
			{
				data_str = realloc(data_str, data_lengths[num_counter] + 1 /* null terminator */);
				if (data_str == NULL)
				{
					handle_error(ERROR_ALLOCATION_FAILED, NULL, line_number);
				}

				memset(data_str, '0', data_lengths[num_counter]);
				data_str[data_lengths[num_counter]] = '\0';
			}
		}

		temp++;
	}

	return data_size;
} /* read_data */

int read_string(const char* string, const int line_number)
{
	char* temp = string;
	int string_length = 0;

	while (*temp != '"' && *temp != '\n' && *temp != '\0')
	{
		temp++;
	}

	if (*temp == '\n' || *temp == '\0')
	{
		handle_error(ERROR_STRING_BAD_FORMAT, NULL, line_number);
	}

	temp++;

	char* binary_stream = (char*)malloc(12 + 1);
	if (!binary_stream)
		handle_error(ERROR_ALLOCATION_FAILED, NULL, line_number);

	memset(binary_stream, '0', 12);
	binary_stream[12] = '\0';

	while (*temp != '"' && *temp != '\n' && *temp != '\0')
	{
		int current = *temp;
		if (current > 127)
		{
			handle_error(ERROR_STRING_INVALID_CHARACTER, *temp, line_number);
		}

		string_length++;
		if (!int_to_binary(&binary_stream, current, 12, FALSE))
		{
			handle_error(ERROR_STRING_INVALID_CHARACTER, *temp, line_number);
		}

		insert_new_data(binary_stream);
		temp++;
	}

	if (*temp == '\n' || *temp == '\0')
	{
		handle_error(ERROR_STRING_BAD_FORMAT, NULL, line_number);
	}

	// Strings are closed by the null terminator.
	int_to_binary(&binary_stream, (int)('\0'), 12, FALSE);
	insert_new_data(binary_stream);

	return string_length + 1 /* null terminator */;
} /* read_string */