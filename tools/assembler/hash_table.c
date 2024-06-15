#include "assembler.h"
#include "simple_assembler.h"

size_t	hash_str(char *str)
{
    int hash_value = 0;
    for (int i = 0; str[i]; i++)
    {
        hash_value = (hash_value * HASH_PRIME + str[i]) % TABLE_SIZE;
        //hash_value = ((hash_value + arr[i]));
    }
    return (hash_value % TABLE_SIZE);
}

void	*make_symbole(char *name, uint16_t val)
{
	t_symbole	*symbole = calloc(1, sizeof(t_symbole));
	assert(symbole);
	symbole->name = strdup(name);
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
	//map index is empty(no collision and not defined symbole)
	if (!(cur_cmp->val))
	{
		cur_cmp->val = make_symbole(name, val);
		//write(1, "adding: ", 8);
		//write(1, name, strlen(name));
		return (val);
	}
	//collision and early defintion handeling
	while (cur_cmp->next && strcmp(((t_symbole *)(cur_cmp->val))->name, name))
	{
		cur_cmp = cur_cmp->next;
	}
	//symbol is allready in map
	if (!strcmp(((t_symbole *)(cur_cmp->val))->name, name))
	{
		if (is_declartion)
		{
			((t_symbole *)(cur_cmp->val))->val = val;
			return (val);
		}
		*name = ' ';
		//write(1, "returning: ", strlen("returning: "));
		//write(1, ((t_symbole *)(cur_cmp->val))->name, strlen(((t_symbole *)(cur_cmp->val))->name));
		return (((t_symbole *)(cur_cmp->val))->val);
	}
	//finally: there was a collision and symbole was not yet in map
	cur_cmp->next = calloc(1, sizeof(t_list_node));
	assert(cur_cmp->next);
	cur_cmp->next->val = make_symbole(name, val);
	//	write(1, "adding: ", 8);
	//	write(1, name, strlen(name));
	return (val);
}

