#include "assembler.h"
int	hash(char *str)
{
	assert(str != NULL);
	int hash = 0;
	for (int i = 0; str[i]; i++)
	{
		hash += str[i] % HASH_PRIME;
	}
	return hash;
}

void	*read_file(void *data)
{
	struct s_reader	*reader = (struct s_reader *)data;
	return (NULL);
}

void	init(struct s_scheduler	*data)
{
	const pthread_attr_t *thread_flags = {0};//todo
	void *(*const thread_fn)(void *)= NULL;//todo
	
	pthread_create(&data->reader_thread, thread_flags, read_file, &data->reader);
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		pthread_create(data->threads + i, thread_flags, thread_fn,
				 data->thread_data + i);
	}
}

void	wait_threads(struct s_scheduler	*data)
{
	pthread_join(data->reader_thread, NULL);
	printf("file input finished\n");
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		pthread_join(data->threads[i], NULL);
	}
	printf("all threads joined\n");
}

void	scheduler(char *path)
{
	struct s_scheduler	data = {0};
	data.reader.path = path;
	
	init(&data);
	wait_threads(&data);
	}

int main(int ac, char *av[])
{
	char	*path = NULL;

	if (ac == 1)
	{
		path = "test.asm";
		//printf("please provide path to .asm file "
		// "(./assembler <PATH_TO_FILE>)\n");
		//exit(1);
	}
	else
		path = av[1];
	printf("assembling %s..\n", path);
	clock_t begin = clock();


	scheduler(path);












	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("%lf seconds\n", time_spent);
	return (0);
}

