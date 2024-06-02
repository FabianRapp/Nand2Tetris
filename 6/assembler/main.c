#include "assembler.h"

void	read_file(char *file)
{
	int fd = open(file, O_RDONLY);
	printf("%s\n", file);
	static char buffer[1000];
	int read_val = read(fd, buffer, 1000);
	while (read_val > 0)
	{
		//write(0, buffer, read_val);
		read_val = read(fd, buffer, 1000);
	}
	if (read_val < 0)
	{
		printf("%s\n", strerror(errno));
		exit(errno);
	}
}


int main(int ac, char *av[])
{
	t_assem	m_data;

	if (ac == 1)
	{
		m_data.path = "test.asm";
		//printf("please provide path to .asm file "
		// "(./assembler <PATH_TO_FILE>)\n");
		//exit(1);
	}
	else
		m_data.path = av[1];
	printf("assembling %s..\n", m_data.path);
	clock_t begin = clock();
	read_file(m_data.path);
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("%lf seconds\n", time_spent);
	return (0);
}

