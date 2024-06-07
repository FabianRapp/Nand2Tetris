## HIGH TROUGHPUT HACK-ASM ASSEMBLER
This assembler is my project of implementing the assembler for the course
	Nand2Tetris. It is not meant to parse code written by human, but rahter
	the output of a compiler(or simmilar programs). Thus it does not do a
	syntax analyzis.
	The goal of this project is to build and assembler optizied for
	unrealistic large input source files of 10GB or larger. It should work
	on any size. The complexity should stay low since this project has a
	two week deadline.
	Extected by this course is a very naive approch
	that only has 5 hourse of expected work put into the assembler. I want
	to use this to better my undearstanding of multithreaded enviorments,
	thread save data structures and SIMD-instructions.

# Hack-Language specification
This specification is based of  www.nand2tetris.org and the book "The Elements of Computing Systems" by Nisan and Schocken, MIT Press.

The Hack machine language is failrly basic.
Each intstruction has 16 bits. There are two types of instructions identified
by the first bit.

# A-Instruction:
A 0 indicates an "A-instruction". An ***A***-instruction is an
***A***ddress instruction that will place the ***A***-Register for the next half CPU
cyle to either use as data *or* an address(which is determined by the next instruction).
The A-instruction does nothing else by it self and is a prepartion for the more complex "***C***-instruction".
# C-Instruction:
The instruction is a ***C***ompute-instruction if the first bit is 1.
It can be split into 6 indivudual parts
(indexing: 15->left/most sig. bit, 0/least significant bit, 
for ranges: inclusive start..inclusive end):
1. `instruction[15]:` *Identifies* the instruction as *C-instr.* if 1
2. `instruction[13..14]:` *Unused*, set to *1* by *convention*
3. `instruction[12]:`  *Selects from two instruction sets* used for the next parts
4. `instruction[6..11]:` Determinines the *compute* instruction. The
instruction set has These are split into two sets of 18 if instr.[12] is 0
and 10 if instr.[12] is 1. Non specified instructions have undefined
behaivior. The set is provided below.
5. `Instruction[3..5]:` Determines the *write back* of the result of the
computation. Every combination is defined (details below).
6. `Instruction[0..2]:` Determines the *jump*. Every combination is defind (details below).


**Compute Instruction Set**  
*These return values are defined for different selectors.*

**Selector == 0 (Using D and A registers):**

| Operation | Description                               |
|-----------|-------------------------------------------|
| `0`       | Zero                                      |
| `1`       | One                                       |
| `-1`      | Negative one                              |
| `D`       | Value in D register                       |
| `A`       | Value in A register                       |
| `!D`      | NOT D (bitwise NOT of D)                  |
| `!A`      | NOT A (bitwise NOT of A)                  |
| `-D`      | Negative of D                             |
| `-A`      | Negative of A                             |
| `D+1`     | D plus one                                |
| `A+1`     | A plus one                                |
| `D-1`     | D minus one                               |
| `A-1`     | A minus one                               |
| `D+A`     | D plus A                                  |
| `D-A`     | D minus A                                 |
| `A-D`     | A minus D                                 |
| `D&A`     | D AND A (bitwise AND)                     |
| `D|A`     | D OR A (bitwise OR)                       |

**Selector == 1 (Using D register and RAM location addressed by A (RAM[A])):**

| Operation    | Description                               |
|--------------|-------------------------------------------|
| `RAM[A]`     | Value at RAM address A                    |
| `!RAM[A]`    | NOT RAM[A] (bitwise NOT of RAM[A])        |
| `RAM[A]+1`   | RAM[A] plus one                           |
| `RAM[A]-1`   | RAM[A] minus one                          |
| `D+RAM[A]`   | D plus RAM[A]                             |
| `D-RAM[A]`   | D minus RAM[A]                            |
| `RAM[A]-D`   | RAM[A] minus D                            |
| `D&RAM[A]`   | D AND RAM[A] (bitwise AND)                |
| `D|RAM[A]`   | D OR RAM[A] (bitwise OR)                  |


**Destination Set**  
*Where is the result of the computation (output of the ALU) going to be stored?*  

| Destination   | Description                 |
|---------------|-----------------------------|
| `null`        | Not saved                   |
| `RAM[A]`      | Saved to RAM at address A   |
| `D`           | Saved to D register         |
| `D and RAM[A]`| Saved to both D and RAM[A]  |
| `A`           | Saved to A register         |
| `RAM[A] and A`| Saved to both RAM[A] and A  |
| `A and D`     | Saved to both A and D       |
| `A, D, RAM[A]`| Saved to A, D, and RAM[A]   |

**Jump Instruction Set**  
*Condition to set the PC (Program Counter) to the current value in A:*  

| Jump Instruction | Condition                         |
|------------------|-----------------------------------|
| `null`           | No jump                           |
| `JGT`            | Computation > 0                   |
| `JEQ`            | Computation == 0                  |
| `JGE`            | Computation >= 0                  |
| `JLT`            | Computation < 0                   |
| `JNE`            | Computation != 0                  |
| `JLE`            | Computation <= 0                  |
| `JMP`            | Always jump                       |

# CURRENT PLAN:
- 1 Data input thread dumping data into a thread save ring buffer
- 1 Lexer thread reading the ring buffer creating tokens using SIMD.
	Tokens are pushed onto some kind of thread save queue
- N parser threads. On each iteration grap the next avialble token
- for now the parsers will output directly based of some sort of queue.
	if there is time I want to implement a mechanism that reduces write() sys calls

CHANGE:
Make lexer do much more. Later check if lexer can be split to more threads.
Lexer converts to tokens still. Each token is allready the binay data. when it
encounters symbols it gives these to another thread to create the symbol table.
The lexer just saves a reference to the location in the symbol table(a string
is fine for the first implementation).
Since I do not resolve the symols in advance I need to be able to hold hold all
the data at the same time. A linked list of arrays seesm reasonable.
This array could hold the binary data. The lexer fills C instructions and gives
A instructions to a diffrent thread to handle (symbols). The array allocation
would need to be done by the lexer since its the only one with full context of
the since.
Reducing the responsiblity of a single lexer thread can be done later.
Once the lexer and symbol/A-instruction thread joined the main thread the
binary can be written to the final binary file.
Taking side step to learn some cuda.


Weird stuff to keep in mind for implementation:
Space has diffrent, tab and newline have diffrent behavior each:
-newline is the instruction seperator
-tab is treated like any other charecter
-space is comply ignores (oven in variables, "ab" == " a b ")

TODO(X is done, O is not needed anymore, not done and optionals are generally lower):
[x]	Implement a data structure to read data into and the reading thread
[x]	Implement a thread that reads and works on that data inplace without
data races (later this will turn into the lexer)
[x]	Set a first rule of what the lexer should return.
	Even this might result in huge changes later on duo to performence,
	I need a starting point to work with(with the deadline of max 2 weeks,
	tho I should move on faster).
[x]	Implement the token struct
[x]	Implement a basic que for the tokens
[x]	lexer loop exit condition needs rework (also currently borken)
[x]	lexer refactor
[x]	Make lexer append entire file as tokens to queue (bug free)
[x]	Be able to pass the full data though reader, lexer and parsers and write it back to a file without errors (diff src cmp -> no diff)
[x]	think of someway to handle the insane space rules without messing with the simd lexing
[x]	instruction union type
[]	switch from data in form of lines to data in form of strings
[]	thread save binary data array list
[]	thread that takes A-instruction-/symbole-defintion-lines and fills the array list according
[]	thread that reads the array list and writes it to the out file
[O]	lexer should be able to identify '//' also as line termination and set the next line after '\n'
[]	Make a function that takes a token and saves it in binary
[]	parser loop needs an exit condition
[]	Handle Symbols somehow, first naivly
[x]	there is a race condition where the lexer never finishes
[]	make data pipeline faster
[]	init and cleanup functions for ring buffer for readability
[]	reduce usage of volatile
[]	seperate init/cleanup functions for reader, lexer, parsers and data structures
[x](slow as is)	make space removal better
[]	remove dynamic memory usage form queue
[]	make queue array based
[]	lockless queue
[]	consider other options besides joining threads that allow to remove checks within the loops

Data Structures and mechanisms used/planned (if enough time the naive approches
wil be replaced 1 by 1):
Main Buffer data structure: Ring Buffer.
Lexer appends tokens to a queue.
Token:
naive approch:
	A node of a queue representing one line of the source code.
	Is a dynamicly allocated 1:1 copie of the source codes line.
	Has to include some sort of index to indicate the postion.
Token-queue:
naive approch:
	A linked list where the lexer has a pointer to the tail.
	A pointer to the head is stored in the main data strucure.
	Each parser thread has a pointer to that pointer (t_token **) which is protected by a shared mutex.
	The head is advanced by the parsers and the tail by the lexer.
	Some sort of protection when the lexer appends new nodes.
Parser-Threads:
naive approch:
	Take tokens from queue.
	Create binary.
	Check a global index and compare it to the index of the token. On match
	they write the binary to the output.
	(symbols later..)


