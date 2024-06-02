// creates a huge test.asm file pased of Pong.asm

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
	errno = 0;
	char *file_in = "Pong.asm";
	int fd_in = open(file_in, 0);
	int fd_out = open("test.asm", O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (errno)
	{
		close(fd_in);
		close(fd_out);
		printf("%s\n", strerror(errno));
		exit(errno);
	}
	char *base[210000];
	int base_size = read(fd_in, base, 210000);
	printf("base_size: %d\n", base_size);
	close(fd_in);
	long size_to_write = 10737418240;
	int write_size = write(fd_out, base, base_size);
	while (size_to_write > 0 && write_size > 0)
	{
		size_to_write -= base_size;
		write(fd_out, "\n", 1);
		write_size = write(fd_out, base, base_size);
	}
	close(fd_out);
	printf("test file successfully generated\n");
	return (0);
}
