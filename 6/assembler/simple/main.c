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

int out_fd = 1;


void	extend_bin(t_bin_list	*arr_list)
{
	for (size_t i = 0; i < arr_list->arr_count; i++)
	{
	}
}

void	resolve_symboles(t_assembler *state)
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
	for (size_t i = 0; i < bin_list->arr_count - 1; i++)
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
		&& c != '@' && c != 'P' && c != '_' && c != 'J' && !isascii(c))
	{
		return (false);
	}
	return (true);
}

size_t	filter_charset(volatile char *buffer, int read_bytes)
{
	size_t	byte_count = 0;
	static bool	empty_line = true;
	static bool	is_comment = false;
	assume(read_bytes > 0 && read_bytes <= READ_CHUNK_SIZE);
	for (int i = 0; i < read_bytes; i++)
	{
		if (buffer[i] == '/' || is_comment)
			is_comment = true;
		while (i < read_bytes && buffer[i] != '\n' && (is_comment ||
			(buffer[i] != '\t'
			&& (buffer[i] == ' ' | buffer[i] < 33 || buffer[i] > 126))))
		{
			i++;
		}
		if (i == read_bytes)
			return (byte_count);
		if (!valid_char(buffer[i]))
			continue ;
		if (buffer[i] == '\n' && !empty_line)
		{
			buffer[byte_count++] = 0;
			//buffer[byte_count++] = '\n';
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
		filter_charset(buffer, read_return);
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

void	read_char(t_basic_lexer *lexer)
{
	lexer->cur = lexer->buffer[lexer->read_pos];
	lexer->pos = lexer->read_pos;
	lexer->read_pos += 1;
}

uint8_t	translate_jmp(char *s)
{
	if (s[1] == 'G')
	{
		if (s[2] == 'T')
			return (0x1);
		else
			return (0x3);
	}
	else if (s[1] == 'E')
		return (0x2);
	else if (s[1] == 'L')
	{
		if (s[2] == 'T')
			return (0x4);
		else
			return (0x6);
	}
	else if (s[1] == 'N')
		return (0x5);
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
		return (0x74);
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

t_c_instr	c_instr_finite_state_machine(char *s, int *len)
{
	t_c_instr instruction = {.is_c_inst = 1, .padding = 3, .comp=0, .dest=0, .jmp=0};
	if (s[1] == 'J')
	{
		*len = 4;
		instruction.jmp = translate_jmp(s + 1);
		return (instruction);
	}
	else if (s[1] == '=' || s[1] == 'M' || s[1] == 'D')
	{
		if (s[1] == '=')
		{
			*len = 2;
			instruction.dest = translate_short_dest(s[0]);
		}
		else if (s[2] == '=')
		{
			*len = 3;
			instruction.dest = translate_med_dest(s);
		}
		else
		{
			*len = 4;
			instruction.dest = 0x7;
		}
	s += (*len);
	}
	if (s[1] == '\0' || s[1] == ';')
	{
		instruction.comp = translate_short_comp(s[0]);
		if (s[1] == ';')
		{
			*len += 2;
			if (!s[2])
				return (instruction);
			s++;
		}
		else
		{
			*len += 1;
			return (instruction);
		}
	}
	else if (!s[2] || s[2] == ';')
	{
		instruction.comp = translate_med_comp(s);
		if (s[2] == ';')
		{
			*len += 3;
			if (!s[3])
				return (instruction);
		}
		else
		{
			*len += 2;
			return (instruction);
		}
	}
	else
	{
		assume(!s[3] || s[3] == ';');
		instruction.comp = translate_long_comp(s);
		if (s[3] == ';')
		{
			*len += 4;
			if (!s[4])
				return (instruction);
		}
		else
		{
			*len += 3;
			return (instruction);
		}
	}
	*len += 3;
	instruction.jmp = translate_jmp(s);
	return (instruction);
}

t_a_instr	a_instr_lexer(char *str, size_t *str_len)
{
}

//very much needed for the 'less complex' assembler
t_instruction	instruction_lexer(t_basic_lexer *lexer)
{
	int	len = 0;
	t_instruction	instruction = {0};

	read_char(lexer);
	if (lexer->cur != '@' && lexer->cur != '(')
	{
		instruction.c = c_instr_finite_state_machine(
			lexer->buffer + (lexer->pos), &len);
		printf("c, len %d\n", len);
	}
	else
	{
		len = strlen(lexer->buffer + lexer->pos);
		printf("a, len: %d\n", len);
	}
	lexer->read_pos += len;
	return (instruction);
}

#define READ_LEN (1 << 14)

// algigns the given buffer to read_len while leaving 12 bytes (longest
// possible instruction infront) for negative indexing
char	*align_buffer(char *base)
{
	const int	longest_instruction_len = 12;
	uintptr_t	address = (uintptr_t)base;
	size_t offset = READ_LEN - (address % READ_LEN);
	if (offset < longest_instruction_len)
		offset += READ_LEN;
	char	*aligned_ptr = base + offset;
	assume (((uintptr_t)aligned_ptr) % READ_LEN == 0);
	return (aligned_ptr);
}

void	main_loop(char *file)
{
	int fd = open(file, O_RDONLY, O_NONBLOCK);
	const int	longest_instruction_len = 12;
	char	raw_buffer[2 * READ_LEN + 1 + longest_instruction_len];
	char	*const buffer = align_buffer(raw_buffer);
	int	read_return = READ_LEN;
	t_basic_lexer	lexer = {0};
	lexer.buffer = buffer;
	int	last_termination_index;
	while (read_return == READ_LEN)
	{
		read_return = read(fd, buffer, READ_LEN);
		assert(read_return >= 0);
		buffer[read_return] = 0;
		if (read_return < READ_LEN)
			break ;
		int bytes = filter_charset(buffer, read_return);
		last_termination_index = bytes -1;
		while (last_termination_index >= 0 && buffer[last_termination_index])
		{
			last_termination_index--;
		}
		assume(last_termination_index > 0);
		buffer[bytes] = 0;
		//write(1, buffer, bytes);
		while (lexer.read_pos < last_termination_index)//while lexer.read_pos < last_termination
		{
			printf("%s\n", lexer.buffer + lexer.read_pos);
			t_instruction instruction = instruction_lexer(&lexer);
			print_instruction(0, instruction);
		}
		assume(lexer.read_pos - 1 == last_termination_index);
		// copy leftovers infront of the buffer and set a negative lexer index
		int	left_over_count = bytes - (last_termination_index + 1);
		assume(left_over_count <= longest_instruction_len);
		printf("left_over_count: %d\n", left_over_count);
		strncpy(buffer - left_over_count, buffer + lexer.read_pos,
			left_over_count);
		lexer.read_pos = -1 * left_over_count;
	}
	exit(0);
printf("end\n");
printf("%s\n", lexer.buffer + lexer.read_pos);
printf("strlen: %lu\n", strlen(lexer.buffer + lexer.read_pos));
	read_return = filter_charset(buffer, read_return);
	while (lexer.read_pos < read_return)
	{
		printf("%s\n", lexer.buffer + lexer.read_pos);
		t_instruction instruction = instruction_lexer(&lexer);
		print_instruction(0, instruction);
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
	main_loop(path);
	//store_symboles();
	return (0);
}
