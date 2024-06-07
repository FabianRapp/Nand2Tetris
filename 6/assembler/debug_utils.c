#include "assembler.h"

void	print_instruction(char *str, t_instruction inst)
{
	uint16_t	val = inst.full;
	uint16_t	mask = 0x8000;
	if (str)
		printf("%s: ", str);
	for (int i = 15; i >= 0; i--)
	{
		printf("%u", (val & mask) >> i);
		mask >>= 1;
	}
	printf("\n");
}
