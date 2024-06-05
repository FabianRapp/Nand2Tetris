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
# define THREAD_COUNT 2
#endif

#ifndef BUFFER_SIZE
# define BUFFER_SIZE (READ_CHUNK_SIZE * (THREAD_COUNT + 10) * 2)
#endif

typedef struct s_token	t_token;

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
	volatile int			index;
	volatile struct s_node	*volatile next;
};

struct s_ring_buffer
{
	volatile char			buffer[BUFFER_SIZE + 2];
	atomic_size_t	tail;
	atomic_size_t	head;
	atomic_size_t	finished;
	//atomic_flag	finished;
};

typedef struct s_token
{
	char				*volatile data;
	volatile size_t		index;
	t_token				*volatile next;
}	t_token;

typedef struct s_token_queue
{
	t_token						*volatile head;
	t_token						*volatile tail;
	pthread_mutex_t	mutex;
	pthread_cond_t		not_empty;
	pthread_cond_t		not_full;//later
	volatile int				count;
	int							max_size;//later to move away from the heap
}	t_token_queue;

//volatiles should not be needed since the structs thems selfs
//have made the fields volatile
typedef struct	s_lexer
{
	struct s_ring_buffer	*buffer;
	t_token_queue			*queue;
}	t_lexer;

//volatiles should not be needed since the structs thems selfs
//have made the fields volatile
struct s_reader
{
	char							*path;
	int								fd;
	struct s_ring_buffer	*buffer;
};

//volatiles should not be needed since the structs thems selfs
//have made the fields volatile
typedef struct s_parser
{
	t_token_queue	*queue;
}	t_parser;

struct s_scheduler
{
	t_token_queue			token_queue;
	struct s_ring_buffer	buffer;
	t_lexer					lexer;
	pthread_t				scanner_thread;
	pthread_t				parser_threads[THREAD_COUNT];
	t_parser				parser_data[THREAD_COUNT];
	pthread_t				reader_thread;
	struct s_reader			reader;
};

void	init_token_queue(t_token_queue *queue);
t_token	*get_token(t_token_queue *queue);
void	add_token(t_token_queue *queue, t_token	new_token);
void	clean_queue(t_token_queue *queue);
#endif
