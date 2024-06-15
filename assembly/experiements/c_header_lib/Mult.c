#include <stdio.h>
#include <stdint.h>

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define R8 8
#define R9 9
#define R10 10
#define R11 11
#define R12 12
#define R13 13
#define R14 14
#define R15 15

int16_t RAM[30000];
int16_t D;
int16_t *M;
int16_t A;

#define AT(n) \
    A = n; \
    M = RAM + n; \

int16_t expect;
#include <stdlib.h>
void test(int ac, char *av[])
{
    if (ac != 3)
    {
        printf("wrong ac!\n");
        exit(1);
    }
    int a = atoi(av[1]);
    int b = atoi(av[2]);
    printf("testing %d * %d\n", a, b);
    RAM[0] = a;
    RAM[1] = b;
    expect = a * b;
}
#define i R3


int main(int ac, char *av[])
{
    test(ac, av);
    
    /*
        Notes:
        Sets R2 to 0 and takes adds R0 R1-times to R2


        potential improvement in decion over which nb to loop
    */

    goto START;

//function to make M it's absolute value
ABS://makes the currently active address positive
    //goes to R10 afterwards
    //uses R15
#define INPUT_ADDRESS R15
#define RETURN_LOCATION R10
    D = A;//save the input memory location
    
    AT(INPUT_ADDRESS)
    *M = D;

    AT(D)//Load the mory location
    D = *M;
    AT(RETURN_LOCATION)//return if value is postive
    if (D > 0)
    {
        goto START;
    }
    D = ~D;//make D positive
    AT(INPUT_ADDRESS)
    D = A;//save the old address
    AT(RETURN_LOCATION);
    goto START;
// END ABS

START:
    AT(2)
    (*M) = 0;//set result to 0
    AT(R0) //check if R0 is 0 for early return
    if (*M == 0)
        goto end;
    AT(R1)//check if R1 is 0 for early return
    if (*M == 0)
        goto end;
    D = *M; // set i to R1 as a counter for the iterations
    AT(i)
    *M = D;
loop:
    AT(R0)//Load R0 to add it to the result in R2
    D = *M;
    AT(R2)//Load the result in R2
    *M = *M + D;//Add R0 to the result in R2
    AT(i)
    if (--(*M) != 0)//if iteration count was reached
        goto loop;

end:
    printf("result: %d (expected: %d)\n", RAM[2], expect);
    //end2:
    //goto end2;
}
