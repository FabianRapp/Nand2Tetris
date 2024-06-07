#include "assembler.h"

void	print_inst(char *str, t_instruction inst)
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


int main(void)
{
	t_instruction	inst;

	printf("t_instrion size: %lu\n", sizeof(t_instruction));
	printf("t_a_instr size: %lu\n", sizeof(t_a_instr));
	printf("t_c_instr size: %lu\n", sizeof(t_c_instr));

	inst.full = 0;
	print_inst("all 0", inst);

	inst.full = 0x8000;
	print_inst("set first bit1", inst);

	inst.c_instr.comp = 0x3F;
	print_inst("set comp all 1", inst);

	inst.a_instr.address = 0b101010101000000;
	print_inst("inst.a_instr.address=0b101010101000000", inst);

	return (0);
}

