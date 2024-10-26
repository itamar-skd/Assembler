#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "enums.h"

/// ==================
/// FIRST-PASS SUMMARY
/// ==================
/// The main purpose of the first-pass is to make a list of all variables.
/// Variable names appear first in a line of assembly code, and their end is marked by a ':' (colon).
/// Legal variable names are those who only consists of letters and numbers. However, the first character must be a letter.
/// The maximum length for a variable name is 31 characters, including the null terminator '\0'.
/// 
/// There are three types of variables: command, data and external.
/// 1.	External variables do not have a value, they are defined in another file.
/// 
/// 2.	Command variables' values correspond to their IC, which is the instruction counter.
///		In other words, the instruction counter is basically the line counter in the output binary file.
///		Every command produces between 1 and 3 lines of binary code, depending on the number of operands (maximum 2) and their addressing methods.
///		The IC is incremented per the number of lines a command takes in the binary file.
/// 
/// 3.	Data variables' values correspond to their DC, which is the data counter, with the addition of the final IC.
///		When a labeled data/string directive is met, the variable takes the current value of the DC (before it's incremented).
///		The DC is incremented per the number of data the directive allocates.
///		For example, ".data 6, -3, 27" increments DC by 3 as it allocates 6, -3 and 27.
///		At the end of the first-pass, the variable's value will be incremented by IC.
///		The reason for this is that data should always be encoded at the end of the binary file.
/// 
/// In addition, the first-pass is also in charge of preparing the ".ob" and ".ext" files.
/// The ".ob" file includes the assembler-translated machine code.
/// The ".ext" file includes labels defined as externals and where they are used in the ".ob" file (their IC).

// First Pass
void first_pass(const char* filename);
void read_line_first_pass(const char* line, int* ic, int* dc, const int line_number);

// Allocation
boolean insert_new_command(const char* data, const int num_instructions);
void insert_new_data(const char* data);

// Commands
int read_command(const OpCode command_type, const char* data, const int line_number);

// Directives
void read_extern(const char* externs, const int line_number);
int read_data(const char* data, const int line_number);
int read_string(const char* string, const int line_number);