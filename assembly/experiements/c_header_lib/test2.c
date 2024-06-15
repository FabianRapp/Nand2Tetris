#include <stdio.h>

int test(void *ptr)
{
    goto ptr;
}


int main(void)
{
    test(b);
a:
    printf("a\n");
    goto end;
b:
    printf("b\n");
    goto end;
end:
    return(0);
}

