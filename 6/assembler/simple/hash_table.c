#include "../assembler.h"
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
   // return (hash_value);
}
