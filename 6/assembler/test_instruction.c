#include "assembler.h"
#include <time.h>


char	*get_rdm_dest(int *i)
{
	int	rdm = rand();
	int	dest_count = 8;
	*i = rdm % 8;

	if (!*i)
		return (NULL);
	if (*i == 1)
		return ("M=");
	if (*i == 2)
		return ("D=");
	if (*i == 3)
		return ("DM=");
	if (*i == 4)
		return ("A=");
	if (*i == 5)
		return ("AM=");
	if (*i == 6)
		return ("AD=");
	if (*i == 7)
		return ("ADM=");
	assert(0);
	return (0);
}

char *get_rdm_comp(int *i) {
    int rdm = rand();
    int dest_count = 28;  // Total number of strings
    *i = rdm % dest_count;

    switch (*i) {
        case 0: return "0";
        case 1: return "1";
        case 2: return "-1";
        case 3: return "D";
        case 4: return "A";
        case 5: return "!D";
        case 6: return "!A";
        case 7: return "-D";
        case 8: return "-A";
        case 9: return "D+1";
        case 10: return "A+1";
        case 11: return "D-1";
        case 12: return "A-1";
        case 13: return "D+A";
        case 14: return "D-A";
        case 15: return "A-D";
        case 16: return "D&A";
        case 17: return "D|A";
        case 18: return "M";
        case 19: return "!M";
        case 20: return "-M";
        case 21: return "M+1";
        case 22: return "M-1";
        case 23: return "D+M";
        case 24: return "D-M";
        case 25: return "M-D";
        case 26: return "D&M";
        case 27: return "D|M";
        default:
            assert(0);
            return NULL;
    }
}

char *get_rdm_jmp(int *i)
{
    int rdm = rand();
    int dest_count = 8;  // Total number of jump mnemonics
    *i = rdm % dest_count;

    switch (*i) {
		case 0: return NULL;
        case 1: return "JGT";
        case 2: return "JEQ";
        case 3: return "JGE";
        case 4: return "JLT";
        case 5: return "JNE";
        case 6: return "JLE";
        case 7: return "JMP";
        default:
            assert(0);
            return NULL;
    }
}

typedef struct s_test_c_string
{
	char	*dest;
	char	*comp;
	bool	semicol;
	char	*jmp;
}	t_test_c_string;

typedef struct s_test_instruction_pair
{
	t_instruction	expect;
	t_instruction	result;
	char			*input;
}	t_test_instruction_pair;

// return has to be free <return_val>.input
t_test_instruction_pair	get_rdm_c_instruction_test(void)
{
	t_test_instruction_pair	test = {0};
	t_test_c_string	parts = {0};
	int			dest_index;
	int			comp_index;
	int			jmp_index;
	parts.dest = get_rdm_dest(&dest_index);
	parts.comp = get_rdm_comp(&comp_index);
	parts.jmp = get_rdm_jmp(&jmp_index);
	if (parts.jmp)
		parts.semicol = true;
	else
	{
		parts.semicol = rand() % 2;
	}

	char	buffer[30];
	buffer[0] = 0;
	if (parts.dest)
		strcat(buffer, parts.dest);
	if (parts.comp)
		strcat(buffer, parts.comp);
	if (parts.semicol)
		strcat(buffer, ";");
	if (parts.jmp)
		strcat(buffer, parts.jmp);
	test.input = strdup(buffer);
	test.expect.c.is_c_inst = 1;
	test.expect.c.padding = 3;
	test.expect.c.jmp = jmp_index;
	test.expect.c.dest = dest_index;
	if (comp_index >= 18)
		test.expect.c.a_type = 1;
	else
		test.expect.c.a_type = 0;
	switch (comp_index)
	{
		case 0: test.expect.c.comp = 0b101010;
		case 1: test.expect.c.comp = 0b111111;
		case 2: test.expect.c.comp = 0b111010;
		case 3: test.expect.c.comp = 0b001100;
		case 4: test.expect.c.comp = 0b110000;
		case 5: test.expect.c.comp = 0b001101;
		case 6: test.expect.c.comp = 0b110001;
		case 7: test.expect.c.comp = 0b001111;
		case 8: test.expect.c.comp = 0b110011;
		case 9: test.expect.c.comp = 0b011111;
		case 10: test.expect.c.comp = 0b110111;
		case 11: test.expect.c.comp = 0b001110;
		case 12: test.expect.c.comp = 0b110010;
		case 13: test.expect.c.comp = 0b000010;
		case 14: test.expect.c.comp = 0b010011;
		case 15: test.expect.c.comp = 0b000111;
		case 16: test.expect.c.comp = 0b000000;
		case 17: test.expect.c.comp = 0b010101;
		case 18: test.expect.c.comp = 0b110000;
		case 19: test.expect.c.comp = 0b110001;
		case 20: test.expect.c.comp = 0b110011;
		case 21: test.expect.c.comp = 0b110111;
		case 22: test.expect.c.comp = 0b110010;
		case 23: test.expect.c.comp = 0b000010;
		case 24: test.expect.c.comp = 0b010011;
		case 25: test.expect.c.comp = 0b000111;
		case 26: test.expect.c.comp = 0b000000;
		case 27: test.expect.c.comp = 0b010101;
	}
	return (test);
}


bool	test_c_inst(char *str)
{
	t_test_instruction_pair	test = {0};
	if (str)
	{
		test.input = str;
		printf("%s\n", test.input);

		test.result = test_handle_instruction(test.input);
		if (test.expect.full != test.result.full)
		{
			print_instruction("acutal", test.result);
			printf("--------------------------\n");
		}
	}
	for (int i =0; i < 1000000; i++)
	{
		test = get_rdm_c_instruction_test();
		printf("%s\n", test.input);

		test.result = test_handle_instruction(test.input);
		if (test.expect.full != test.result.full)
		{
			print_instruction("expected", test.expect);
			print_instruction("acutal", test.result);
			printf("--------------------------\n");
		}
		assert(test.expect.c.dest == test.result.c.dest);
		//assert(test.expect.c.comp == test.result.c.comp);
		//assert(test.expect.c.jmp == test.result.c.jmp);
		free(test.input);
	}

	return (true);
}

void	print_instruction_type_sizes(void)
{
	t_instruction	inst;
	printf("t_instrion size: %lu\n", sizeof(t_instruction));
	printf("t_a_instr size: %lu\n", sizeof(t_a_instr));
	printf("t_c_instr size: %lu\n", sizeof(t_c_instr));

	inst.full = 0;
	print_instruction("all 0", inst);

	inst.full = 0x8000;
	print_instruction("set first bit1", inst);

	inst.c.comp = 0x3F;
	print_instruction("set comp all 1", inst);

	inst.a.address = 0b101010101000000;
	print_instruction("inst.a_instr.address=0b101010101000000", inst);


}

int main(int ac, char *av[])
{
	srand(time(NULL));
	if (ac > 1)
		test_c_inst(av[1]);
	else
		test_c_inst(0);
	return (0);
}

