#include "assembler.h"

//In this file I want to experiment with the usefull ness of feeding the
//compiler an exessive amount of information
//I want to learn how the compiler handels very verbouse value comparisions
//of on compile time unknown values

/*
All possiblities for the c instruction at this points (cleanned inputs):

Destination part, optional,
exists only if the '=' is at the 2nd/3rd/4th index depending of the dest.
if a destination part exists it is always followed by a comput part.
if a destination part exists it is at the very begining of the string.
symbols:
M=
D=
DM=
A=
AM=
AD=
ADM=

Compute part, not fully optional: if no destination exists but a jump exists
the string can start with a semicolon(';') followed by a jump part, otherwise
there always has to be a compute part.
either followed by a semicolon (';') or the string null(0)-termionation
either at the very beginning of the string or after a '='.
symbols:
0
1
-1
D
A
!D
!A
-D
-A
D+1
A+1
D-1
A-1
D+A
D-A
A-D
D&A
D|A
M
!M
-M
M+1
M-1
D+M
D-M
M-D
D&M
D|M

Semicolon, optonal
can exists after the compute part, or has to exits if no compute part(and thus no
desination part).
either followed by the string null(0)-termionation or a jump part.
symbol:
;

jump part, optional.
can only exists after a semicolon.
after the jump part there is always the null termination.
symbols:
JGT
JEQ
JGE
JLT
JNE
JLE
JMP

All in all syntax:
(destination part)(compute part)(semicolon)(jump part)

*/

// this assumes that the given char is only on the exact index given
static __attribute__((always_inline)) void	c_only_pos(char *s, char c,
	size_t index, size_t len)
{
	assume(index >= 0 && index <= len);
	for (int i = 0; i < len; i++)
	{
		if (i != index)
			assume(s[i] != c);
		else
			assume(s[i] == c);
	}
}

// this assumes that the given char is not in the string
static __attribute__((always_inline)) void	no_c(char *s, char c,
	size_t len)
{
	for (int i = 0; i < len; i++)
	{
		assume(s[i] != c);
	}
}

static inline bool	valid_jmp(char *j)
{
	if (*j != ';')
		return false;
	if (*(j+1) != 'J')
		return false;
	char c = *(j+2);
	if (c != 'G' && c != 'E' && c != 'G' && c != 'L' && c != 'N'
		&& c != 'M')
	{
		return false;
	}
	char n = *(j+3);
	if (
		(c == 'G' && !(n == 'T' || n == 'E'))
		|| (c == 'E' && n != 'Q')
		|| (c == 'L' && !(n == 'T' || n == 'E'))
		|| (c == 'N' && n != 'E')
		|| (c == 'M' && n != 'P')
	)
	{
		return false;
	}
	if (*j+4)
		return (false);
	return (true);
}

static inline bool	has_dest(char *s, size_t len)
{
	if (len < 3)
		return false;
	if (*s != 'A' && *s != 'D' && *s != 'M')
		return (false);
	if (*(s + 1) == '=' || *(s + 2) == '=' || *(s+3) == '=')
		return (true);
	return (false);
}

static inline bool	is_first_comp_char(char c)
{
	if (c == '0' || c  == '1' || c == '-' || c == 'A' || c == 'D'
		 || c == 'M' || c == '!')
	{
		return (true);
	}
	return (false);
}

// todo finish this
static inline bool	is_comp(char *s)
{
	//if (*s == '0' || *s == '1' || *s
	if (!is_first_comp_char(*s))
		return (false);


	return (true);
}

// should heavily rely on compile time
// should allow the compile to optimize allot on compile time
// basically a super complex lookup table to reduce runtime checks
// todo:
// assume a max count of the chars early on
static inline t_instruction	fill_c_instruction(char *s)
{
	assume(s && *s);
	t_instruction	inst = {.c={.is_c_inst=1, .padding=3, .a_type=0,.comp=0,.dest=0,.jmp=0}};
	size_t len = strnlen(s, 11);

	assume(len > 0 && len <= 11);
	assume(s[len] == 0);
	//instruction can only start with these if its only a signle char
	assume(!(len == 1 && *s != '0' && *s != '1'&& *s != 'D' && *s != 'A' && *s != 'M'));
	//explains what valid semicolns and what valid jumps are
	for (int i = 0; i<=len; i++)
	{
		if (i < len)
			assume(s[i]);
		if (s[i] == ';' && len > i + 1)
		{
			c_only_pos(s, ';', i, len);
			c_only_pos(s, 'J', i+1, len);
			assume(i < len - 3);//there has to be space left for the jmp symbole
			assume(valid_jmp(s + i));
			assume (len - i == 4);
			if (has_dest(s, len))
			{
				//todo: assume the '=' position and reduce the len
				assume(len >= 7);
			}
		}
		// else if there is no jump
		else if (s[i] == ';' || (!s[i] && s[i-1] != ';' && (len < 4 || s[i-4] != ';')))
		{
			if (!s[i])
			{
				no_c(s, ';', len);
				assume(len == i);
				if (has_dest(s, len))
					assume(len>=1 && len <=7);
				else
					assume(len>=1 && len <= 3);
			}
			if (s[i] == ';')
			{
				c_only_pos(s, ';', i, len);
				assume(!s[i + 1]);
				assume(len == i + 1);
				if (has_dest(s, len))
					assume(len>=4 && len <=8);
				else
					assume(len<=4&&len>=2);
			}
			no_c(s, 'J', len);
			no_c(s, 'G', len);
			no_c(s, 'T', len);
			no_c(s, 'E', len);
			no_c(s, 'Q', len);
			no_c(s, 'L', len);
			no_c(s, 'N', len);
			no_c(s, 'P', len);
			assume(i >= 1);
			inst.c.jmp = 0;
		}
	}
	if (has_dest(s, len))
	{
		if (*(s+1) == '=')
		{
			c_only_pos(s, '=', 1, len);
			assume(is_comp(s+2));
			assume(len <= 9 && len >= 3);
		}
		else if (*(s+2) == '=')
		{
			c_only_pos(s, '=', 2, len);
			assume(is_comp(s+3));
			assume(len<=10 && len >= 4);
		}
		else if (*(s+3) == '=')//dest is 'ADM='
		{
			c_only_pos(s, '=', 3, len);
			assume(len>=5);
			assume(is_comp(s+4));
			inst.c.dest = 0b111;
		}
		else
			assume(0);
		if (*s == 'M')//dest is 'M='
		{
			assume(*(s+1) == '=');
			inst.c.dest = 0b001;
		}
		else if (*s == 'D')
		{
			if (*(s+1) == '=')//dest is 'D='
			{
				inst.c.dest = 0b010;
			}
			else//dest is 'DM='
			{
				assume(*(s+1) == 'M');
				assume(*(s+2) == '=');
				inst.c.dest = 0b011;
			}
		}
		else if (*s == 'A')
		{
			if (*(s+1) == '=')//dest is 'A='
			{
				inst.c.dest = 0b100;
			}
			else if (*(s+2) == '=')
			{
				if (*(s+1) == 'M') //dest is 'AM='
				{
					inst.c.dest = 0b101;
				}
				else // dest is 'AD='
				{
					inst.c.dest = 0b110;
					assume(*(s+1) == 'D');
				}
			}
			else
				assume(*(s+3) == '=');// dest is 'ADM='(fill else where)
		}
		else
			assume(0);
	}
	else
	{
		no_c(s, '=', len);
		assume(len > 0 && len <= 7);
		assume((*s == ';' && len == 4 && valid_jmp(s))
		 || (is_first_comp_char(*s)));//i think there can be more here
		inst.c.dest = 0;
	}
	//todo: compute part
	//todo: there are so many potential len checks...
	//todo: assume termination based of len
	return (inst);
}

static void	handle_c_instruction(char *str_data)
{
	
	t_instruction	inst = fill_c_instruction(str_data);
	int				i = 0;
	
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


