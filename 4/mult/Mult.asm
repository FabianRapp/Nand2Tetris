// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
// The algorithm is based on repetitive addition.
// A - Address
// M - Memory
// D - Data
/*
@1          //set A to 1
M=A-1;JEQ   //computes A - 1 -> 0, then
            //store in M, which ich RAM[1] (A is still 1)
            //checks if the computations (A - 1) is 0 by JEQ
            //if so the next instruction is 1
*/

/*
direct;
if (R1 > 0)
    direct = -1;
if (r2 < 0)

    direct = 1;
i = R2;
if (i)
    goto loop;
else
    goto end;
loop:

if (i)
    goto loop;
end:
*/
@2
M=0//reset target
@1
D=M;//counter
///conditon to skip loop
@3 //write counter to register
M=D;

@0//load R0 to add to R2
D=M;

@2//increase R2 by R0
M=D+M;

@3
D=M;
@5??
D=D-1;JMP//somehow specifify target address

//loop condition


