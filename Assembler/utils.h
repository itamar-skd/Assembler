#pragma once
#include "enums.h"
#include "structs.h"

#define nullptr ((void*)0)

// Preassembler
inline boolean is_assembly_file(const char* filename);
inline void get_spread_filename(const char* filename, char* spread_filename);
Error get_macro_name(const char* line, char** macro_name);
macroPtr get_macro(const char* line);
void insert_macro_line(const char* line, const char* macro_name);

// Utility Functions
char* change_file_extension(char* filename, char* prev_extension, char* new_extension);
void twos_complement(char** output);
boolean int_to_binary(char** output, int num, int length, boolean use_twos_complement);

int get_line_length(const char* line);

// Line Reading
boolean is_space(const char c);
void skip_spaces(char** line);
boolean do_ignore_line(char** line);
int has_label(char* line, int line_length);
int is_number(const char* str);
int is_register(const char* str);
boolean is_valid_label(const char* str);

// Commands
OpCode is_command(const char* str);
int get_num_operands(const OpCode command_type);

// Directives
boolean read_directive(const char** line_ptr, int* line_length, const char* directive);

// Labels
boolean label_exists(const char* str);
Error add_label(const char* label, const int address, boolean is_code, boolean has_data, boolean is_extern);
int get_label_address(const char* label_name);
Error is_label_extern(const char* label_name, boolean* is_extern);
labelPtr set_label_as_entry(const char* label_name);

// Output Files
void write_external_to_file(const char* external_name, const int ic);
void write_entries_to_file(const char* entry_name, const int address);
void write_ob_file();

// Error Handling
void handle_error(Error error, const char* data, const int line_number);
inline void log_error(const char* error_message, const int line_number);