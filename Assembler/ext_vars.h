#pragma once

#include "structs.h"
#include "enums.h"

// Constants
extern const char* filename;
extern const char* command_names[NUM_COMMANDS];

// Output Files
extern boolean externs_file_exists;
extern boolean entries_file_exists;

// Assembler
extern char* command_instructions;
extern char* data_instructions;
extern labelPtr firstLabel;
extern macroPtr firstMacro;