// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.

//SCREEN == screen base address == 1638DD4
//KBD == keyboard base address == 24576

// sets pixel color variable and
// initial color to white (0)
// black would be -1
    @pixel_val
    M=0;//init color == white

@MAIN_LOOP
    0;JMP
//==================utils===================

//=================end utils===============
//=================keyboard togglle=========
//--load keyboard data since if there is any the screen should be black
//--otherwise white
(KEYBOARD_TOGGLE)
    @KBD
    D=M;
    @NO_PRESSED_KEY
    D;JEQ//if the keyboard buffer is empty jump to handler
    @pixel_val//else set color to -1
    M=-1;
    @SCREEN_FILL
    0;JMP
(NO_PRESSED_KEY)
    @pixel_val
    M=0; //set color to 0 since no keypress
    @SCREEN_FILL
    0;JMP
//================end keyboard toggle=======
//=================start render=============
(SCREEN_FILL)
    @SCREEN//set screen address as start counter i
    D=A;
    @i
    M=D;
(LOOP)//loop start
// get the pixel color into D
    @pixel_val//20
    D=M;
    @i//select next pixel
    A=M;
//fill memory location with pixel value
    M=D;
// when keyboard buffer is reach screen buffer is over
// and the loop should terminate
// -> check if i-KBD < 0 -> jump to loop
    @i
    M=M+1;//26
    D=M; //i++
    @KBD
    D=D-A;//29 //check if the keyboard buffer is reached
    @LOOP
    D;JLT//if not go to start of loop
//======end render loop================
(MAIN_LOOP)
    @KEYBOARD_TOGGLE//32
    0;JMP//continues loop

(END)
    @END
    0;JMP

