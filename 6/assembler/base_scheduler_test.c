#include "assembler.h"

//this file is only meant to veryfi that the base scheduler consept works
//it should just copy the input file into cmp
//
//TODO: first naive implemenation uses a sningle mutex for all shared data

//============global vars for testing thread syncro ==========
int				out_fd;
pthread_mutex_t	out_fd_mutex;
unsigned long long line_nb = 0;
//=============================================================

void	compute_wait_data(struct s_ring_buffer *buffer)
{
	pthread_mutex_lock(&buffer->state_mutex);
	while (buffer->write - buffer->read < READ_CHUNK_SIZE && (!buffer->finished))
	{
		printf("buffer->write: %lu, buffer->read: %lu\n",
			buffer->write, buffer->read);
		pthread_mutex_unlock(&buffer->state_mutex);
		usleep(200);
		pthread_mutex_lock(&buffer->state_mutex);
	}
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

	while (!data->buffer->finished)
	{
		size_t	local_read_pos = data->buffer->tail % BUFFER_SIZE;
		size_t	local_head = data->buffer->head % BUFFER_SIZE;
		while (local_read_pos + 16 < local_head)
		{
			//todo: handle unalinged data before this loop
			__m128i vec = _mm_loadu_si128(
				(__m128i *)(data->buffer->buffer + local_read_pos));
			//compares byte by byte for byte equlity
			__m128i cmp = _mm_cmpeq_epi8(vec, newline);
			// returns 1 bit for each byte, if the byte is 0 returns 0, otherwise 1
			int mask = _mm_movemask_epi8(cmp);			while (mask != 0)
			{
				int offset = __builtin_ctz(mask);//count trailing zeros as the index
				push(data->line_stack, local_read_pos + offset);
				mask &= mask - 1; //remove the least significant bit
			}
		}
	}
	return (NULL);
}
//TODO: rework
void	*compute_thread(void *thread_data)
{
	struct s_thread_data	*data = (struct s_thread_data *)thread_data;
	struct s_ring_buffer	*buffer = data->buffer;
	
	return (NULL);//deactived right now
	pthread_mutex_lock(&buffer->state_mutex);
	char	*part1 = 0;
	int		part1_len = 0;
	int		part2_len = 0;
	char	*part2 = 0;
	while (!(buffer->finished && buffer->read == buffer->write))
	{
		assert(buffer->write >= buffer->read);
		pthread_mutex_unlock(&buffer->state_mutex);
		compute_wait_data(buffer);//mutex locked after
		if(buffer->finished && buffer->read == buffer->write)
			break ;
		unsigned local_read = buffer->read % BUFFER_SIZE;
		part1 = buffer->buffer;
		char *line_end = strchr(buffer->buffer + local_read, '\n');
		if (!line_end)
		{
			part1 = buffer->buffer+local_read;
			part1_len = strlen(part1);
			line_end = strchr(buffer->buffer, '\n');
			if (!line_end)
			{//todo
				assert(!line_end);
			}
			part2 = buffer->buffer;
			part2_len = line_end - buffer->buffer + 1;

			//if (local_read + strlen(buffer->buffer + local_read) >= BUFFER_SIZE)
			//	assert(1);
			//todo what if line is split to end and start of buf
			//buffer->read += 1;
			//continue ;
			//if (!strlen(buffer->buffer + local_read) && buffer->read + 1 <
			//   buffer->write) //todo
			//{
			//	buffer->read = 0;
			//	continue;
			//}
			//printf("no line_end\n");
			//printf("local_read: %u\n", local_read);
			//printf("buffer->read: %lu, buffer->write: %lu, buffersizer: %d\n",
			//	buffer->read, buffer->write, BUFFER_SIZE);
			//printf("buffer: %s(len: %lu)\n", buffer->buffer + local_read,
			//	strlen(buffer->buffer + local_read));
			//pthread_mutex_unlock(&buffer->state_mutex);
			//usleep(200);
			//pthread_mutex_lock(&buffer->state_mutex);
			//continue ;
		}
		else
		{
			part1_len = 1 + line_end - buffer->buffer - local_read;
			part2_len = 0;
		}
		if (buffer->read + part1_len + part2_len >= buffer->write)
		{
			printf("buffer->read %lu + part1_len %i + part2_len %i >= buffer->write %lu\n",
				buffer->read, part1_len, part2_len, buffer->write);
			pthread_mutex_unlock(&buffer->state_mutex);
			usleep(200);
			pthread_mutex_lock(&buffer->state_mutex);
			continue ;
		}
		pthread_mutex_unlock(&buffer->state_mutex);
		pthread_mutex_lock(&out_fd_mutex);
		assert(local_read < BUFFER_SIZE);
		if (local_read + part1_len >= BUFFER_SIZE)
		{
			printf("buffer: %p, line_end: %p\n", buffer->buffer, line_end);
			printf("local_read: %u part1_len: %d\n", local_read, part1_len);
			assert(local_read + part1_len < BUFFER_SIZE);
		}
		write(out_fd, part1, part1_len);
		if (part2_len)
			write(out_fd, part2, part2_len);
		pthread_mutex_unlock(&out_fd_mutex);
		if (!(line_nb % 100000))
		{
			printf("line %llu/1523327961 (%llu%%)\n", line_nb,
				(line_nb)/(1523327961 / 100));
		}
		line_nb++;
		//if (((double)line_nb)/(1523327961.0 / 100.0) > 2.0)
			//exit(0);
		buffer->read += part1_len + part2_len;
		pthread_mutex_lock(&buffer->state_mutex);
	}
	pthread_mutex_unlock(&buffer->state_mutex);
	return (NULL);
}

void	reader_wait_buffer_space(struct s_ring_buffer *buffer,
			unsigned long local_total_write)
{
	while (1)
	{
		pthread_mutex_lock(&buffer->state_mutex);
		if (local_total_write - buffer->read < BUFFER_SIZE - READ_CHUNK_SIZE)
		{
			pthread_mutex_unlock(&buffer->state_mutex);
			return ;
		}
		//printf("buffer still full besause: "
		//	"local_total_write: %lu - buffer->read: %lu >= BUFFER_SIZE %d - READ_C.S. %d\n"
		//	"(%lu >= %d)\n"
		//	"buffer->write: %lu\n", local_total_write, buffer->read,
		//	BUFFER_SIZE, READ_CHUNK_SIZE, local_total_write - buffer->read,
		// BUFFER_SIZE- READ_CHUNK_SIZE, buffer->write);
		pthread_mutex_unlock(&buffer->state_mutex);
		usleep(5000);
	}
}

//TODO: refactor
void	*read_file(void *data)
{
	struct s_reader			*reader = (struct s_reader *)data;

	assert(reader->path != NULL);
	reader->fd = open(reader->path, O_RDONLY);
	assert(reader->fd > 0);
	bool	local_was_full = false;//if this is true needs to cehcks with mutex
									//locks (assumes only 1 file reading thread)
	int				read_return = 1;
	unsigned long	local_write_index = 0;
	unsigned long	local_total_write = 0;
	unsigned long long read_lines = 0;
	while (read_return > 0)
	{
		if (local_was_full)
		//if (0)
		{
			//printf("buffer full\n");
			reader_wait_buffer_space(reader->buffer, local_total_write);
			//printf("buffer not full anymore\n");
		}
		assert(local_write_index < BUFFER_SIZE);
		if (local_write_index + READ_CHUNK_SIZE >= BUFFER_SIZE)
		{
			printf("buffer size: %d\n", BUFFER_SIZE);
			printf("local_write: %lu\n", local_write_index);
			assert(local_write_index + READ_CHUNK_SIZE < BUFFER_SIZE);
		}
		read_return = read(reader->fd, reader->buffer->buffer + local_write_index,
			READ_CHUNK_SIZE);
		//write(out_fd, reader->buffer->buffer + local_write_index, read_return);
		if (errno)
		{
			assert(errno);
		}
		//printf("read chunk %llu\n", read_lines++);
		local_write_index += read_return;
		local_total_write += read_return;
		local_write_index %= (BUFFER_SIZE);
		if (local_write_index + READ_CHUNK_SIZE >= BUFFER_SIZE)
			local_write_index = 0;
		pthread_mutex_lock(&(reader->buffer->state_mutex));
		reader->buffer->write = local_total_write;
		local_was_full = (local_total_write - reader->buffer->read
			>= BUFFER_SIZE - READ_CHUNK_SIZE);
		reader->buffer->is_empty = false;
		pthread_mutex_unlock(&reader->buffer->state_mutex);
	}
	assert(read_return >-1);
	pthread_mutex_lock(&(reader->buffer->state_mutex));
	reader->buffer->buffer[local_write_index] = '\n';
	reader->buffer->buffer[local_write_index + 1] = 0;
	reader->buffer->finished = true;
	pthread_mutex_unlock(&reader->buffer->state_mutex);
	return (NULL);
}

//TODO: refactor
void	init(struct s_scheduler *data)
{
	const pthread_attr_t *thread_flags = {0};//todo
	void *(*const thread_fn)(void *)= compute_thread;//todo
	
	printf("qq:%s\n", data->reader.path);
	assert(data->reader.path != NULL);


	data->buffer.write = 0;
	data->buffer.read = 0;
	data->buffer.is_full = false;
	data->buffer.is_empty = true;
	data->buffer.finished = false;
	pthread_mutex_init(&data->buffer.head_mutex, 0);
	pthread_mutex_init(&data->buffer.tail_mutex, 0);
	pthread_mutex_init(&data->buffer.state_mutex, 0);
	data->reader.buffer = &data->buffer;
	pthread_create(&data->reader_thread, thread_flags, read_file, &data->reader);
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
	printf("file input finished\n");
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		pthread_join(data->threads[i], NULL);
	}
	printf("all threads joined\n");
	pthread_mutex_destroy(&data->buffer.head_mutex);
	pthread_mutex_destroy(&data->buffer.tail_mutex);
	pthread_mutex_destroy(&data->buffer.state_mutex);
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
	pthread_mutex_init(&out_fd_mutex, 0);
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
	pthread_mutex_destroy(&out_fd_mutex);
	return (0);
}

