#include "assembler.h"
#include "syntax.h"

//this file is only meant to veryfi that the base scheduler consept works
//it should just copy the input file into cmp
//
//TODO: first naive implemenation uses a sningle mutex for all shared data

//============global vars for testing thread syncro ==========
int				out_fd;
pthread_mutex_t	out_mutex;
unsigned long long line_nb = 0;
unsigned long long read_chunks = 0;
//=============================================================

void	compute_wait_data(struct s_ring_buffer *buffer)
{
}

// appends last token even if is not terminated in the buffer
void	append_last_token(size_t next_line_start, size_t local_head,
			t_lexer *data, size_t token_index)
{
	local_head = atomic_load(&data->buffer->head); 
	if (next_line_start < local_head)
	{
		t_token	token;
		token.index = token_index++;
		size_t	len = local_head - next_line_start;
		token.data = malloc(sizeof(char) * (1 + len));
		token.data[len] = 0;
		// what ever just execued max once
		for (int i = 0; i + next_line_start < local_head; i++)
		{
			token.data[i] =
				data->buffer->buffer[(i + next_line_start) % BUFFER_SIZE];
		}
		//printf("left over:\n");
		//printf("%s", token.data);
		add_token(data->queue, token);
	}
}

// exctracts string data from the ring buffer from the absolute index start
// until start + offset
// the real index of the undelying array is index % BUFFER_SIZE
// start : inclusive
// start + offset : inclusive
char	*token_data_extraction(volatile char *buffer, size_t start, size_t offset)
{
	size_t	size = offset + 1;
	assert(size < BUFFER_SIZE);
	char *data = malloc(sizeof(char) * (size + 1));
	assert(data != NULL);
	size_t	real_start = start % BUFFER_SIZE;
	if (real_start + offset < BUFFER_SIZE)
	{
		//printf("if\n");
		//write(1, (char *)buffer + real_start, size);

		memcpy(data, (void *)buffer + real_start, size);
	}
	else
	{
		//printf("else\n");
		size_t first_size = BUFFER_SIZE - real_start;
		assert(first_size < BUFFER_SIZE);
		assert(size - first_size < BUFFER_SIZE);
		memcpy(data, (void *)buffer + real_start, first_size);
		memcpy(data + first_size, (void *)buffer, size - first_size);
	}
	data[size] = 0;
	return (data);
}

/*
*	TODO:
*	- REIMPLEMENT TO BUG FIX, THIS IS NOT POSSIBLE TO DEBUG ANYMORE
*	- check what other lexerical analysis would mke sense using vectors
*	- improve memory access pattern
*/
void	*lexer(void *thread_data)
{
	__m128i	newline = _mm_set1_epi8('\n');
	t_lexer	*data = (t_lexer*)thread_data;
	size_t	token_index = 0;
	size_t	local_tail = 0;
	size_t	local_head = 1;
	size_t	next_line_start = 0;
	size_t	adjusted_index = 0;
	while (local_tail < local_head || !atomic_load(&data->buffer->finished))
	{
		local_head = atomic_load(&data->buffer->head);
		local_tail = next_line_start;
		while (local_tail < local_head)
		{
			__m128i	vec = _mm_loadu_si128(
				(__m128i *)(data->buffer->buffer + adjusted_index));
			__m128i cmp = _mm_cmpeq_epi8(vec, newline);//16 bytes result,
					 //1 for each char, each byte either full 1 or full 0
			int mask = _mm_movemask_epi8(cmp);//takes the msb. of each byte 
					 //and stores it in the first 16 bits
			//printf("mask: %x\n", mask);
			int offset = 0;
			//while (mask != 0)
			if (mask != 0)
			{
				offset = __builtin_ctz(mask);//count trailing zeros as the index
				//printf("offset: %d\n", offset);
				if (offset + local_tail >= local_head)
					break ;
				t_token	token;
				assert(offset<16);
				int	real_offset = ((long)local_tail) - ((long)next_line_start) + offset;
				//printf("l tail: %lu, nlstart: %lu\n", local_tail, next_line_start);
			//	printf("real_offset: %d\n", real_offset);
				assert(real_offset >= 0);
				token.data = token_data_extraction(data->buffer->buffer,
					next_line_start, real_offset);
				token.index = token_index++;
				//write(out_fd, token.data, strlen(token.data));
				//printf("%s", token.data);
				add_token(data->queue, token);
				next_line_start = local_tail + offset + 1;
				atomic_store(&data->buffer->tail, next_line_start);
				mask >>= offset + 1;
				mask &= mask - 1; //remove the least significant bit
			}
			local_tail += 16;
			adjusted_index = local_tail % BUFFER_SIZE;
		}
		//if (next_line_start)
			//atomic_store(&data->buffer->tail, next_line_start - 1);
		//atomic_store(&data->buffer->tail, local_tail);
	}
	usleep(10000);//race codition..
	append_last_token(next_line_start, local_head, data, token_index);
	printf("exiting lexer, local_tail: %lu, local_head: %lu\n", local_tail, local_head);
	return (NULL);
}

void	*parser(void *thread_data)
{
	t_parser	*data = (t_parser *)thread_data;
	
	//return(NULL);
	while (1)//todo exit condition
	{
		pthread_mutex_lock(&out_mutex);
		t_token	*token = get_token(data->queue);
		write(out_fd, token->data, strlen(token->data));
		pthread_mutex_unlock(&out_mutex);
		//printf("parsed token %lu: %s\n", token->index, token->data);
		free(token->data);
		free(token);
	}
	return (NULL);
}

void	*read_file(void *data)
{
	struct s_reader			*reader = (struct s_reader *)data;

	assert(reader->path != NULL);
	reader->fd = open(reader->path, O_RDONLY);
	assert(reader->fd > 0);
	volatile int read_return = 1;
	volatile size_t	local_head = 0;
	volatile size_t	local_tail = 0;
	while (read_return > 0)
	{
		// buffer is full, let lexer empty the buffer for a while and try
		// again
		while (local_head - local_tail > BUFFER_SIZE - READ_CHUNK_SIZE)
		{
			usleep(3000);
			local_tail = atomic_load(&reader->buffer->tail);
		}
		volatile unsigned offset = local_head % BUFFER_SIZE;
		read_return = read(reader->fd, (void *)(reader->buffer->buffer + offset), READ_CHUNK_SIZE);
		assert(read_return >= 0);
		++read_chunks;
		local_head += read_return;
		local_tail = atomic_load(&reader->buffer->tail);
		atomic_store(&reader->buffer->head, local_head);
	}
	if (read_return !=0)
	{
		printf("%s\n", strerror(errno));
		assert(read_return != 0);
	}
	printf("reading sending finsihed sig\n");
	atomic_store(&reader->buffer->finished, 1);
	//atomic_flag_test_and_set(&reader->buffer->finished);
	printf("exiting read with local_head: %lu\n", local_head);
	return (NULL);
}

//TODO: refactor
void	init(struct s_scheduler *data)
{
	const pthread_attr_t *thread_flags = {0};//todo
	void *(*const thread_fn)(void *)= parser;//todo
	
	init_token_queue(&data->token_queue);
	data->lexer.queue = &data->token_queue;
	assert(data->reader.path != NULL);
	data->lexer.buffer = &data->buffer;
	//atomic_flag_clear(&data->buffer.finished);
	atomic_store(&data->buffer.finished, 0);
	atomic_store(&data->buffer.head, 0);
	atomic_store(&data->buffer.tail, 0);

	data->reader.buffer = &data->buffer;
	pthread_create(&data->reader_thread, thread_flags, read_file, &data->reader);
	pthread_create(&data->scanner_thread, thread_flags, lexer, &data->lexer);
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		data->parser_data[i].queue = &data->token_queue;
		pthread_create(data->parser_threads + i, thread_flags, thread_fn,
				 data->parser_data + i);
	}
	printf("all threads running\n");
}

//todo: replace pthread_join() with pthread_cancel() and the nececary cleanups
//this allows to remove a huge lock overhead by removing exit checks
//this can be done after the read thread joins, the parsing and lexing
//progress needs to be checked from the main process to not terminate early
//alternatly to pthread_cancel() the program could just exit and let the os
//deal with the mess...this would make this project not really extendable tho
void	wait_threads(struct s_scheduler	*data)
{
	pthread_join(data->reader_thread, NULL);
	printf("reader joined\n");
	pthread_join(data->scanner_thread, NULL);
	printf("lexer joined\n");
	//usleep(1000000);
	//exit(0);//todo parser threas can currently not exit


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
	pthread_mutex_init(&out_mutex, 0);
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

