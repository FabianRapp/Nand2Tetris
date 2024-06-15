#include "../assembler.h"

void	init_token_queue(t_token_queue *queue)
{
	queue->head = NULL;
	queue->tail = NULL;
	queue->count = 0;
	pthread_mutex_init(&queue->mutex, 0);
	pthread_cond_init(&queue->not_empty, 0);
}

void	clean_queue_locks(t_token_queue *queue)
{
	pthread_mutex_destroy(&queue->mutex);
	pthread_cond_destroy(&queue->not_empty);
}

// can be called from many threads
t_token	*get_token(t_token_queue *queue)
{
	pthread_mutex_lock(&queue->mutex);
	while (queue->count == 0)
	{
		pthread_cond_wait(&queue->not_empty, &queue->mutex);
	}
	assume(queue->tail && queue->head);
	t_token	*token = queue->tail;
	if (queue->count > 1)
		queue->tail = token->next;
	else
	{
		queue->tail = NULL;
	}
	queue->count -= 1;
	pthread_mutex_unlock(&queue->mutex);
	if (errno)
		panic(__FILE__, __LINE__);
	token->next = NULL;
	return (token);
}

// need slight changes if this should be callable by multiple threads
void	add_token(t_token_queue *queue, t_token	new_token)
{
	t_token	*token = malloc(sizeof(new_token));
	while (!token)
	{
		int counter = 0;
		token = malloc(sizeof(new_token));
		if (!token)
		{
			if (counter++ > 100)
			{
				fprintf(stderr, "Continious memory problems\n");
				panic(__FILE__, __LINE__);
			}
			usleep(4000);
		}
	}
	errno = 0;
	*token = new_token;
	pthread_mutex_lock(&queue->mutex);
	if (queue->count)
	{
		queue->head->next = token;
	}
	else
	{
		queue->tail = token;
		pthread_cond_signal(&queue->not_empty);
	}
	queue->count += 1;
	pthread_mutex_unlock(&queue->mutex);
	queue->head = token;
	if (errno)
		panic(__FILE__, __LINE__);
}
