#include "../assembler.h"

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

static	__attribute__((always_inline)) bool	is_charset(char c)
{
	switch(c)
	{
		case '0': return (true);
		case '1': return (true);
		case '-': return (true);
		case 'D': return (true);
		case 'A': return (true);
		case '!': return (true);
		case '+': return (true);
		case '&': return (true);
		case '|': return (true);
		case 'M': return (true);
		case '=': return (true);
		case ';': return (true);
		case 'J': return (true);
		case 'G': return (true);
		case 'T': return (true);
		case 'E': return (true);
		case 'Q': return (true);
		case 'L': return (true);
		case 'N': return (true);
		case 'P': return (true);
		default:
			assume(0);
			return (false);
	}
}

//many optimizations possible
static	__attribute__((always_inline)) size_t	assume_strlen(char *s)
{
	switch (s[0])
	{
		case ';': return 4;
		case '\0': assume(0);
		default:
		assume(is_charset(s[1]));
		switch(s[1])
		{
			case '\0': return 1;
			case ';': return 5;
			default:
			switch(s[2])
			{
				case '\0': return 2;
				case ';': return 6;
				default: assume(0);
			}
		}
	}


	size_t	len = 0;

	while (s[len])
	{
		assume(is_charset(s[len]));
		len++;
	}
	assume(len >= 1 && len <= 11);
	return (len);
}

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
static __attribute__((always_inline)) void	no_c(char *s, char c)
{
	for (int i = 0; s[i]; i++)
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
	if (j[4])
	{
		return (false);
	}
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

// pointer points to start of copute part
static __attribute__((always_inline)) bool	is_comp(char *s)
{
	if (
		*s == '0' || *s == '1' || *s == '-' || *s == '!' ||
		s[1] == ';' || s[1] == '1' || s[1] == '+' || s[1] == '&' || s[1] == '|'
		|| !s[1] || s[1] == '-'
	)
	{
		assume(s[0] != 'J');
		assume(s[0] != 'G');
		assume(s[0] != 'T');
		assume(s[0] != 'E');
		assume(s[0] != 'Q');
		assume(s[0] != 'L');
		assume(s[0] != 'N');
		assume(s[0] != 'P');
		assume(s[0] != ';');
		no_c(s, '=');
		return (true);
	}
	printf("not comp: %s\n", s);
	return (false);
}

static __attribute__((always_inline)) void	base(char *s, int len)
{
	assume(len > 0 && len <= 11);
	assume(s[len] == 0);
}

static __attribute__((always_inline)) void	small_len(char *s, int len)
{
	if (len < 5)//no possible jump
	{
		no_c(s, 'J');
		no_c(s, 'G');
		no_c(s, 'T');
		no_c(s, 'E');
		no_c(s, 'Q');
		no_c(s, 'L');
		no_c(s, 'N');
		no_c(s, 'P');
	}
	if (*s != 'M' && *s != 'D' && *s != 'A')
		no_c(s, '=');
	if (len == 1)
	{
		no_c(s, ';');
		char	c = *s;
		assume(c=='0' || c=='1' || c=='D' || c=='A' || c=='M');
	}
	if (len == 1 || len == 2)
	{
		no_c(s, '=');
	}
}

static __attribute__((always_inline)) void	eq_len(char *s, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (s[i] == '=')
		{
			assume(len>=3);
			return ;
		}
	}
	assume(len<=7);
}

//s has to be the start of the compute part
//does not give the compiler info atm
size_t	comp_val(char *s)
{
	switch (s[0])
	{
		case '0': return (0x2A);
		case '1': return (0x3F);
		case '!':
		{
			switch (s[1])
			{
				case 'D': return (0xD);
				case 'A': return (0x31);
				case 'M': return (0x71);
				default: assume(0);
			}
		}
		case '-':
		{
			switch (s[1])
			{
				case '1': return (0x3a);
				case 'D': return (0xf);
				case 'A': return (0x33);
				case 'M': return (0x73);
				default: assume(0);
			}
		}
		case 'D':
		{
			switch(s[1])
			{
				case '\0': return (0xc);
				case ';': return (0xc);
				case '+':
				{
					switch (s[2])
					{
						case '1': return (0x1f);
						case 'A': return (0x2);
						case 'M': return (0x42);
						default: assume(0);
					}
				}
				case '-':
				{
					switch (s[2])
					{
						case '1': return (0xe);
						case 'A': return (0x13);
						case 'M': return (0x53);
						default: assume(0);
					}
				}
				case '&':
				{
					switch (s[2])
					{
						case 'A': return (0x0);
						case 'M': return (0x40);
						default: assume(0);
					}
				}
				case '|':
				{
					switch (s[2])
					{
						case 'A': return (0x15);
						case 'M': return (0x55);
						default: assume(0);
					}
				}
			}
		}
		case 'A':
		{
			switch (s[1])
			{
				case '-':
				{
					switch (s[2])
					{
						case '1': return (0x32);
						case 'D': return (0x7);
						default: assume(0);
					}
				}
				case '+': return (0x37);
				case '\0': return (0x30);
				case ';': return (0x30);
				default: assume(0);
			}
		}
		case 'M':
		{
			switch (s[1])
			{
				case '\0': return (0x70);
				case ';': return (0x70);
				case '+': return (0x77);
				case '-':
				{
					switch (s[2])
					{
						case '1': return (0x72);
						case 'D': return (0x47);
						default: assume(0);
					}
				}
				default: assume (0);
			}
		}
		default: assume(0);
	}
	assume(0);
	return (0);
}

// should heavily rely on compile time
// should allow the compile to optimize allot on compile time
// basically a super complex lookup table to reduce runtime checks
// todo:
// assume a max count of the chars early on
static inline t_instruction	fill_c_instruction(char *s)
{
	assume(s && *s);
	t_instruction	inst = {.c={.is_c_inst=1, .padding=3, .comp=0,.dest=0,.jmp=0}};
	size_t len = assume_strlen(s);
	base(s, len);
	small_len(s, len);
	eq_len(s, len);
	size_t	comp_start = 0;
	//explains what valid semicolns and what valid jumps are
	for (int i = 0; i<=len; i++)
	{
		if (i < len)
			assume(s[i]);
		if (s[i] == '=')
			comp_start = i + 1;
		if (s[i] == ';' && len > i + 1)
		{
			c_only_pos(s, ';', i, len);
			c_only_pos(s, 'J', i+1, len);
			assume(i < len - 3);//there has to be space left for the jmp symbole
			assume(i > 0);
			assume(valid_jmp(s + i));
			assume (len - i == 4);
			if (has_dest(s, len))
			{
				//todo: assume the '=' position and reduce the len
				assume(len >= 7);
			}
			//=========fill JMP============
			// todo: way more assumtions tbd here
			if (s[i+2] == 'E')//JEQ
			{
				inst.c.jmp = 0b010;
				c_only_pos(s, 'Q', i+3, len);
				no_c(s, 'G');
				no_c(s, 'T');
				no_c(s, 'L');
				no_c(s, 'N');
				no_c(s, 'P');
			}
			else if (s[i+3] == 'P')//JMP
			{
				inst.c.jmp = 0b111;
				assume(s[i+2] == 'M');
				c_only_pos(s, 'P', i+3, len);
			}
			else if (s[i+2] == 'G')
			{
				no_c(s, 'P');
				if (s[i+3] == 'E')//JGE
				{
					inst.c.jmp = 0b011;
				}
				else//JGT
				{
					inst.c.jmp = 0b001;
					assume(s[i+3] == 'T');
				}
			}
			else if (s[i+2] == 'L')
			{
				no_c(s, 'P');
				if (s[i+3] == 'T')//JLT
				{
					inst.c.jmp = 0b100;
				}
				else//JJLE
				{
					inst.c.jmp = 0b110;
					assume(s[i+3] == 'E');
				}
			}
			else//JNE
			{
				inst.c.jmp = 0b101;
				no_c(s, 'P');
			}
			//=============================

		}
		// else if there is no jump
		else if (s[i] == ';' || (!s[i] && s[i-1] != ';' && (len < 4 || s[i-4] != ';')))
		{
			if (!s[i])
			{
				no_c(s, ';');
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
			no_c(s, 'J');
			no_c(s, 'G');
			no_c(s, 'T');
			no_c(s, 'E');
			no_c(s, 'Q');
			no_c(s, 'L');
			no_c(s, 'N');
			no_c(s, 'P');
			assume(i >= 1);
			inst.c.jmp = 0;
		}
	}
	inst.c.comp = comp_val(s + comp_start);
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
		else if (s[3] == '=')//dest is 'ADM='
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
		no_c(s, '=');
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

static t_instruction	handle_c_instruction(char *str_data)
{
	
	t_instruction	inst = fill_c_instruction(str_data);
	int				i = 0;
	return (inst);
}

static t_instruction	handle_a_instruction(char *str_data)
{
	t_instruction	inst = {0};
	return (inst);
}

void	handle_instruction(char *single_instruction_str_data)
{
	if (*single_instruction_str_data == '@' || *single_instruction_str_data == '(')
		handle_a_instruction(single_instruction_str_data);
	else
		handle_c_instruction(single_instruction_str_data);
}

t_instruction	test_handle_instruction(char *single_instruction_str_data)
{
	if (*single_instruction_str_data == '@' || *single_instruction_str_data == '(')
		return(handle_a_instruction(single_instruction_str_data));
	else
		return (handle_c_instruction(single_instruction_str_data));
}

