// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
// The algorithm is based on repetitive addition.

    @START//-1
    -1;JMP//1
//============UTILS===============================================

// if R0 was negative adds 1 to sign_flags and
// makes R0 its absolute value (R0_ABS)
// returns to address in return_location
(R0_ABS)
    @R0
    D=M;
//--load return_location into A and jump there if R0 is >=0
    @return_location
    A=M;
    D;JGE//return if R0 is >= 0
//---R0/D are negative
//--inverse R0
    @R0
    M=!D;
    M=M+1
//--add 1 to sign_flags since R0 was negative
    @sign_flags
    M=M+1;
//---return
    @return_location//1
    A=M;
    0;JMP

// if R1 was negative adds 2 to sign_flags and
// makes R1 its absolute value (R1_ABS)
// returns to address in return_location
(R1_ABS)
    @R1
    D=M;
//--load return_location into A and jump there if R1 is >=0
    @return_location
    A=M;
    //inverser D and if it is <0now R1 is >= 0, so return
    D;JGE
//---if this reaches R1/D are negative
//---inverse R1
    @R1
    M=!D;
    M=M+1;
//---add 2 to the sing_flags to indicate R1 was negative
    @sign_flags
    M=M+1;
    M=M+1;
//---return to return_location
    @return_location
    A=M;
    0;JMP

//=====================END UTILS=========================

(START)
//--set result to 0
    @R2
    M=0;

//--set flags that indicate the sign of R0 and R1 to 0
@sign_flags
    M=0;
//--abs(R0) and set flag
    @GOT_ABS_R0
    D=A;
    @return_location
    M=D;
    @R0_ABS
    0;JMP
(GOT_ABS_R0)
//--abs(R1)and set falg
    @GOT_ABS_R1
    D=A;
    @return_location
    M=D;
    @R1_ABS
    0;JMP
(GOT_ABS_R1)

//--set R1 as counter i---
    @R1
    D=M;
    @i
    M=D;

//--jump to end if counter is 0--
    @i
    D=M;
    @SIGN_HANDLER
    D;JEQ

//--load counter i into D for loop beginning state
    @i
    D=M;

(LOOP)//loop start
//--save counter i that is currently in D to memory
    @i
    M=D;

//---load R0 into D to add it to R2 afterwards
    @R0
    D=M;

//--adds R0(D) to R2
    @R2
    M=D+M;

//--load counter i into D
    @i
    D=M;

//--decrease counter currently in D
// and jump to loop start if it is not 0 afterwards
@LOOP
    D=D-1;JNE

// after the loop this checks if the result has to be inverted
// this decsions is based of the sign_flags determined before the loop
// if sign_falgs == 0b00 R0 and R1 were both positive, no flip
// if sign_flags == 0b01 or sign_flags == 0b10 either R0 or R1 was negative->
// flip the result in R2
// if sing_flags == 0b11 both R0 and R1 were >= 0 ->no flip
// TODO: also resets R0 and R1 to their original state
(SIGN_HANDLER)
    @sign_flags
    D=M;
    @END
    D;JEQ   // jmp if no sign flasgs
    D=D-1;

    D=D-1;JGT   // if the sign flasgs were == 3 (0b11)
                //both R0 and R1 were negative
                //->no flip
//---finally flip the result since only one multiplier was negative
    @R2
    M=!M;
    M=M+1;

(END)
@END
0; JMP
