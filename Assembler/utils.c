#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>

#include "utils.h"
#include "ext_vars.h"

boolean is_assembly_file(const char* filename)
{
	int filename_length = strlen(filename);
	return strcmp(&filename[filename_length - 3], ".as") == 0;
}

Error get_macro_name(char* line, char** macro_name)
{
	int macro_length = 0;
	char* temp = line;

	while (!is_space(*temp) && *temp != '\n' && *temp != '\0')
	{
		macro_length++;
		temp++;
	}

	if (*macro_name)
	{
		free(*macro_name);
		*macro_name = NULL;
	}

	*macro_name = (char*)malloc(macro_length + 1 /* null terminator */);
	if (!(*macro_name))
		handle_error(ERROR_ALLOCATION_FAILED, NULL, 0);

	(*macro_name)[macro_length] = '\0';
	memcpy(*macro_name, line, macro_length);

	if (!is_valid_label(*macro_name))
		return ERROR_MACRO_INVALID;

	return NO_ERROR;
}

macroPtr get_macro(char* line)
{
	if (!line)
		return NULL;

	if (*line == '.' || is_command(line) != UNKNOWN_COMMAND)
		return NULL;

	macroPtr temp = firstMacro;
	int line_length = get_line_length(line);

	while (temp)
	{
		if (strlen(temp->name) == line_length)
		{
			if (strncmp(temp->name, line, line_length) == 0)
				return temp;
		}

		temp = temp->next;
	}

	return NULL;
}

void insert_macro_line(const char* line, const char* macro_name)
{
	macroPtr newMacroLine = (macroPtr)malloc(sizeof(Macro));
	if (!newMacroLine)
		handle_error(ERROR_ALLOCATION_FAILED, NULL, 0);

	memset(newMacroLine, 0, sizeof(Macro));
	strcpy(newMacroLine->name, macro_name);
	strcpy(newMacroLine->line, line);
	newMacroLine->next = NULL;

	if (!firstMacro)
	{
		firstMacro = newMacroLine;
		return;
	}

	macroPtr temp = firstMacro;
	while (temp->next)
	{
		temp = temp->next;
	}

	temp->next = newMacroLine;
}

char* change_file_extension(const char* filename, const char* prev_extension, const char* new_extension)
{
	size_t prev_filename_length = strlen(filename);
	int new_filename_length = prev_filename_length - strlen(prev_extension) + strlen(new_extension);

	char* new_filename = malloc(new_filename_length + 1 /* null terminator */);
	if (!new_filename)
		handle_error(ERROR_ALLOCATION_FAILED, NULL, 0);

	memset(new_filename, '0', new_filename_length);
	new_filename[new_filename_length] = '\0';

	memcpy(new_filename, filename, prev_filename_length - strlen(prev_extension));
	strcpy(&(new_filename[prev_filename_length - strlen(prev_extension)]), new_extension);
	return new_filename;
}

char* get_full_path(char* filename)
{
	char cwd[PATH_MAX];
	if (!getcwd(cwd, sizeof(cwd)))
		handle_error(ERROR_ALLOCATION_FAILED, NULL, 0);

	char* full_path;
	asprintf(&full_path, "%s/%s", cwd, filename);
	return full_path;
}

void twos_complement(char** output)
{
	if (output == NULL || *output == NULL)
		return;

	int length = strlen(*output);
	for (int i = 0; i < length; i++)
	{
		(*output)[i] = (*output)[i] == '1' ? '0' : '1';
	}

	for (int i = length - 1; i >= 0; i--) {
		if ((*output)[i] == '0') {
			(*output)[i] = '1';
			break;
		}
		else {
			(*output)[i] = '0';
		}
	}
}

boolean int_to_binary(char** output, int num, int length, boolean use_twos_complement)
{
	if (output == NULL || *output == NULL || length > strlen(*output))
		return FALSE;

	int max = use_twos_complement ? (pow(2, length - 1) - 1) : (pow(2, length) - 1);
	int min = -max;

	if (min > num || max < num)
	{
		return FALSE;
	}

	(*output)[length] = '\0';

	boolean is_negative = FALSE;
	if (num < 0)
	{
		is_negative = TRUE;
		num = fabs(num);
	}

	for (int i = length - 1; i >= 0; i--) {
		(*output)[i] = (num & 1) ? '1' : '0';
		num >>= 1;
	}

	if (is_negative)
		twos_complement(output);

	return TRUE;
} /* int_to_binary */

int get_line_length(char* line)
{
	char* temp = line;
	int length = 0;
	while (*temp != '\n' && *temp != '\0')
	{
		length++;
		temp++;
	}

	return length;
}

boolean is_space(const char c)
{
	return c == ' ' || c == '\t';
}

void skip_spaces(char** line)
{
	if (!line || !*line)
		return;

	char* temp = *line;

	// Move the pointer to skip leading spaces and tabs
	while (*temp == ' ' || *temp == '\t') {
		temp++;
	}

	// Shift the remaining characters to the beginning of the string
	memmove(*line, temp, strlen(*line) + 1 /* null terminator */);
} /* skip_spaces */

boolean do_ignore_line(char** line)
{
	skip_spaces(line);

	return (**line == ';' || **line == '\n' || **line == '\0');
} /* do_ignore_line */

int has_label(char* line, int line_length)
{
	int label_length = 0;
	char* temp = line;
	while (*temp != '\0')
	{
		if (*temp == ':')
		{
			return label_length;
		}

		label_length++;
		temp++;
	}

	return -1;
} /* has_label */

int is_number(char* str)
{
	if (!str || *str == '\0')
		return INT_MIN;

	char* temp = str;
	if (!isdigit(*temp) && *temp != '-')
		return INT_MIN;

	temp++;

	while (*temp != '\0')
	{
		if (!isdigit(*temp))
		{
			return INT_MIN;
		}

		temp++;
	}

	char* endptr = nullptr;
	int num = strtol(str, &endptr, 10);
	if (*endptr != '\0')
		return INT_MIN;

	return num;
} /* is_number */

int is_register(const char* str)
{
	if (!str)
		return -1;

	boolean is_register = (strlen(str) == 3
		&& strncmp(str, "@r", strlen("@r")) == 0
		&& isdigit(str[2]) && str[2] > '0' && str[2] <= '8');

	if (!is_register)
		return -1;

	char* endptr = nullptr;
	int register_num = strtol(&(str[2]), &endptr, 10);
	if (*endptr != '\0')
		return -1;

	return register_num;
} /* is_register */

boolean is_valid_label(char* str)
{
	if (!str)
		return FALSE;

	char* temp = str;
	if (!isalpha(*temp))
		return FALSE;

	while (*temp != '\0')
	{
		if (!isalpha(*temp) && !isdigit(*temp))
			return FALSE;

		temp++;
	}

	return TRUE;
} /* is_valid_label */

OpCode is_command(const char* str)
{
	for (int i = 0; i < NUM_COMMANDS; i++)
	{
		if (strncmp(str, command_names[i], strlen(command_names[i])) == 0)
		{
			return i;
		}
	}

	return UNKNOWN_COMMAND;
} /* is_command */

int get_num_operands(const OpCode opcode)
{
	switch (opcode)
	{
		case MOV:
		case CMP:
		case ADD:
		case SUB:
		case LEA:
			return 2;

		case NOT:
		case CLR:
		case INC:
		case DEC:
		case JMP:
		case BNE:
		case RED:
		case PRN:
		case JSR:
			return 1;

		case RTS:
		case STOP:
			return 0;

		default:
			return -1;
	}
}

boolean read_directive(char** line_ptr, int* line_length, const char* directive)
{
	char* line = *line_ptr;
	const int directive_length = strlen(directive);

	// Check that there exists a space/tab between the directive's name and the operands
	if (!is_space(*(line + directive_length)))
		return FALSE;

	// Advance the line pointer to ahead of the directive's name
	*line_ptr += directive_length + 1;
	*line_length -= directive_length + 1;

	return TRUE;
}

boolean label_exists(const char* str)
{
	labelPtr temp = firstLabel;

	while (temp)
	{
		if (strcmp(temp->name, str) == 0)
			return TRUE;

		temp = temp->next;
	}

	return FALSE;
} /* label_exists */

Error add_label(char* label, const int address, boolean is_code, boolean is_data, boolean is_extern)
{
	if (label_exists(label))
	{
		return ERROR_LABEL_MULTIPLE_DECLARATIONS;
	}

	if (!is_valid_label(label))
		return ERROR_LABEL_INVALID;

	// allocate new label
	labelPtr newLabel = (labelPtr)malloc(sizeof(Label));
	if (!newLabel)
		handle_error(ERROR_ALLOCATION_FAILED, NULL, 0);

	memset(newLabel, 0, sizeof(Label));
	strcpy(newLabel->name, label);
	newLabel->address = address;
	newLabel->is_code = is_code;
	newLabel->is_data = is_data;
	newLabel->is_extern = is_extern;
	newLabel->is_entry = FALSE; // This is not determined at this pass.
	newLabel->next = nullptr;

	// if there are no labels, set the first label to the new label
	if (!firstLabel)
	{
		firstLabel = newLabel;
		return NO_ERROR;
	}

	// if there are existing labels, add the new label to the end
	labelPtr temp = firstLabel;
	while (temp->next)
	{
		temp = temp->next;
	}

	temp->next = newLabel;
	return NO_ERROR;
} /* add_label */

int get_label_address(const char* label_name)
{
	labelPtr temp = firstLabel;
	int label_length = strlen(label_name);

	while (temp)
	{
		if (label_length != strlen(temp->name))
		{
			temp = temp->next;
			continue;
		}

		if (strcmp(temp->name, label_name) == 0)
			return temp->address;

		temp = temp->next;
	}

	return -1;
} /* get_label_address */

Error is_label_extern(const char* label_name, boolean* is_extern)
{
	labelPtr temp = firstLabel;
	const int label_length = strlen(label_name);

	while (temp)
	{
		if (label_length != strlen(temp->name))
		{
			temp = temp->next;
			continue;
		}

		if (strcmp(temp->name, label_name) == 0)
		{
			*is_extern = temp->is_extern;
			return NO_ERROR;
		}

		temp = temp->next;
	}

	return ERROR_LABEL_UNDEFINED;
} /* is_label_extern */

labelPtr set_label_as_entry(const char* label_name)
{
	labelPtr temp = firstLabel;
	const int label_length = strlen(label_name);

	while (temp)
	{
		if (label_length != strlen(temp->name))
		{
			temp = temp->next;
			continue;
		}
		
		// If the labels' name match, set that label as an entry.
		if (strcmp(label_name, temp->name) == 0)
		{
			temp->is_entry = TRUE;
			return temp;
		}

		temp = temp->next;
	}

	return NULL;
} /* set_label_as_entry */

void write_external_to_file(char* external_name, const int ic)
{
	char* output_filename = change_file_extension(filename, ".as", ".ext");
	FILE* output_file;

	if (!externs_file_exists)
	{
		output_file = fopen(output_filename, "w");
		externs_file_exists = TRUE;
	}
	else
		output_file = fopen(output_filename, "a");

	if (!output_file)
		handle_error(ERROR_FILE_CANT_WRITE, output_filename, 0);

	char* newLine;
	asprintf(&newLine, "%-8s%d\n", external_name, ic);
	fwrite(newLine, sizeof(char), strlen(newLine), output_file);

	free(output_filename);
	fclose(output_file);
}

void write_entries_to_file(char* entry_name, const int address)
{
	char* output_filename = change_file_extension(filename, ".as", ".ent");
	FILE* output_file;

	if (!entries_file_exists)
	{
		output_file = fopen(output_filename, "w");
		entries_file_exists = TRUE;
	}
	else
		output_file = fopen(output_filename, "a");

	if (!output_file)
		handle_error(ERROR_FILE_CANT_WRITE, output_filename, 0);

	char* newLine;
	asprintf(&newLine, "%-8s%d\n", entry_name, address);
	fwrite(newLine, sizeof(char), strlen(newLine), output_file);

	free(output_filename);
	fclose(output_file);
}

void write_ob_file()
{
	char* output_filename = change_file_extension(filename, ".as", ".ob");
	FILE* output_file = fopen(output_filename, "w");
	if (!output_file)
		handle_error(ERROR_FILE_CANT_WRITE, output_filename, 0);

	char temp[12 + 1 /* null terminator */];
	memset(temp, '0', 12);
	temp[12] = '\n';

	char* command_temp = command_instructions;
	char* data_temp = data_instructions;

	size_t length_command = strlen(command_temp);
	size_t length_data = strlen(data_temp);

	const int num_commands = length_command / 12;
	const int num_data = length_data / 12;

	for (int i = 0; i < num_commands; i++)
	{
		memcpy(temp, command_temp, 12);
		fwrite(temp, sizeof(char), 13, output_file);
		command_temp += 12;
		length_command -= 12;
	}

	for (int i = 0; i < num_data; i++)
	{
		memcpy(temp, data_temp, 12);
		// If it's the last datum, don't write the \n character
		fwrite(temp, sizeof(char), (length_data == 12) ? 12 : 13, output_file);
		data_temp += 12;
		length_data -= 12;
	}

	free(output_filename);
	fclose(output_file);
}

void handle_error(Error error, const char* data, const int line_number)
{
	char* error_message;

	switch (error)
	{
		case ERROR_ALLOCATION_FAILED:
			asprintf(&error_message, "Allocation failed.");
			break;
		case ERROR_FILE_INVALID:
			asprintf(&error_message, "Invalid file: \"%s\".", data);
			break;
		case ERROR_FILE_NOT_ASSEMBLY:
			asprintf(&error_message, "Not an assembly file: \"%s\".", data);
			break;
		case ERROR_FILE_CANT_WRITE:
			asprintf(&error_message, "Couldn't create new spread file: \"%s\".", data);
			break;
		case ERROR_MACRO_INVALID:
			asprintf(&error_message, "Invalid macro name: \"%s\".", data);
			break;
		case ERROR_MACRO_UNTERMINATED:
			asprintf(&error_message, "Previously declared macro was not terminated: \"%s\".", data);
			break;
		case ERROR_MACRO_UNSTARTED:
			asprintf(&error_message, "\"endmcro\" statement for a macro that was never declared.");
			break;
		case ERROR_DIRECTIVE_UNKNOWN:
			asprintf(&error_message, "Uknown Directive: \"%s\".", data);
			break;
		case ERROR_DIRECTIVE_UNSEPARATED:
			asprintf(&error_message, "Directive unseparated from Operands.");
			break;
		case ERROR_COMMAND_UNKNOWN:
			asprintf(&error_message, "Uknown Command: \"%s\".", data);
			break;
		case ERROR_DATA_INVALID_INPUT:
			asprintf(&error_message, "Invalid data.");
			break;
		case ERROR_DATA_INVALID_INTEGER:
			asprintf(&error_message, "Invalid Data. Number too small/large: \"%s\".", data);
			break;
		case ERROR_STRING_BAD_FORMAT:
			asprintf(&error_message, "Bad string format.");
			break;
		case ERROR_STRING_INVALID_CHARACTER:
			asprintf(&error_message, "Invalid string character input: \'%s\'.", data);
			break;
		case ERROR_LABEL_INVALID:
			asprintf(&error_message, "Invalid label name: \"%s\".", data);
			break;
		case ERROR_LABEL_UNDEFINED:
			asprintf(&error_message, "Label not defined: \"%s\".", data);
			break;
		case ERROR_LABEL_MULTIPLE_DECLARATIONS:
			asprintf(&error_message, "Multiple label declarations for label: \"%s\".", data);
			break;
		case ERROR_OPERAND_INVALID:
			asprintf(&error_message, "Invalid operand: \"%s\".", data);
			break;
		case NO_ERROR:
			return;
	}

	log_error(error_message, line_number);
	exit(-1);
}

void log_error(const char* error_message, const int line_number)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	printf("An unexpected error was detected: %s\n", error_message);
	printf("Line number in input file: %d\n", line_number);
}