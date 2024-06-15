#include "assembler.h"
#include "simple_assembler.h"
/*
	- dual pass: first pass store symoles, second translate to binary
	- will crash on huge files duo to no malloc protection
*/
//===================basic ll==================
//=================================================================

int out_fd = 1;

t_a_instr	a_instr_parser(t_symbole_table *symboles, char *str)
{
	static int	next_new_var_val = 16;
	t_a_instr	instruction = {0};

	str++; // skip '@'
	assume(strlen(str) > 0);
	if (*str >= '0' && *str <= '9')
		instruction.address = (uint16_t)atoi(str);
	else
	{
		instruction.address = (uint16_t)add_symbole(
			symboles, str, next_new_var_val, false);
		if (*str != ' ')//hack: str[0] == ' ' symboles a the variable is new
			next_new_var_val++;
	}
	return (instruction);
}

void	instruction_parser(char *line, t_symbole_table *symboles)
{
	t_instruction	instruction = {0};

	if (*line != '@' && *line != '(')
	{
		instruction.c = c_instr_finite_state_machine(line);
	}
	else if (*line == '@')
	{
		instruction.a = a_instr_parser(symboles, line);
	}
	else
	{
		assume(*line == '(');
		return ;
	}
// the subission asked for the file me in an ascii format
// to combile to binary add -DBINARY_ASSEMBLY=1 to CFLAGS in the makefile
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
	int			fd = open(file, O_RDONLY, O_NONBLOCK);
	const int	longest_instruction_len = 12;
	char		raw_buffer[2 * READ_LEN + 1 + longest_instruction_len];
	// alignging the buffer for O_NONBLOCK (left over from multi-threaded
	// implementation, meant to reduce cache usage by the reading thread)
	// also leaves a few bytes infront of the buffer pointer for
	// negative indexing
	char		*const buffer = align_buffer(raw_buffer);
	int			read_return;
	int			last_termination_index;
	int			left_over_count = 0;
	int			position = 0;

	while (1)
	{
		read_return = read(fd, buffer, READ_LEN);
		if (read_return < 0)
			panic(__FILE__, __LINE__);
		if (!read_return)
			break ;
		int bytes = filter_charset(buffer, read_return);
		buffer[bytes] = 0;
		last_termination_index = bytes -1;
		while (last_termination_index >= 0 && buffer[last_termination_index])
		{
			last_termination_index--;
		}
		assume(last_termination_index > 0);
		while (position < last_termination_index)
		{
			char	*instruction = buffer + position;
			line_handler(instruction, symboles);
			position += strlen(instruction) + 1;
		}
		assume(position - 1 == last_termination_index);
		// copy leftovers infront of the buffer and set a negative position
		left_over_count = bytes - (last_termination_index + 1);
		strncpy(buffer - left_over_count, buffer + position,
			left_over_count);
		position = -1 * left_over_count;
	}
	assume(!left_over_count);
	close(fd);
}

void	add_macros(t_symbole_table *symboles)
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
	add_macros(symboles);
	loop(file, add_goto, symboles);
}

void	second_pass(char *file, t_symbole_table *symboles)
{
	loop(file, instruction_parser, symboles);
}

int main(int ac, char *av[])
{
	char	*path;

	if (ac > 1)
	{
		char	*asm_type = strstr(av[1], ".asm");
		if (!asm_type || asm_type[4])
		{
			fprintf(stderr, "wrong file type, expected .asm\n");
			exit(1);
		}
		path = av[1];
		if (ac > 2)
			out_fd = open(av[2], O_WRONLY | O_TRUNC | O_CREAT, 0644);
		else
		{
			size_t	out_file_len = 5 + asm_type - av[1];
			char	*out_file = malloc((out_file_len + 1) * sizeof(char));
			if (!out_file)
				panic(__FILE__, __LINE__);
			memcpy(out_file, av[1], out_file_len - 5);
			strcpy(out_file + out_file_len - 5, ".hack");
			out_fd = open(out_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
			free(out_file);
		}
	}
	else
	{
		fprintf(stderr, "no file provided, using test.asm\n");
		path = "test.asm";
		out_fd = open("test.hack", O_WRONLY | O_TRUNC | O_CREAT, 0644);
	}
	if (errno)
		panic(__FILE__, __LINE__);
	t_symbole_table	symboles = {0};
	first_pass(path, &symboles);
	second_pass(path, &symboles);
	close(out_fd);
	if (errno)
		panic(__FILE__, __LINE__);
	printf("finished!\n");
	return (0);
}
