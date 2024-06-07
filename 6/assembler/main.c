#include "assembler.h"
#include "syntax.h"

//============global vars for testing thread syncro ==========
int				out_fd;
pthread_mutex_t	out_mutex;
unsigned long long line_nb = 0;
unsigned long long read_chunks = 0;

//=============================================================

// todo: this function should not exists and token_data_extraction should be used
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
		while (!(token.data))
		{
			usleep(1000);
			token.data = malloc(sizeof(char) * (1 + len));
		}
		token.data[len] = 0;
		// what ever just execued max once
		for (int i = 0; i + next_line_start < local_head; i++)
		{
			token.data[i] =
				data->buffer->buffer[(i + next_line_start) % BUFFER_SIZE];
		}
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
	assume(size < BUFFER_SIZE);
	char *data = malloc(sizeof(char) * (size + 1));
	while (!data)
	{
		usleep(1000);
		data = malloc(sizeof(char) * (size + 1));
	}
	assume(data != NULL);
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
		assume(first_size < BUFFER_SIZE);
		assume(size - first_size < BUFFER_SIZE);
		memcpy(data, (void *)buffer + real_start, first_size);
		memcpy(data + first_size, (void *)buffer, size - first_size);
	}
	data[size] = 0;
	return (data);
}

inline int8_t	first_0xff_other_0x00(__m128i	bytes)
{
	int	mask = _mm_movemask_epi8(bytes);
	if (!mask)
		return (-1);
	assume(mask <= 0x7FFF && mask > 0);
	return (__builtin_clz(mask));//clz counts leading zeros
}

void	*lexer(void *thread_data)
{
	__m128i	newline = _mm_set1_epi8('\n');
	__m128i	variable = _mm_set1_epi8('@');
	__m128i	line_var_start = _mm_set1_epi8('(');
	__m128i	line_var_end = _mm_set1_epi8(')');
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
			__m128i nl_cmp = _mm_cmpeq_epi8(vec, newline);//16 bytes result,
					 //1 for each char, each byte either full 1 or full 0
			int nl_mask = _mm_movemask_epi8(nl_cmp);//takes the msb. of each byte 
					 //and stores it in the first 16 bits
			int offset = 0;
			int i = 0;
			while (nl_mask != 0)
			//if (mask != 0)
			{
				offset = __builtin_ctz(nl_mask);//count trailing zeros as the index
				//printf("offset: %d\n", offset);
				if (offset + local_tail >= local_head)
					break ;
				t_token	token;
				assume(offset<16);
				int	real_offset = ((long)local_tail) - ((long)next_line_start) + offset;
				//skip lines that can only be a comment or nothing (valid syntax is expected)
				if (real_offset > 0)
				{
					assume(real_offset >= 0);
					token.data = token_data_extraction(data->buffer->buffer,
						next_line_start, real_offset);
					token.index = token_index++;
					add_token(data->queue, token);
				}
				next_line_start = local_tail + offset + 1;
				atomic_store(&data->buffer->tail, next_line_start);
				//mask >>= offset + 1;
				nl_mask &= nl_mask - 1; //remove the least significant bit
				i++;
			}
			local_tail += 16;
			adjusted_index = local_tail % BUFFER_SIZE;
		}
	}
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

size_t	remove_spaces_cpu(volatile char *buffer, int read_bytes)
{
	size_t	byte_count = 0;
	static bool	empty_line = true;
	static bool	is_comment = false;
	assume(read_bytes > 0 && read_bytes <= READ_CHUNK_SIZE);
	for (int i = 0; i < read_bytes; i++)
	{
		if (buffer[i] == '/' || is_comment)
			is_comment = true;
		// the provided assembly code is littered with unprintable ascii values
		// the tools seem to ignore it the same way they ignore spaces
		// space does not need to be checked since it's ascii is 32 but
		// I will leave it for clearness
		// for somereason tabs are interpreted as normal ascii and new lines are
		// the instruction or comment terminations
		while (i < read_bytes && buffer[i] != '\n' && (is_comment ||
			(buffer[i] != '\t'
			&& (buffer[i] == ' ' | buffer[i] < 33 || buffer[i] > 126))))
		{
			i++;
		}
		if (i == read_bytes)
			return (byte_count);
		if (buffer[i] == '\n' && !empty_line)
		{
			buffer[byte_count++] = '\n';
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

// TODO
// the lexing done in here should not be done by this thread
// move it to the lexer so the read can be done in efficient steps
void	*read_file(void *data)
{
	struct s_reader			*reader = (struct s_reader *)data;

	assume(reader->path != NULL);
	reader->fd = open(reader->path, O_RDONLY);
	int unaligned_read_flags = fcntl(reader->fd, F_GETFL);
	//fcntl(reader->fd, F_SETFL, unaligned_read_flags | O_DIRECT);
	int aligned_read_flags = fcntl(reader->fd, F_GETFL);
	assume(reader->fd > 0);
	volatile int read_return = 1;
	volatile size_t	local_head = 0;
	volatile size_t	local_tail = 0;
	while (read_return > 0)
	{
		// buffer is full, let lexer empty the buffer for a while and try
		// again
		assume(local_head >= local_tail);
		while (local_head - local_tail > BUFFER_SIZE - READ_CHUNK_SIZE)
		{
			usleep(3000);
			local_tail = atomic_load(&reader->buffer->tail);
		}
		volatile unsigned offset = local_head % BUFFER_SIZE;
		int reading_count = READ_CHUNK_SIZE;
		if (READ_CHUNK_SIZE > BUFFER_SIZE - offset)
		{
			reading_count = BUFFER_SIZE - offset;
			assume(reading_count > 0 && reading_count < READ_CHUNK_SIZE);
			read_return = read(reader->fd, (void *)
					(reader->buffer->buffer + offset), reading_count);
		}
		else
		{
			//assume(offset % READ_CHUNK_SIZE == 0);
			read_return = read(reader->fd, (void *)
					  (reader->buffer->buffer + offset), READ_CHUNK_SIZE);
		}
			if (read_return < 0)
		{
			fprintf(stderr, "error reading file\n");
			exit(1);
		}
		if (read_return == 0)
			break ;
		size_t new_bytes = remove_spaces_cpu(reader->buffer->buffer + offset, read_return);
		//printf("%s", reader->buffer->buffer + offset);
		++read_chunks;
		local_head += new_bytes;
		local_tail = atomic_load(&reader->buffer->tail);
		atomic_store(&reader->buffer->head, local_head);
	}
	if (read_return !=0)
	{
		printf("%s\n", strerror(errno));
		assume(read_return != 0);
	}
	printf("reading sending finsihed sig\n");
	atomic_store(&reader->buffer->finished, 1);
	//atomic_flag_test_and_set(&reader->buffer->finished);
	printf("exiting read with local_head: %lu\n", local_head);
	close(reader->fd);
	return (NULL);
}

//TODO: refactor
void	init(struct s_scheduler *data)
{
	const pthread_attr_t *thread_flags = {0};//todo
	void *(*const thread_fn)(void *)= parser;//todo
	
	init_token_queue(&data->token_queue);
	data->lexer.queue = &data->token_queue;
	assume(data->reader.path != NULL);
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
	close(out_fd);
	return (0);
}

