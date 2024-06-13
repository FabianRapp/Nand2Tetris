
CC = clang
CFLAGS = -g -Wextra -fsanitize=address
SRC = main.c hash_table.c init.c list_utils.c ../debug_utils.c
OBJ= $(SRC:%.c=%.o)

NAME=assembler

all: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)


clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

