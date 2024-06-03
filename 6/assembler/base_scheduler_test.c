#include "assembler.h"

//this file is only meant to veryfi that the base scheduler consept works
//it should just copy the input file into cmp
//
//TODO: first naive implemenation uses a sningle mutex for all shared data

//============global vars for testing thread syncro ==========
int				out_fd;
unsigned long long line_nb = 0;
unsigned long long read_chunks = 0;
//=============================================================

void	compute_wait_data(struct s_ring_buffer *buffer)
{
}

/*
*	TODO:
*	- multi line comment logic has to be in here
*	- single line comment logic should be in here
*	- improve memory access pattern
*/
void	*nl_scanner(void *thread_data)
{
	__m128i newline = _mm_set1_epi8('\n');
	struct	s_nl_scanner	*data = (struct s_nl_scanner *)thread_data;

	size_t	local_tail = 0;
	size_t	local_head;
	while (!atomic_flag_test_and_set(&data->buffer->finished))
	{
		atomic_flag_clear(&data->buffer->finished);
		local_head = atomic_load(&data->buffer->head);
		while (local_tail < local_head)
		{
			//printf("local_reader_pos: %lu\n", local_read_pos);
			//todo: handle unalinged data before this loop
			__m128i vec = _mm_loadu_si128(
				(__m128i *)(data->buffer->buffer +
				local_tail % BUFFER_SIZE));
			//compares byte by byte for byte equlity
			__m128i cmp = _mm_cmpeq_epi8(vec, newline);
			// returns 1 bit for each byte, if the byte is 0 returns 0, otherwise 1
			int mask = _mm_movemask_epi8(cmp);
			while (mask != 0)
			{
				int offset = __builtin_ctz(mask);//count trailing zeros as the index
				//todo: implement the stack
				//push(data->line_stack, local_read_pos + offset);
				mask &= mask - 1; //remove the least significant bit
			}
			local_tail += 16;
		}
		atomic_store(&data->buffer->tail, local_tail);
	}
	return (NULL);
}
//TODO: rework
void	*compute_thread(void *thread_data)
{
	struct s_thread_data	*data = (struct s_thread_data *)thread_data;
	struct s_ring_buffer	*buffer = data->buffer;
	
	return (NULL);//deactived right now
}

void	*read_file(void *data)
{
	struct s_reader			*reader = (struct s_reader *)data;

	assert(reader->path != NULL);
	reader->fd = open(reader->path, O_RDONLY);
	assert(reader->fd > 0);
	int read_return = 1;
	size_t	local_head = 0;
	size_t	local_tail = 0;
	while (read_return > 0)
	{
		while (local_head - local_tail > BUFFER_SIZE - READ_CHUNK_SIZE)
		{
			usleep(3000);
			local_tail = atomic_load(&reader->buffer->tail);
		}
		unsigned offset = local_head % BUFFER_SIZE;
		read_return = read(reader->fd, reader->buffer->buffer + offset, READ_CHUNK_SIZE);
		++read_chunks;
		printf("read chunks: %llu (%llu mb)\n", read_chunks, read_chunks * READ_CHUNK_SIZE / 1000000);
		local_head += READ_CHUNK_SIZE;
		local_tail = atomic_load(&reader->buffer->tail);
		atomic_store(&reader->buffer->head, local_head);
	}
	if (read_return !=0)
	{
		printf("%s\n", strerror(errno));
		assert(read_return != 0);
	}
	atomic_flag_test_and_set(&reader->buffer->finished);
	return (NULL);
}

//TODO: refactor
void	init(struct s_scheduler *data)
{
	const pthread_attr_t *thread_flags = {0};//todo
	void *(*const thread_fn)(void *)= compute_thread;//todo
	
	printf("qq:%s\n", data->reader.path);
	assert(data->reader.path != NULL);

	data->scanner.buffer = &data->buffer;
	atomic_flag_clear(&data->buffer.finished);
	atomic_store(&data->buffer.head, 0);
	atomic_store(&data->buffer.tail, 0);
	data->reader.buffer = &data->buffer;
	pthread_create(&data->reader_thread, thread_flags, read_file, &data->reader);
	usleep(5000);
	pthread_create(&data->scanner_thread, thread_flags, nl_scanner, &data->scanner);
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		data->thread_data[i].buffer = &data->buffer;
		pthread_create(data->threads + i, thread_flags, thread_fn,
				 data->thread_data + i);
	}
}

void	wait_threads(struct s_scheduler	*data)
{
	pthread_join(data->reader_thread, NULL);
	printf("reader joined\n");
	pthread_join(data->scanner_thread, NULL);
	printf("file \n");
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		pthread_join(data->threads[i], NULL);
	}
	printf("all threads joined\n");
	//pthread_cond_destroy(&data->buffer.is_full);
	//pthread_cond_destroy(&data->buffer.is_empty);
}

void	scheduler(char *path)
{
	static struct s_scheduler	data = {0};
	data.reader.path = path;
	
	init(&data);
	wait_threads(&data);
	}

void	init_thread_io_test(void)
{
	out_fd = open("cmp.asm", O_WRONLY | O_TRUNC | O_CREAT, 0644);
}

int main(int ac, char *av[])
{
	char	*path = NULL;
	init_thread_io_test();
	if (ac == 1)
		path = "test.asm";
	else
		path = av[1];
	scheduler(path);
	return (0);
}

