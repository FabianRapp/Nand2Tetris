#include "../assembler.h"
#include "simple_assembler.h"
/*
	* simple solution as base line efficiency for high throughput version
	- singl threaded
	- dual pass: first pass store symoles, second translate to binary
	- will crash on huge files duo to no malloc protection
*/
//===================basic ll==================
//=================================================================


void	extend_bin(t_bin_list	*arr_list)
{
	for (int i = 0; i < arr_list->arr_count; i++)
	{
	}
}

void	resolve_symboles(t_assembler	*state)
{

}

// assumes the list contain the fully resolved binary machine code
// also cleans the list
void	output(t_bin_list	*bin_list)
{
	const char	*out_file = "test.bin";
	int	fd = open(out_file, O_WRONLY);
	t_list_node	*cur = bin_list->list;
	bin_list->list = NULL;
	t_list_node	*last = NULL;
	for (int i = 0; i < bin_list->arr_count - 1; i++)
	{
		write(fd, cur->val, BIN_ARR_SIZE);
		free(cur->val);
		last = cur;
		cur = cur->next;
		free(last);
	}
	write(fd, cur->val, bin_list->instruction_count % BIN_ARR_SIZE);
	free(cur->val);
	free(cur);
}

size_t	remove_spaces(volatile char *buffer, int read_bytes)
{
	size_t	byte_count = 0;
	static bool	empty_line = true;
	static bool	is_comment = false;
	assume(read_bytes > 0 && read_bytes <= READ_CHUNK_SIZE);
	for (int i = 0; i < read_bytes; i++)
	{
		if (buffer[i] == '/' || is_comment)
			is_comment = true;
		// the provided assembly code is littered with unprintable ascii values
		// the tools seem to ignore it the same way they ignore spaces
		// space does not need to be checked since it's ascii is 32 but
		// I will leave it for clearness
		// for somereason tabs are interpreted as normal ascii and new lines are
		// the instruction or comment terminations
		while (i < read_bytes && buffer[i] != '\n' && (is_comment ||
			(buffer[i] != '\t'
			&& (buffer[i] == ' ' | buffer[i] < 33 || buffer[i] > 126))))
		{
			i++;
		}
		if (i == read_bytes)
			return (byte_count);
		if (buffer[i] == '\n' && !empty_line)
		{
			buffer[byte_count++] = 0;
			is_comment = false;
			empty_line = true;
		}
		else if(buffer[i] == '\n')
			is_comment = false;
		else
		{
			buffer[byte_count++] = buffer[i];
			empty_line = false;
		}
	}
	return (byte_count);
}

void	add_symbole(t_symbole_table *table, char *name, uint16_t val,
			 bool is_declartion, int instruction_index)
{
	assume(name && *name);
	t_symbole	symbole = {.val = val, .name = strdup(name), .references = NULL};
	size_t		index = hash_str(name);
	t_list_node	*cur_cmp = table->map + index;
	//map index is empty, enter new symbole here
	if (!((t_symbole *)(cur_cmp->val))->name)
	{
		// do stuff
		return ;
	}
	//symbol is allready in map
	if (!strcmp(((t_symbole *)(cur_cmp->val))->name, name))
	{
		//do stuff
		return ;
	}
	t_list_node	*last = NULL;
	//collision handeling
	while (cur_cmp)
	{
		//symbol is allready in map
		if (!strcmp(((t_symbole *)(cur_cmp->val))->name, name))
		{
			//do stuff
			return ;
		}
		cur_cmp = cur_cmp->next;
	}
	//symbol is allready in map
	if (!strcmp(((t_symbole *)(cur_cmp->val))->name, name))
	{
		//do stuff
		return ;
	}
	//finally: there was a collision and symbole was not yet in map
}


typedef struct s_instruction_token
{
	char	*str_data;
	bool	is_c_instruction;//->false indicates a-instruction
	bool	is_a_declaration;//only matters if !is_c_instruction
}	t_instruction_token;

typedef struct s_basic_lexer
{
	char	*rest;
	int		pos;
	int		read_pos;
}	t_basic_lexer;

// note: this returns false for 'J' since it is not needed to parse
// the instruction
// besides this this returns true for every valid char is the hack-asm
// language
bool	valid_char(char c)
{
	if (c != 'D' && c != 'M' && c != 'A' && c != '0' && c != '1' && c != '-'
		&& c != '&' && c != '|' && c != '!' && c != ';' && c != '\n'
		&& c != '=' && c != '+' && c != 'G' && c != 'T' && c != 'E'
		&& c != 'Q' && c != 'L' && c != 'N' && c != '(' && c != '('
		&& c != '@' && c != 'P')
	{
		return (false);
	}
	return (true);
}

//very much needed for the 'less complex' assembler
void	instruction_lexer(char *buffer)
{
	static t_basic_lexer	lexer = {0};
	
}

void	store_symboles(t_symbole_table *table, char *file)
{
	int fd = open(file, O_RDONLY, O_NONBLOCK);
	const size_t buffer_size = 1 << 13;
	alignas(1<<13) char	buffer[buffer_size + 1];
	assume(fd > 0);
	int	read_return = 1;
	char	*rest = NULL;
	uint16_t	line_nb = 0;
	uint32_t	next_symbol_val = 0;//change this later
	while (read_return > 0)
	{
		
		read_return = read(fd, buffer, buffer_size);
		assert(read_return >= 0);
		remove_spaces(buffer, read_return);
		buffer[read_return]  = 0;
		int start = 0;
		if (rest && (*rest == '(' || *rest == '@'))
		{
			size_t	rest_len = strlen(rest);
			size_t	new_len = strlen(buffer);
			char	*instruction = malloc(rest_len + new_len + 1);
			memcpy(instruction, rest, rest_len);
			memcpy(instruction + rest_len, buffer, new_len + 1);
			bool	is_goto_dest = (*instruction == '(');
			if (is_goto_dest)
			{
				instruction[rest_len + new_len] = 0;
				add_symbole(table, instruction + 1, line_nb,  true, -1);
			}
			else
			{
				add_symbole(table, instruction, next_symbol_val, false, line_nb);
				line_nb++;
			}
		}
		else
			line_nb++;
		free(rest);
		rest = NULL;
		for (int i = start; i < read_return; i++)
		{
			//repeat the above until the end of th read buffer
		}
	}
	if (read_return < 0)
	{
		fprintf(stderr, "file io error\n");
		exit(errno);
	}
	close(fd);
}

int main(int ac, char *av[])
{
	char *path = "test.asm";
	if (ac > 1)
		path = av[1];
	t_assembler		state = {0};
	init(&state);
	store_symboles();

}
