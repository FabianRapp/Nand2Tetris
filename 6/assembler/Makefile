
CC = clang
CFLAGS =-D_FILE_OFFSET_BITS=64 -g -O3 -march=native -pthread -fsanitize=thread
#-fsanitize=address 
NAME = assembler

SRC = main.c queue.c

OBJ = $(SRC:%.c=%.o)

#-fprofile-generate -o myprog myprog.c

#-fprofile-use -o myprog myprog.c

all: $(OBJ)
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

run: all
	sync; echo 3 > /proc/sys/vm/drop_caches && \
	./assembler

scheduler_test: clean
	$(CC) -D_FILE_OFFSET_BITS=64 -g -fsanitize=address base_scheduler_test.c queue.c

scheduler_thread: clean
	$(CC) -D_FILE_OFFSET_BITS=64 -g -fsanitize=thread queue.c base_scheduler_test.c

scheduler_fast: clean
	$(CC) -D_FILE_OFFSET_BITS=64 -O3 -march=native queue.c base_scheduler_test.c -o scheduler_fast

asm: clean
	$(CC) -fverbose-asm -Wa,--comment -D_FILE_OFFSET_BITS=64 -O3 -S -march=native base_scheduler_test.c queue.c -o scheduler_fast.asm


scheduler_prof:
	gcc -pg -g -D_FILE_OFFSET_BITS=64 -O3 -march=native -o scheduler_fast base_scheduler_test.c queue.c

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

lsp: clean
	bear -- make

test_file:
	$(CC) -Ofast -D_FILE_OFFSET_BITS=64 make_test_input.c -o gen_input
	./gen_input

clean:
	@rm -f scheduler_fast a.out
	@rm -f $(OBJ)

fclean:
	@rm -f compile_commands.json $(OBJ) $(NAME)

re: fclean all



