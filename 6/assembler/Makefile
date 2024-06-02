
CC = clang
CFLAGS =-D_FILE_OFFSET_BITS=64 -Ofast -march=native
#-fsanitize=address 
NAME = assembler

SRC = main.c

OBJ = $(SRC:%.c=%.o)


all: $(OBJ)
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

run: all
	sync; echo 3 > /proc/sys/vm/drop_caches && \
	./assembler

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

lsp: clean
	bear -- make

test_file:
	$(CC) -Ofast -D_FILE_OFFSET_BITS=64 make_test_input.c -o gen_input
	./gen_input

clean:
	@rm compile_commands.json
	@rm $(OBJ)

fclean:
	@rm $(OBJ) $(NAME)

re: fclean all



