#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ext_vars.h"
#include "utils.h"
#include "preassembler.h"
#include "first_pass.h"
#include "second_pass.h"

// Input assembly file name
const char* filename = NULL;
const char* command_names[NUM_COMMANDS] = { 
    "mov", "cmp", "add", "sub", 
    "not", "clr", "lea", "inc", 
    "dec", "jmp", "bne", "red", 
    "prn", "jsr", "rts", "stop" 
};

boolean externs_file_exists = FALSE;
boolean entries_file_exists = FALSE;

char* command_instructions = NULL;
char* data_instructions = NULL;
labelPtr firstLabel = NULL;
macroPtr firstMacro = NULL;

int main(int argc, char* argv[]) {
    filename = get_full_path(change_file_extension(argv[1], "", ".as"));

    command_instructions = (char*)malloc(1);
    if (!command_instructions)
        handle_error(ERROR_ALLOCATION_FAILED, NULL, 0);

    command_instructions[0] = '\0';
    data_instructions = (char*)malloc(1);
    if (!data_instructions)
        handle_error(ERROR_ALLOCATION_FAILED, NULL, 0);

    data_instructions[0] = '\0';

    char* spread_filename = preassembler();
    first_pass(spread_filename);
    second_pass(spread_filename);

    free(spread_filename);

    /*first_pass("/home/shaitamar/projects/Assembler/assembly/test.as");
    second_pass("/home/shaitamar/projects/Assembler/assembly/test.as");*/
    
    write_ob_file();
    printf("%s 1 \n", filename);

    exit(EXIT_SUCCESS);
}