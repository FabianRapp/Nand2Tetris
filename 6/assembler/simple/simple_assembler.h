#ifndef SIMPLE_ASSEMBLER_H
# define SIMPLE_ASSEMBLER_H

#include "../assembler.h"
#include <ctype.h>

typedef struct s_list_node
{
	struct s_list_node	*next;
	void				*val;
}	t_list_node;


//====================symbole table implementation as basic hash map====
typedef struct s_symbole	t_symbole;
typedef struct s_symbole
{
	uint16_t		val;
	char			*name;
	t_list_node		*references;
}	t_symbole;

typedef struct s_symbole_table
{
	t_list_node	map[TABLE_SIZE];
}	t_symbole_table;

//========================output bin data data structure as an array list===
#define BIN_ARR_SIZE (16 * 1000)
typedef struct s_bin_list
{
	t_list_node		*list;
	size_t			instruction_count;
	size_t			arr_count;
}	t_bin_list;

typedef union u_bin_data
{
	unsigned char	bytes[BIN_ARR_SIZE];
	t_instruction	instructions[BIN_ARR_SIZE / 2];
}	__attribute__((packed))	t_bin_data;

//====================================================================
typedef struct s_assembler
{
	t_symbole_table	symboles;

}	t_assembler;



void	append_node(t_list_node	**list, void *new_data);
void	init_bin(t_bin_list *arr_list);
void	init_symbole_table(t_symbole_table *table);
void	init(t_assembler	*state);

size_t	hash_str(char *str);




#endif
