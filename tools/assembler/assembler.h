#ifndef ASSEMBLER_H
# define ASSEMBLER_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>
#include <stdalign.h>
#include <machine/endian.h>

//intel headers
#include <x86intrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

// cuda
//#include <cuda_runtime.h>

#define unreachable __builtin_unreachable

//#define NDEBUG 1
#if defined(NDEBUG)
#define assume(cond) do { if (!(cond)) __builtin_unreachable(); } while (0)
#else
#include <assert.h>
#define assume(cond) assert(cond)
#endif


#ifndef READ_CHUNCK_SIZE
# define READ_CHUNK_SIZE 2097152 //has to be a power of 2 
//# define READ_CHUNK_SIZE 1048576
//# define READ_CHUNK_SIZE 524288
//# define READ_CHUNK_SIZE 4194304
//# define READ_CHUNK_SIZE 262144
#endif

#ifndef TABLE_SIZE
# define TABLE_SIZE 1000
#endif

#ifndef HASH_PRIME
# define HASH_PRIME 31
#endif

#ifndef THREAD_COUNT
# define THREAD_COUNT 6
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
	struct s_symbole_node base_arr[TABLE_SIZE];
}	t_symbol_table;

struct s_node
{
	volatile int			index;
	volatile struct s_node	*volatile next;
};

struct s_ring_buffer
{
	alignas(READ_CHUNK_SIZE) volatile char	buffer[BUFFER_SIZE + 2];
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
	t_token				*volatile head;
	t_token				*volatile tail;
	pthread_mutex_t		mutex;
	pthread_cond_t		not_empty;
	pthread_cond_t		not_full;//later
	volatile int		count;
	int					max_size;//later to move away from the heap
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

#if BYTE_ORDER == LITTLE_ENDIAN
typedef struct s_c_instr
{
	uint16_t		jmp			:3;
	uint16_t		dest		:3;
	uint16_t		comp		:7;
	uint16_t		padding		:2;
	uint16_t		is_c_inst	:1;
}	__attribute__((packed)) t_c_instr;

typedef struct s_a_instr
{
	uint16_t		address			:15;
	uint16_t		is_not_a_inst	:1;
}	__attribute__((packed)) t_a_instr;

#else

typedef struct s_c_instr
{
	uint16_t		is_c_inst	:1;
	uint16_t		padding		:2;
	uint16_t		comp		:7;
	uint16_t		dest		:3;
	uint16_t		jmp			:3;
}	__attribute__((packed)) t_c_instr;

typedef struct s_a_instr
{
	uint16_t		is_not_a_inst	:1;
	uint16_t		address			:15;
}	__attribute__((packed)) t_a_instr;

#endif

// binary data of instruction
typedef union u_instruction
{
	uint16_t		full;
	t_c_instr		c;
	t_a_instr		a;
}	__attribute__((packed)) t_instruction;

typedef struct s_linked_list_arr
{
	alignas(16) t_instruction				arr[2000];
	struct s_linked_list_arr	*next;
}	t_linked_list_arr;

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

void	panic(char *file, int line);

// signle_instruction_handeling.c
void	handle_instruction(char *single_instruction_str_data);
t_instruction	test_handle_instruction(char *single_instruction_str_data);


//queue.c
void	init_token_queue(t_token_queue *queue);
t_token	*get_token(t_token_queue *queue);
void	add_token(t_token_queue *queue, t_token	new_token);
void	clean_queue(t_token_queue *queue);

//debug_utils.c
void	print_instruction(char *str, t_instruction instruction, int fd);

#endif
