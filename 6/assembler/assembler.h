#ifndef ASSEMBLER_H
# define ASSEMBLER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <stdatomic.h>

#ifndef READ_CHUNCK_SIZE
# define READ_CHUNK_SIZE 2560000 //has to be a muliple of 16
#endif

#ifndef DEFAULT_TABLE_SIZE
# define DEFAULT_TABLE_SIZE 1000
#endif

#ifndef HASH_PRIME
# define HASH_PRIME 31
#endif

#ifndef THREAD_COUNT
# define THREAD_COUNT 1
#endif

#ifndef BUFFER_SIZE
# define BUFFER_SIZE (READ_CHUNK_SIZE * (THREAD_COUNT + 10) * 2)
#endif

struct s_symbole_reference
{
	struct s_symbole_reference	*next;
	//some sort of index of where the symbole was unknown
};

struct s_symbole_node
{
	int							val;
	char						*name;
	struct s_symbole_node		*next;
	struct s_symbole_reference	*to_fill_symboles;
};

typedef struct s_symbol_table
{
	struct s_symbole_node base_arr[DEFAULT_TABLE_SIZE];
}	t_symbol_table;

struct s_node
{
	int				index;
	struct s_node	*next;
};

typedef struct s_stack
{
    struct node	*top;
} t_stack;

struct s_ring_buffer
{
	char				buffer[BUFFER_SIZE + 2];
	atomic_size_t		tail;
	atomic_size_t		head;
	atomic_flag			finished;
};

struct s_nl_scanner
{
	t_stack					*line_stack;
	struct s_ring_buffer	*buffer;
};

struct s_thread_data
{
	int8_t					thread_index;
	pthread_mutex_t			idel_mutex;
	bool					idel;
	struct s_ring_buffer	*buffer;
};


struct s_reader
{
	char					*path;
	int						fd;
	struct s_ring_buffer	*buffer;
};

struct s_scheduler
{
	t_stack					line_stack;
	struct s_ring_buffer	buffer;
	struct s_nl_scanner		scanner;
	pthread_t				scanner_thread;
	pthread_t				threads[THREAD_COUNT];
	struct s_thread_data	thread_data[THREAD_COUNT];
	pthread_t				reader_thread;
	struct s_reader			reader;
};


#endif
