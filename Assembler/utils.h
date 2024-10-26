#pragma once
#include "enums.h"
#include "structs.h"

#define nullptr ((void*)0)

// Preassembler
boolean is_assembly_file(const char* filename);
Error get_macro_name(char* line, char** macro_name);
macroPtr get_macro(char* line);
void insert_macro_line(const char* line, const char* macro_name);

// Utility Functions
char* change_file_extension(const char* filename, const char* prev_extension, const char* new_extension);
char* get_full_path(char* filename);
void twos_complement(char** output);
boolean int_to_binary(char** output, int num, int length, boolean use_twos_complement);
int get_line_length(char* line);

// Line Reading
boolean is_space(char c);
boolean is_end_of_line(char c);
void skip_spaces(char** line);
boolean do_ignore_line(char** line);
int has_label(char* line, int line_length);
int is_number(char* str);
int is_register(const char* str);
boolean is_valid_label(char* str);

// Commands
OpCode is_command(const char* str);
int get_num_operands(const OpCode command_type);

// Directives
boolean read_directive(char** line_ptr, int* line_length, const char* directive);

// Labels
boolean label_exists(const char* str);
Error add_label(char* label, const int address, boolean is_code, boolean has_data, boolean is_extern);
int get_label_address(const char* label_name);
Error is_label_extern(const char* label_name, boolean* is_extern);
labelPtr set_label_as_entry(const char* label_name);

// Output Files
void write_external_to_file(char* external_name, const int ic);
void write_entries_to_file(char* entry_name, const int address);
void write_ob_file();

// Error Handling
void handle_error(Error error, const char* data, const int line_number);
void log_error(const char* error_message, const int line_number);