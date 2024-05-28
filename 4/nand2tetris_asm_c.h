// Made by Fabian Rapp
// This is meant to learn the nand2tetris hack assembly language
// It should provide a little more flexibility and acessability that the
// provided simulator while not strictly enforcing the asm syntax
//
// Syntax that is slightly changed:
// 1. The '@<i>' symbold from the hack-asm language will need to be repalced with
// 'AT(<i>)' while '<i>' is the value followed after @
// 2.
//
//
// Besides these all the C features are avilable
#ifndef NAND2TETRIS_ASM_C_H
#define NAND2TETRIS_ASM_C_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

    

#define NAND2TETRIS_TOTAL_BASE_MEMORY_SIZE 16384
#define NAND2TETRIS_TOTAL_SCREEN_SIZE 8192
#define NAND2TETRIS_TOTAL_KEYBOARD_SIZE 1
//NAND2TETRIS_TOTAL_MEMORY_SIZE consists of the base RAM,
// the screen buffer and the keyboard buffer
#define NAND2TETRIS_TOTAL_MEMORY_SIZE ( \
    NAND2TETRIS_TOTAL_BASE_MEMORY_SIZE \
    + NAND2TETRIS_TOTAL_SCREEN_SIZE \
    + NAND2TETRIS_TOTAL_KEYBOARD_SIZE)
//---------------start hack ASM symbols------------------
#define R0 NAND2TETRIS_CPU_REGISTERS[0]
#define R1 NAND2TETRIS_CPU_REGISTERS[1]
#define R2 NAND2TETRIS_CPU_REGISTERS[2]
#define R3 NAND2TETRIS_CPU_REGISTERS[3]
#define R4 NAND2TETRIS_CPU_REGISTERS[4]
#define R5 NAND2TETRIS_CPU_REGISTERS[5]
#define R6 NAND2TETRIS_CPU_REGISTERS[6]
#define R7 NAND2TETRIS_CPU_REGISTERS[7]
#define R8 NAND2TETRIS_CPU_REGISTERS[8]
#define R9 NAND2TETRIS_CPU_REGISTERS[9]
#define R10 NAND2TETRIS_CPU_REGISTERS[10]
#define R11 NAND2TETRIS_CPU_REGISTERS[11]
#define R12 NAND2TETRIS_CPU_REGISTERS[12]
#define R13 NAND2TETRIS_CPU_REGISTERS[13]
#define R14 NAND2TETRIS_CPU_REGISTERS[14]
#define R15 NAND2TETRIS_CPU_REGISTERS[15]
#define M NAND2TETRIS_MEMORY_CONTENT
#define A NAND2TETRIS_SELECTED_ADDRESS
#define AT(i)( { /*for .asm replace AT() with @ */\
    int32_t address = NAND2TETRIS_GET_VAR_VAL(i); \
    if (address < 0) {\
        fprintf(stderr, "ERROR: Address is negative: %u(%s)\n", address, #i); \
        exit(1);\
    } else if (address >= NAND2TETRIS_TOTAL_MEMORY_SIZE) {\
        fprintf(stderr, "ERROR: Address is too large: %u (%s)\n", address, #i); \
        exit(1);\
    }\
    M = &NAND2TETRIS_MEMORY_SPACE[address];\
    A = address; \
})
// Starting address of the screen
#define SCREEN (NAND2TETRIS_TOTAL_BASE_MEMORY_SIZE)
// Starting address of the keybaord
#define KBD (NAND2TETRIS_TOTAL_BASE_MEMORY_SIZE + NAND2TETRIS_TOTAL_SCREEN_SIZE)
//--------------end hack ASM symbols-----------------

int32_t NAND2TETRIS_SELECTED_ADDRESS;
int16_t *NAND2TETRIS_MEMORY_CONTENT;
int16_t NAND2TETRIS_MEMORY_SPACE[NAND2TETRIS_TOTAL_MEMORY_SIZE];
int16_t *NAND2TETRIS_CPUREGISTERS = NAND2TETRIS_MEMORY_SPACE;


typedef stru1ct S_NAND2TETRIS_VARS {
    char *name;
    int val;
} T_NAND2TETRIS_VARS;

T_NAND2TETRIS_VARS VARS[30000] = {0};

#define ADD_VAR(name) ( {\
\
)}


#endif
