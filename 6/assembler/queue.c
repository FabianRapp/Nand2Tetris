#include "assembler.h"

void	init_token_queue(t_token_queue *queue)
{
	queue->head = NULL;
	queue->tail = NULL;
	queue->count = 0;
	pthread_mutex_init(&queue->mutex, 0);
	pthread_cond_init(&queue->not_full, 0);
	pthread_cond_init(&queue->not_empty, 0);
}

//cleans locks
void	clean_queue(t_token_queue *queue)
{
	pthread_mutex_destroy(&queue->mutex);
	pthread_cond_destroy(&queue->not_empty);
	pthread_cond_destroy(&queue->not_full);
}

t_token	*get_token(t_token_queue *queue)
{
	pthread_mutex_lock(&queue->mutex);
	while (queue->count == 0)
	{
		pthread_cond_wait(&queue->not_empty, &queue->mutex);
	}
	t_token	*token = queue->head;
	if (queue->head->next)
		queue->head = queue->head->next;
	else
		queue->head = NULL;
	queue->count -= 1;
	pthread_mutex_unlock(&queue->mutex);
	return (token);
}

// for now allocates dynamically
void	add_token(t_token_queue *queue, t_token	new_token)
{
	t_token	*token = malloc(sizeof(t_token));
	assert(token);
	memcpy(token, &new_token, sizeof(t_token));
	token->next = NULL;
	// todo
	// later add condition here to check if queue is full
	// when moving it off the heap
	pthread_mutex_lock(&queue->mutex);
	if (queue->head)
		queue->head->next = token;
	queue->head = token;
	queue->count += 1;
	pthread_cond_signal(&queue->not_empty);
	pthread_mutex_unlock(&queue->mutex);
}

