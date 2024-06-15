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

// thread save
t_token	*get_token(t_token_queue *queue)
{
	pthread_mutex_lock(&queue->mutex);
	while (queue->count == 0)
	{
		pthread_cond_wait(&queue->not_empty, &queue->mutex);
	}
	t_token	*token = queue->tail;
	if (queue->tail->next)
		queue->tail = queue->tail->next;
	else
	{
		//queue->head = NULL;
		queue->tail = NULL;
	}
	queue->count -= 1;
	pthread_mutex_unlock(&queue->mutex);
	return (token);
}

// thread safe if this is only called by a single thread (taking from
// the list does not matter)
// to enable fully thread safty queue->head needs to be moved into the mutex
void	add_token(t_token_queue *queue, t_token	new_token)
{
	t_token	*token = malloc(sizeof new_token);
	int	malloc_fails = 0;
	while (!token)
	{
		if (malloc_fails++ > 100)
		{
			fprintf(stderr, "Continuous memory problems!\n");
			fprintf(stderr, "Try reduce the memory usage with 'ulimit -v' and"
				" rerun the programm\n");
			panic(__FILE__, __LINE__);
		}
		usleep(4000);
		token = malloc(sizeof new_token);
	}
	*token = new_token;
	pthread_mutex_lock(&queue->mutex);
	if (queue->count)
	{
		assume(queue->head && queue->tail);
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
}

