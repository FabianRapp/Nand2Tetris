## Hack-Language specification
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


