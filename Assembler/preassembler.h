#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include "enums.h"

/// ====================
/// PREASSEMBLER SUMMARY
/// ====================
/// The Preassembler is the first assembler routine.
/// It's purpose is to spread the input file's macros. A macro is Assembly's parallel to a function. It's a pasteable block of code.
/// The following is the structured of a macro:
///		mcro <macro name>
///			...
///		endmcro
///
/// A macro must be opened with the "mcro" keyword followed by the macro's name, and it must be terminated by the "endmcro" keyword.
/// A macro must be defined before it is used.
/// To use a macro, one must simply write its name, for example:
///		mcro m1
///			inc K
///		endmcro
///		mov -13,@r3
///		m1 
///		END: stop
///		...
///
/// The Preassembler generates a ".am" file, with the macros spreaded.



char* preassembler();

void read_line_preassembler(FILE* spread_file, char* line, const int line_number, boolean* macro_flag, char** macro_name);