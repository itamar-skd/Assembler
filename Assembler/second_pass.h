#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "enums.h"

/// ===================
/// SECOND-PASS SUMMARY
/// ===================
/// The main purpose of the second-pass is to complete the ".ob" file.
/// Since the assembler is now aware of all program labels and their definitions, it can replace the first with the latter to write computer instructions.
/// The second-pass is also in charge of writing the ".ent" file, which includes all of the entries names and addresses.

// Second Pass
void second_pass(const char* filename);
void read_line_second_pass(const char* line, const int line_number, int* ic);

// Allocation
void write_to_commands(const char* word, int ic);

// Directives
void read_entry(const char* entries, const int line_number);

// Commands
int read_command_second_pass(const OpCode opcode, const char* operands, const int line_number, int ic);