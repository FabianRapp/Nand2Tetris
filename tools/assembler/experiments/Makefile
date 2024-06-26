
CC = clang
CFLAGS =-D_FILE_OFFSET_BITS=64 -g -O3 -pthread -march=native
#-fsanitize=address
#-fsanitize=address 
NAME = assembler

BASE_SRC = queue.c ../debug_utils.c \
			c_instruction_handler.c

SRC = $(BASE_SRC) main.c
BASE_OBJS = $(BASE_SRC:%.c=%.o)
OBJ = $(SRC:%.c=%.o)

#-fprofile-generate -o myprog myprog.c

#-fprofile-use -o myprog myprog.c

all: $(OBJ)
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

run: all
	sync; echo 3 > /proc/sys/vm/drop_caches && \
	./assembler

test_instruction: $(BASE_OBJS)
	$(CC) $(CFLAGS) $(BASE_SRC) test_instruction.c -o instruction_test

test_instruction_fast: $(BASE_OBJS)
	$(CC) $(CFLAGS) -pg -fprofile-use -DNDEBUG=1 $(BASE_SRC) test_instruction.c -o instruction_test_fast

llvm_profdata_mege:
	llvm-profdata merge -sparse default_*.profraw -o default.profdata

test_instruction_fast_gen: $(BASE_OBJS)
	$(CC) $(CFLAGS) -fprofile-generate -DNDEBUG=1 $(BASE_SRC) test_instruction.c -o instruction_test_fast_gen
src:
	clang $(CFLAGS) $(SRC) -o $(NAME)

prof:
	gcc -pg -g -D_FILE_OFFSET_BITS=64 -O3 -march=native $(SRC) -o $(NAME) -DNDEBUG=1

set_mem_limit:
	ulimit -v 14680064
	@echo set this terminals max memory to about 24GB

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

test_file:
	$(CC) -Ofast -D_FILE_OFFSET_BITS=64 make_test_input.c -o gen_input
	./gen_input

clean:
	@rm -f scheduler_fast a.out
	@rm -f $(OBJ)

fclean:
	@rm -f compile_commands.json $(OBJ) $(NAME)

re: fclean all



