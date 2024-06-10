#include "../assembler.h"
#include "simple_assembler.h"


void	append_node(t_list_node	**list, void *new_data)
{
	t_list_node	*new = malloc(sizeof(t_list_node));
	new->next = NULL;
	new->val = new_data;
	if (!*list)
	{
		*list = new;
		return ;
	}
	t_list_node	*cur = *list;
	while (cur->next)
	{
		cur = cur->next;
	}
	cur->next = new;
}
