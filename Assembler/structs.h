#pragma once

#define INSTRUCTION_WORD_SIZE		12
#define MAX_LABEL_SIZE				30
#define MAX_LINE_SIZE				80

typedef struct Label* labelPtr;

typedef struct Label
{
	char name[MAX_LABEL_SIZE];
	int address;
	int is_code;
	int is_data;
	int is_extern;
	int is_entry;
	labelPtr next;
} Label;

typedef struct Macro* macroPtr;

typedef struct Macro
{
	char name[MAX_LABEL_SIZE];
	char line[MAX_LINE_SIZE];
	macroPtr next;
} Macro;