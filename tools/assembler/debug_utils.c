#include "assembler.h"

void	panic(char *file, int line)
{
	fprintf(stderr, "Error: %s. In file %s and line %d\n", strerror(errno),
		file, line);
	exit(errno);
}

void	print_instruction(char *str, t_instruction inst, int fd)
{
	uint16_t	val = inst.full;
	uint16_t	mask = 0x8000;
	if (str && fd == 1)
		write(1, str, strlen(str));
	for (int i = 15; i >= 0; i--)
	{
		if (val & mask)
			write(fd, "1", 1);
		else
			write(fd, "0", 1);
		//printf("%u", (val & mask) >> i);
		mask >>= 1;
		if (fd == 1 && (i == 15 || i == 13 || i == 12 || i == 6 || i == 3 || i == 0))
			write(1, "|", 1);
	}
	write(fd, "\n", 1);
}
