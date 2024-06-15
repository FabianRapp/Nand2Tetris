#include "assembler.h"
#include "simple_assembler.h"

size_t	hash_str(char *str)
{
    int hash_value = 0;
    for (int i = 0; str[i]; i++)
    {
        hash_value = (hash_value * HASH_PRIME + str[i]) % TABLE_SIZE;
    }
    return (hash_value % TABLE_SIZE);
}

void	*make_symbole(char *name, uint16_t val)
{
	t_symbole	*symbole = calloc(1, sizeof(t_symbole));
	if (!symbole)
		panic(__FILE__, __LINE__);
	symbole->name = strdup(name);
	if (!symbole->name)
		panic(__FILE__, __LINE__);
	symbole->val = val;
	return (symbole);
}

/*
*  hack: if no value is added to the map the first char of name is set to ' '
* - if is_declartion is true the value is always added, even if it might overwrite
* an exitisting value
* - otherwise if the name is allready in the map it is not added again
* - returns the value of the node with the given name
*/
int	add_symbole(t_symbole_table *table, char *name, int val,
			 bool is_declartion)
{
	assume(name && *name);
	t_symbole	symbole = {.val = val, .name = strdup(name)};
	size_t		index = hash_str(name);
	t_list_node	*cur_cmp = table->map + index;
	if (!(cur_cmp->val))
	{
		//map index is empty(no collision and not defined symbole)
		cur_cmp->val = make_symbole(name, val);
		return (val);
	}
	while (cur_cmp->next && strcmp(((t_symbole *)(cur_cmp->val))->name, name))
	{
		//collision
		cur_cmp = cur_cmp->next;
	}
	if (!strcmp(((t_symbole *)(cur_cmp->val))->name, name))
	{
		//symbol is allready in map
		if (is_declartion)
		{
			((t_symbole *)(cur_cmp->val))->val = val;
			return (val);
		}
		*name = ' ';
		return (((t_symbole *)(cur_cmp->val))->val);
	}
	//finally: there was a collision and symbole was not yet in map
	cur_cmp->next = calloc(1, sizeof(t_list_node));
	if (!cur_cmp->next)
		panic(__FILE__, __LINE__);
	cur_cmp->next->val = make_symbole(name, val);
	return (val);
}

