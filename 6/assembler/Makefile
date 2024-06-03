
CC = clang
CFLAGS =-D_FILE_OFFSET_BITS=64 -g -O3 -march=native -pthread -fsanitize=thread
#-fsanitize=address 
NAME = assembler

SRC = main.c

OBJ = $(SRC:%.c=%.o)


all: $(OBJ)
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

run: all
	sync; echo 3 > /proc/sys/vm/drop_caches && \
	./assembler

scheduler_test:
	$(CC) -D_FILE_OFFSET_BITS=64 -g -fsanitize=address base_scheduler_test.c

scheduler_thread:
	$(CC) -D_FILE_OFFSET_BITS=64 -g -fsanitize=thread base_scheduler_test.c

scheduler_fast:
	$(CC) -D_FILE_OFFSET_BITS=64 -O3 -march=native base_scheduler_test.c

scheduler_prof:
	gcc -pg -g -D_FILE_OFFSET_BITS=64 -O3 -march=native -o scheduler_fast base_scheduler_test.c

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



