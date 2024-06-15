#ifndef SIMPLE_ASSEMBLER_H
# define SIMPLE_ASSEMBLER_H

#include "assembler.h"
#include <ctype.h>

#define READ_LEN (1 << 14)

#define R0 0

typedef struct s_instruction_token
{
	char	*str_data;
	bool	is_c_instruction;//->false indicates a-instruction
	bool	is_a_declaration;//only matters if !is_c_instruction
}	t_instruction_token;

typedef struct s_basic_lexer
{
	char	*buffer;
	int		pos;
	int		read_pos;
	char	cur;
}	t_basic_lexer;

typedef struct s_list_node
{
	struct s_list_node	*next;
	void				*val;
}	t_list_node;

//====================symbole table implementation as basic hash map====
typedef struct s_symbole	t_symbole;
typedef struct s_symbole
{
	int32_t			val;
	char			*name;
}	t_symbole;

typedef struct s_symbole_table
{
	t_list_node	map[TABLE_SIZE];
}	t_symbole_table;

//====================================================================

t_c_instr	c_instr_finite_state_machine(char *s);

void	append_node(t_list_node	**list, void *new_data);

size_t	hash_str(char *str);
int		add_symbole(t_symbole_table *table, char *name, int val, bool is_declartion);

size_t	filter_charset(volatile char *buffer, int read_bytes);
char	*align_buffer(char *base);

#endif
