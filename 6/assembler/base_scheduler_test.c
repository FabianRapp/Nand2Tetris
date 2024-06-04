#include "assembler.h"
#include "syntax.h"

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
void	*lexer(void *thread_data)
{
	__m128i	newline = _mm_set1_epi8('\n');
	t_lexer	*data = (t_lexer*)thread_data;

	size_t	local_tail = 0;
	size_t	local_head;
	size_t	next_line_start = 0;
	size_t	adjusted_index = 0;
	while (!atomic_flag_test_and_set(&data->buffer->finished))
	{
		atomic_flag_clear(&data->buffer->finished);
		local_head = atomic_load(&data->buffer->head);
		while (local_tail < local_head)
		{
			//printf("local_reader_pos: %lu\n", local_read_pos);
			//todo: handle unalinged data before this loop
			__m128i	vec = _mm_loadu_si128(
				(__m128i *)(data->buffer->buffer + adjusted_index));
			//compares byte by byte for byte equlity
			__m128i cmp = _mm_cmpeq_epi8(vec, newline);
			// returns 1 bit for each byte, if the byte is 0 returns 0, otherwise 1
			int mask = _mm_movemask_epi8(cmp);
			while (mask != 0)
			{
				int offset = __builtin_ctz(mask);//count trailing zeros as the index
				if (offset)
				{
					t_token	token;
					size_t	line_end = adjusted_index + offset;
					if (line_end > next_line_start)
					{
						size_t	line_size = sizeof(char)
							* (line_end - next_line_start + 1);
						token.data = malloc(line_size + 1);
						//printf("line_size: %lu\n, offset: %d\n, line_end: %lu\n next_start:%lu\n", line_size, offset, line_end, next_line_start);
						assert(token.data);
						memcpy(token.data,
							data->buffer->buffer + adjusted_index, line_size);
						token.data[line_size] = 0;
						next_line_start = line_end + 1;
						add_token(data->queue, token);
						//free(token.data);//todo add to stack instead
					}
					else
					{//todo
					}
				}

				mask &= mask - 1; //remove the least significant bit
			}
			local_tail += 16;
			adjusted_index = local_tail % BUFFER_SIZE;
		}
		atomic_store(&data->buffer->tail, local_tail);
	}
	return (NULL);
}
//TODO: rework
void	*compute_thread(void *thread_data)
{
	t_parser	*data = (t_parser *)thread_data;
	return (NULL);
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
	
	init_token_queue(&data->token_queue);
	data->lexer.queue = &data->token_queue;
	assert(data->reader.path != NULL);
	data->lexer.buffer = &data->buffer;
	atomic_flag_clear(&data->buffer.finished);
	atomic_store(&data->buffer.head, 0);
	atomic_store(&data->buffer.tail, 0);

	data->reader.buffer = &data->buffer;
	pthread_create(&data->reader_thread, thread_flags, read_file, &data->reader);
	usleep(5000);
	pthread_create(&data->scanner_thread, thread_flags, lexer, &data->lexer);
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		data->parser_data[i].queue = &data->token_queue;
		pthread_create(data->parser_threads + i, thread_flags, thread_fn,
				 data->parser_data + i);
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
		pthread_join(data->parser_threads[i], NULL);
	}
	printf("all threads joined\n");
	clean_queue(&data->token_queue);
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

