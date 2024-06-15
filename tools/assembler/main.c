#include "assembler.h"
#include "simple_assembler.h"
/*
	- dual pass: first pass store symoles, second translate to binary
	- will crash on huge files duo to no malloc protection
*/
//===================basic ll==================
//=================================================================

int out_fd = 1;

void	read_char(t_basic_lexer *lexer)
{
	lexer->cur = lexer->buffer[lexer->read_pos];
	lexer->pos = lexer->read_pos;
	lexer->read_pos += 1;
}


t_a_instr	a_instr_lexer(t_symbole_table *symboles, char *str)
{
	static int	next_new_var_val = 16;
	t_a_instr	instruction = {0};
	str++;
	assume(strlen(str) > 0);
	int	val;
	if (*str >= '0' && *str <= '9')
	{
		val = atoi(str);
	}
	else
	{
		val = add_symbole(symboles, str, next_new_var_val, false);
		if (*str != ' ')
		{
			next_new_var_val++;
		}
	}
	instruction.address = (uint16_t)val;
	return (instruction);
}

//very much needed for the 'less complex' assembler
void	instruction_lexer(char *line, t_symbole_table *symboles)
{
	write(1, line, strlen(line));
	write(1, "\t:", 2);
	t_instruction	instruction = {0};

	if (*line != '@' && *line != '(')
	{
		instruction.c = c_instr_finite_state_machine(line);
	}
	else if (*line == '@')
	{
		instruction.a = a_instr_lexer(symboles, line);
	}
	else
	{
		assume(*line == '(');
		return ;
	}
	uint32_t	val = instruction.full;

	print_instruction("binary data:\t", instruction, 1);
#ifdef BINARY_ASSEMBLY
	# if BYTE_ORDER == LITTLE_ENDIAN
	instruction.full = (instruction.full >> 8) | (instruction.full << 8);
# endif
	write(out_fd, &instruction, sizeof(instruction));
#else
	print_instruction(0, instruction, out_fd);
#endif
}

void	loop(char *file, void (*line_handler)(char *, t_symbole_table *), t_symbole_table *symboles)
{
	int fd = open(file, O_RDONLY, O_NONBLOCK);
	const int	longest_instruction_len = 12;
	char	raw_buffer[2 * READ_LEN + 1 + longest_instruction_len];
	char	*const buffer = align_buffer(raw_buffer);
	int	read_return = READ_LEN;
	t_basic_lexer	lexer = {0};
	lexer.buffer = buffer;
	int	last_termination_index;
	int	left_over_count = 0;
	while (read_return > 0)
	{
		read_return = read(fd, buffer, READ_LEN);
		assert(read_return >= 0);
		buffer[read_return] = 0;
		if (read_return <= 0)
			break ;
		int bytes = filter_charset(buffer, read_return);
		last_termination_index = bytes -1;
		while (last_termination_index >= 0 && buffer[last_termination_index])
		{
			last_termination_index--;
		}
		assume(last_termination_index > 0);
		while (lexer.read_pos < last_termination_index)//while lexer.read_pos < last_termination
		{
			char	*line = buffer + lexer.read_pos;
			line_handler(line, symboles);
			lexer.read_pos += strlen(line) + 1;
		}
		assume(lexer.read_pos - 1 == last_termination_index);
		// copy leftovers infront of the buffer and set a negative lexer index
		left_over_count = bytes - (last_termination_index + 1);
		strncpy(buffer - left_over_count, buffer + lexer.read_pos,
			left_over_count);
		lexer.read_pos = -1 * left_over_count;
	}
	assert(read_return == 0);
	assume(!left_over_count);
	close(fd);
}

void	init_symboles(t_symbole_table *symboles)
{
	add_symbole(symboles, "R0", 0, true);
	add_symbole(symboles, "R1", 1, true);
	add_symbole(symboles, "R2", 2, true);
	add_symbole(symboles, "R3", 3, true);
	add_symbole(symboles, "R4", 4, true);
	add_symbole(symboles, "R5", 5, true);
	add_symbole(symboles, "R6", 6, true);
	add_symbole(symboles, "R7", 7, true);
	add_symbole(symboles, "R8", 8, true);
	add_symbole(symboles, "R9", 9, true);
	add_symbole(symboles, "R10", 10, true);
	add_symbole(symboles, "R11", 11, true);
	add_symbole(symboles, "R12", 12, true);
	add_symbole(symboles, "R13", 13, true);
	add_symbole(symboles, "R14", 14, true);
	add_symbole(symboles, "R15", 15, true);
	add_symbole(symboles, "SCREEN", 16384, true);
	add_symbole(symboles, "KBD", 24576, true);
	add_symbole(symboles, "SP", 0, true);
	add_symbole(symboles, "LCL", 1, true);
	add_symbole(symboles, "ARG", 2, true);
	add_symbole(symboles, "THIS", 3, true);
	add_symbole(symboles, "THAT", 4, true);
}

void	add_goto(char *line, t_symbole_table *symboles)
{
	static int	pc = 0;
	if (*line != '(')
	{
		pc++;
		return ;
	}
	line++;//todo is this corret?
	add_symbole(symboles, line, pc, true);
}

void	first_pass(char *file, t_symbole_table *symboles)
{
	init_symboles(symboles);
	loop(file, add_goto, symboles);
}

void	second_pass(char *file, t_symbole_table *symboles)
{
	loop(file, instruction_lexer, symboles);
}

int main(int ac, char *av[])
{
	char *path = "test.asm";
	if (ac > 1)
		path = av[1];
	out_fd = open("test.hack", O_WRONLY | O_TRUNC | O_CREAT, 0644);
	if (errno)
	{
		printf("%s\n", strerror(errno));
	}
	assert(out_fd > 0);
	t_symbole_table	symboles = {0};
	if (ac > 1)
		path = av[1];
	first_pass(path, &symboles);
	second_pass(path, &symboles);
	close(out_fd);
	return (0);
}
