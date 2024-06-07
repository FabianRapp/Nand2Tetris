#include "assembler.h"

// ALL UNTESTED AND UNFINISHED, JUST PROTOTYPES

static inline t_instruction	fill_c_instruction(char *str_data)
{
	t_instruction	inst = {0};

	inst.c.is_c_inst = 1;
	return (inst);
}

static void	handle_c_instruction(char *str_data)
{
	t_instruction	inst = fill_c_instruction(str_data);

}

static void	handle_a_instruction(char *str_data)
{
}

void	handle_instruction(char *single_instruction_str_data)
{
	if (*single_instruction_str_data == '@' || *single_instruction_str_data == '(')
		handle_a_instruction(single_instruction_str_data);
	else
		handle_c_instruction(single_instruction_str_data);
}

