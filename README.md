# Assembler
This is my solution to the final assignment of the C-Course (20465) at The Open University.<br/>
It should be noted that I am **not** affiliated with OpenU, and I do not attend as a student there. I was given this project by a coworker who attended OpenU.<br/>
This project has not been formally graded.

## 1 Getting Started
This project was written on the Ubuntu distro, but it may run on all other distributions of Linux.

To build the solution, open the terminal in the solution folder and run:
```
>  make
```
To run the solution, open the terminal in the build folder and run:
```
>  ./assembler [input file path]
```
The input file path is **relative to the current working directory**.
For example, if you're running this project from: `home/uname/Assembler`:
```
>  ./assembler test
```
Will look for the file: `home/uname/Assembler/test.as`.

## 2 Understanding Assembly Code
Every line in Assembly code is, at maximum, 80 characters long.<br/>
Every line is of the form:
> [OPTIONAL LABEL]: [INSTRUCTION|DIRECTIVE] [OPTIONAL SOURCE OPERAND],[OPTIONAL TARGET OPERAND]
The number of operands varies between every instruction and directive.
### 2.1 Labels
Labels are the Assembly parallel to variables in C.<br/>
Labels are a string of up to 30 alphanumeric characters, with the first character being a letter.<br/>
The value stored in labels is the address to the first line in the translated machine code that the instruction they point at is written.
```
MAIN: mov -13, @r3 ; (ic = 0 -> MAIN = 0)
jmp END ; (ic = 3, 5 is substituted for END)
END: stop ; (ic = 5 -> END = 5)
```
A label may be **local** or **external**.<br/>
Labels marked as entries in one file may be used as external labels in other files.<br/>
An external label in one file takes the value of where it was defined as an entry. See [2.3 Directives](https://github.com/itamar-skd/Assembler?tab=readme-ov-file#23-directives) to see how this is done.
### 2.2 Instructions
Instructions control the flow of the program and instruct the machine what to do.<br/>
The following is a table of all instructions, their opcode (see [3.2 Instructions](https://github.com/itamar-skd/Assembler?tab=readme-ov-file#32-instructions)), and how many operands they take.
| Instruction | Opcode | # Operands |
| --- | --- | --- |
| MOV | 0 | 2 |
| CMP | 1 | 2 |
| ADD | 2 | 2 |
| SUB | 3 | 2 |
| NOT | 4 | 1 |
| CLR | 5 | 1 |
| LEA | 6 | 2 |
| INC | 7 | 1 |
| DEC | 8 | 1 |
| JMP | 9 | 1 |
| BNE | 10 | 1 |
| RED | 11 | 1 |
| PRN | 12 | 1 |
| JSR | 13 | 1 |
| RTS | 14 | 0 |
| STOP | 15 | 0 |

For instructions that take 2 operands, the first operand is the source operand and the second is the target operand.<br/>
For instructions that take 1 operand, the first operand is always the target operand.<br/><br/>
For more information about each instruction, please visit [this page](https://en.wikipedia.org/wiki/X86_instruction_listings).
### 2.3 Directives
Directives are data and storage instruction providers. They are always initiated by a dot.<br/>
Directives take either one or an unlimited number of operands:
| Directive | # Operands | Purpose | Usage |
| --- | --- | --- | --- |
| .string | 1 | Store a string* of characters in memory | .string "abcdef" |
| .data | Unlimited | Store integers** in memory | .data 6, 7, -13, 25 |
| .extern | Unlimited | Mark a label as an external (see [2.1 Labels](https://github.com/itamar-skd/Assembler?tab=readme-ov-file#21-labels)) | .extern LENGTH, L1 |
| .entry | Unlimited | Mark a label as an entry (see [2.1 Labels](https://github.com/itamar-skd/Assembler?tab=readme-ov-file#21-labels)) | .entry MAIN, END |

\* Recall that a string is terminated by the null-terminator: `'\0'`.<br/>
\*\* Integers are converted to binary using the [Two's complement method](https://en.wikipedia.org/wiki/Two%27s_complement).
### 2.4 Macros
Macros are the Assembly parallels to functions in C.<br/>
A macro is declared using the `mcro` keyword, and terminated using the `endmcro` keyword.
```
mcro <macro name>
  ...
  ...
endmcro
```
You can paste the block of code between `mcro` and `endmcro` around your program by writing the macro's name. See [4.1 Preassembler](https://github.com/itamar-skd/Assembler?tab=readme-ov-file#41-preassembler).<br/>
`Program.as` File:
```
mcro m1
  inc K
  sub @r1,@r2
endmcro
...
m1
...
m1
END: stop
K: .data 6
```
`Program.am` File:
```
...
inc K
sub @r1,@r2
...
inc K
sub @r1,@r2
END: stop
K: .data 6
```
## 3 Understanding Machine Code
A machine does not understand ASCII symbols, it understands 1's and 0's. More specifically, it understand 1's and 0's that are arranged in a predetermined order.<br/>
The point of the Assembler is to translate Assembly code into Machine code.
### 3.1 An example of Machine Code
The following is the first 10 lines of the `test.ob` file outputted by running the Assembler:
```
101000001100
000110000000
000010000110
000100101100
000001000110
111111101100
000101001100
101001110100
000000000001
000101001100
```
Just like you (probably) can't read this, the computer also couldn't before it was told **how** to read it.
### 3.2 Instructions
Instructions and directives are translated into words.<br/>
Words come in many different formats, but the first word of an instruction is always the same.
The first word of an instruction is a 12-bit-long binary stream that is arranged in the following order:
| SOURCE OPERAND METHOD | OPCODE | TARGET OPERAND METHOD | A, R, E |
| :---: | :---: | :---: | :---: |
| 3 bits | 4 bits | 3 bits | 2 bits |

Other than telling the machine **what** to do (opcode), it also tells the machine how to read the next words, which describe the source and target operands.

### 3.2.1 Operand Methods
There are 3 operand methods:
| METHOD | BINARY REPRESENTATION | DESCRIPTION |
| :---: | :---: | :---: |
| Immediate | 001 | Integer Value |
| Direct | 011 | Local/External Address |
| Register Direct | 101 | Register Address |

If an operand is of an immediate method, it will be written as:
| Integer in Binary | A, R, E |
| :---: | :---: |
| 10 bits | 2 bits |

If an operand is of a direct method, it will be written as:
| Address | A, R, E |
| :---: | :---: |
| 10 bits | 2 bits |

If an operand is of a register direct method, it will be written as:
| Address of Source Register | Address of Target Register | A, R, E |
| :---: | :---: | :---: |
| 5 bits | 5 bits | 2 bits |

Every operand adds 1 word, **unless** both operands are of the register direct method.<br/>
In that case, both words are merged into one.

### 3.2.2 A, R, E
A, R, E is used for the linkage process - it describes where the content of the word is located.
| | METHOD | BINARY REPRESENTATION | DESCRIPTION |
| :---: | :---: | :---: | :---: |
| A | Absolute | 00 | Immediate Values (Integer/Register Addresses) |
| R | Relocatable | 10 | Local Variable Address |
| E | External | 01 | External Variable Address |

### 3.2.3 Instruction Word Example
Consider the following assembly code:
```
mov @r3, LENGTH
add @r1, @r2
LENGTH: .data 6
```
This code generates the following machine code:
| IC | Assembly Code | Description | Machine Code |
| :---: | :---: | :---: | :---: |
| 0 | mov @r3, LENGTH | First word | 101000001100 |
| 1 | | Source @r3 | 000110000000 |
| 2 | | Target LENGTH | 000000010110 |
| 3 | add @r1, @r2 | First Word | 101001010100 |
| 4 | | Source @r1 AND Target @r2 | 000010001000 |
| 5 | LENGTH: .data 6 | 6 in Binary | 000000000110 |

### 3.3 Directives
If the first word does not follow the first instruction word format, it should be read as a `.data` or `.string` directive.<br/>
Data and string directives take all 12 bits and write the binary representation of that character.<br/>
The number of words produced by a `.data` or `.string` directive is equal to the amount of integers/characters wanting to be stored.<br/><br/>
**Note:** `.extern` and `.entry` directives do not translate into machine code. They are only to be used by the assembler.
### 3.3.1 `.data` and `.string` Directive Word Example
Consider the following assembly code:
```
.data 6, 13, -7
.string "abc"
```
This code generates the following machine code:
| DC | Assembly Code | Description | Machine Code |
| :---: | :---: | :---: | :---: |
| 0 | .data 6, 13, -7 | 6 | 000000000110 |
| 1 | | 13 | 000000001101 |
| 2 | | -7 | 111111111001 |
| 3 | .string "abc" | 'a' = 97 | 000001100001 |
| 4 | | 'b' = 98 | 000001100010 |
| 5 | | 'c' = 99 | 000001100011 |
| 6 | | '\0' = 0 | 000000000000 |

## 4 Understanding the Assembler
Now that we understand assembly and machine code, it's time to make the connection between the two.<br/>
There are a few problems we will need to face, which we will get into in every stage.
### 4.1 Preassembler
The preassembler's job is to set-up the input file.<br/>
The assembler itself doesn't read macros. It wants to receive pure assembly code so it can execute it line by line. The preassembler's purpose is to spread these macros where they are called.<br/>
The work done by the preassembler is written to a `Program.am` file, that is passed to the First-Pass and Second-Pass functions.<br/><br/>
To see an example of the output of the preassembler, please refer to [2.4 Macros](https://github.com/itamar-skd/Assembler?tab=readme-ov-file#24-macros).
### 4.2 First-Pass
As the name suggests, the first-pass routine is the first routine to properly read through the assembly file.<br/>
Here, we tackle a new problem: Labels do not have to be defined before they are used.<br/>
This assembly code, for example, is completely valid:
```
mov @r3, LENGTH
LENGTH: .data 6
```
Although `LENGTH` was defined after the `mov` instruction, the assembler will translate this into valid machine code.<br/>
But the assembler reads the assembly code line-by-line, how does it know what to write as the value of `LENGTH` to the output `Program.ob` file?<br/>
Well, here's the trick: it doesn't. The main purpose of the first-pass routine is to **write down information about local and external labels**. What is the label's address? Is it local or external? Should it be used as an entry for other files? Everything is written down and memorized in the first-pass routine.<br/>
Along the way, first-pass also starts writing machine code for values and addresses it already holds. Those are:
1. First words
2. Immediate and Register Direct operands
3. Data and Strings

Additionally, the first-pass also allocates space for where Direct operand words should go.
Finally, the first-pass also writes the `.ext` file, see [5.3 `Program.ext` File](https://github.com/itamar-skd/Assembler?tab=readme-ov-file#53-programext-file).

### 4.2.1 Post First-Pass Example
Consider the following code:
```
.extern W
mov @r3, LENGTH
inc W
.entry LENGTH
add @r1, @r2
LENGTH: .data 6
```
After the first-pass, the following machine code is generated:
| IC | Assembly Code | Description | Machine Code |
| :---: | :---: | :---: | :---: |
| 0 | .extern W | No machine code is generated | |
| 0 | mov @r3, LENGTH | First word | 101000001100 |
| 1 | | Source @r3 | 000110000000 |
| 2 | | Target LENGTH | ? |
| 3 | inc W | First word | 000011101100 |
| 4 | | Target W | ? |
| 5 | .entry LENGTH | No machine code is generated | |
| 5 | add @r1, @r2 | First Word | 101001010100 |
| 6 | | Source @r1 AND Target @r2 | 000010001000 |
| 7 | LENGTH: .data 6 | 6 in Binary | 000000000110 |

After the first-pass, the following label table has been written down:
| Label | Address | Is Instruction? | Is Data? | Is External? | Is Entry? |
| :---: | :---: | :---: | :---: | :---: | :---: |
| W | -1 | 0 | 0 | 1 | ? |
| LENGTH | 7 | 0 | 1 | 0 | ? |

### 4.3 Second-Pass
Now that we're aware of all of the labels and the addresses they correspond to, the second-pass routine's objective is to fill in for the missing holes in the machine code.<br/>
Additionally, the labels can now be marked as entry/non-entry to prepare the `.ent` file, see [5.4 `Program.ent` File](https://github.com/itamar-skd/Assembler?tab=readme-ov-file#54-programent-file).

### 4.3.2 Post Second-Pass Example
Consider the code from before.
```
.extern W
mov @r3, LENGTH
inc W
.entry LENGTH
add @r1, @r2
LENGTH: .data 6
```
After the second-pass, the following machine code is generated:
| IC | Assembly Code | Description | Machine Code |
| :---: | :---: | :---: | :---: |
| 0 | .extern W | No machine code is generated | |
| 0 | mov @r3, LENGTH | First word | 101000001100 |
| 1 | | Source @r3 | 000110000000 |
| 2 | | Target LENGTH | 000000011110 |
| 3 | inc W | First word | 000011101100 |
| 4 | | Target W | 000000000001 |
| 5 | .entry LENGTH | No machine code is generated | |
| 5 | add @r1, @r2 | First Word | 101001010100 |
| 6 | | Source @r1 AND Target @r2 | 000010001000 |
| 7 | LENGTH: .data 6 | 6 in Binary | 000000000110 |

After the second-pass, the following label table has been written down:
| Label | Address | Is Instruction? | Is Data? | Is External? | Is Entry? |
| :---: | :---: | :---: | :---: | :---: | :---: |
| W | -1 | 0 | 0 | 1 | 0 |
| LENGTH | 7 | 0 | 1 | 0 | 1 |

## 5 Output Files
### 5.1 `Program.am` File
The `Program.am` file is the result of the work done by the preassembler routine.<br/>
To see an example of what the `program.am` looks like, please refer to [2.4 Macros](https://github.com/itamar-skd/Assembler?tab=readme-ov-file#24-macros).
### 5.2 `Program.ob` File
The `Program.ob` file is the translated machine code.<br/><br/>
This, for example, is the first 10 lines of code generated by running the example test file included in this repository:
```
101000001100
000110000000
000010000110
000100101100
000001000110
111111101100
000101001100
101001110100
000000000001
000101001100
```
To see the rest, feel free to run the program yourself.
### 5.3 `Program.ext` File
The `Program.ext` file provides information about external variables used by the `Program.as` file.<br/>
Such information is limited to what these variables' names are and where they are used in the machine code.<br/>
This information is crucial for the linking stage, which is not part of this project.<br/><br/>
This, for example, is the `Program.ext` file generated by running the example test file included in this repository:
> W       8<br/>
> L3      12<br/>
> W       24<br/>
### 5.4 `Program.ent` File
The `Program.ent` file provides information about entry variables defined in the `Program.as` file.<br/>
Such information is limited to what these variables' names are and what their address is.<br/>
This information is crucial for the linking stage, which is not part of this project.<br/><br/>
This, for example, is the `Program.ext` file generated by running the example test file included in this repository:
> LENGTH  33 <br/>
> LOOP    3
