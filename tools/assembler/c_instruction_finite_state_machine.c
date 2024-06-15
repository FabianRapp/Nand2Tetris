#include "assembler.h"
#include "simple_assembler.h"

uint8_t	translate_jmp(char *s)
{
	//write(1, s, strlen(s));
	if (*s == ';')
		s++;
	if (s[1] == 'N')
		return (0x5);//JNE
	else if (s[1] == 'E')
		return (0x2);//JEQ
	else if (s[1] == 'G')
	{
		if (s[2] == 'T')
			return (0x1);//JGT
		else
			return (0x3);//JGE
	}
	else if (s[1] == 'L')
	{
		if (s[2] == 'T')
			return (0x4);//JLT
		else
			return (0x6);//JLE
	}
	else
		return (0x7);
}

uint8_t	translate_med_dest(char *s)
{
	if (s[0] == 'M')
		return (0x3);
	else if (s[1] == 'M')
		return (0x5);
	assume(s[1] == 'D');
	return (0x6);
}

uint8_t	translate_short_dest(char c)
{
	if (c == 'M')
		return (0x1);
	else if (c == 'D')
		return (0x2);
	assume(c == 'A');
	return (0x4);
}

uint8_t	translate_short_comp(char c)
{
	if (c == '0')
		return (0x2a);
	else if (c == '1')
		return (0x3f);
	else if (c == 'D')
		return (0xc);
	else if (c == 'A')
		return (0x30);
	write(1, "\n", 1);
	write(1, &c, 1);
	write(1, "\n", 1);
	assume(c == 'M');
	return (0x70);
}

uint8_t	translate_med_comp(char *s)
{
	if (s[0] == '-')
	{
		if (s[1] == '1')
			return (0x3a);
		else if (s[1] == 'D')
			return (0xf);
		else if (s[1] == 'A')
			return (0x33);
		assume(s[1] == 'M');
		return (0x73);
	}
	assume (s[0] == '!');
	if (s[1] == 'D')
		return (0xd);
	else if (s[1] == 'A')
		return (0x31);
	assume(s[1] == 'M');
	return (0x71);
}

uint8_t	translate_long_comp(char *s)
{
	if (s[1] == '+')
	{
		if (s[0] == 'D')
		{
			if (s[2] == '1')
				return (0x1f);
			else if (s[2] == 'A')
				return (0x2);
			assume(s[2] == 'M');
			return (0x42);
		}
		if (s[0] == 'A')
		{
			assume(s[2] == '1');
			return (0x37);
		}
		assume(s[0] == 'M' && s[2] == '1');
		return (0x77);
	}
	else if (s[1] == '-')
	{
		if (s[0] == 'D')
		{
			if (s[2] == '1')
				return (0xe);
			else if (s[2] == 'A')
				return (0x13);
			assume(s[2] == 'M');
			return (0x53);
		}
		if (s[0] == 'A')
		{
			if (s[2] == '1')
				return (0x32);
			assume(s[2] == 'D');
			return (0x7);
		}
		assume(s[0] == 'M');
		if (s[2] == '1')
			return (0x72);
		assume(s[2] == 'D');
		return (0x47);
	}
	else if (s[1] == '&')
	{
		if (s[2] == 'A')
			return (0x0);
		assume(s[2] == 'M');
		return (0x40);
	}
	assume(s[1] == '|');
	if (s[2] == 'A')
		return (0x15);
	assume(s[2] == 'M');
	return (0x55);
}

t_c_instr	c_instr_finite_state_machine(char *s)
{
	t_c_instr instruction = {.is_c_inst = 1, .padding = 3, .comp=0, .dest=0, .jmp=0};
	if (s[1] == 'J')
	{
		instruction.jmp = translate_jmp(s + 1);
		return (instruction);
	}
	else if (s[1] == '=' || s[1] == 'M' || s[1] == 'D')
	{
		if (s[1] == '=')
		{
			instruction.dest = translate_short_dest(s[0]);
			s += 2;
		}
		else if (s[2] == '=')
		{
			instruction.dest = translate_med_dest(s);
			s += 3;
		}
		else
		{
			instruction.dest = 0x7;
			s += 4;
		}
	}
	if (s[1] == '\0' || s[1] == ';')
	{
		instruction.comp = translate_short_comp(s[0]);
		if (s[1] == ';')
		{
			if (!s[2])
				return (instruction);
			s++;
		}
		else
			return (instruction);
	}
	else if (!s[2] || s[2] == ';')
	{
		instruction.comp = translate_med_comp(s);
		if (s[2] == ';')
		{
			if (!s[3])
				return (instruction);
		}
		else
			return (instruction);
	}
	else
	{
		assume(!s[3] || s[3] == ';');
		instruction.comp = translate_long_comp(s);
		if (s[3] == ';')
		{
			if (!s[4])
				return (instruction);
		}
		else
			return (instruction);
	}
	instruction.jmp = translate_jmp(s);
	char c = instruction.jmp + '0';
	//write(1, &c, 1);
	//write(1, "\n", 1);
	return (instruction);
}

