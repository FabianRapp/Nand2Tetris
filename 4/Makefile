
cc = CC
SRC_BASE = test.c
SRC = tmp
OBJ = $(SRC:.c=.o)

all: transpile
	@echo "done"

init:
	@cat $(SRC_BASE) > tmp_init

trim_whitespace: init
	@echo "trim_whitespace.."
	@sed -e 's/^[[:space:]]*//g' tmp_init > tmp0;\
	sed -e 's/[[:space:]]*$//g' tmp0 > tmp; rm tmp0

#@sed 's/^[ \t]*//' tmp_init > tmp0; rm tmp_init
#@sed 's/^*[ \t]//' tmp0 > tmp;rm tmp0


var_count = 100
transpile: trim_whitespace
	@echo "Transpiling.."
	@var_count=100; awk -v count=$$var_count '/^\(.*\)/ {gsub(/[()]/, "", $$0); printf("#define %s %d\n", $$0, count++); next} {print}' tmp > tmp2; rm tmp












