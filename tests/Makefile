COMPILER	= gcc
#COMPILER	= clang
CC 		= ${COMPILER}
LINK 		= ${COMPILER} -o

CFLAGS 	   	+= -g3 -Istubs -I ..

CHECKFLAGS 	= -Wall -Wextra

LDFLAGS 	= -g3 -Wall -Wextra

TESTTARGETS = \
	bin/test_kv_table

all: check_dirs $(TESTTARGETS)

# Rule for test sources
obj/%.o:%.c
	$(CC) -c $(CFLAGS) $(CHECKFLAGS) $< -o $@

# Rule for project sources
obj/%.o:../%.c
	$(CC) -c $(CFLAGS) $(CHECKFLAGS) $< -o $@

bin/test_kv_table: obj/test_kv_table/main.o \
	obj/parse_dmi.o \
	obj/dmi_intel.o
	$(CC) $(LDFLAGS) $(CHECKFLAGS) -o $@ $^

clean:
	@echo "Clean objects and binaries"
	@if [ -d obj ]; then \
	    find obj -name "*.o" -exec $(RM) {} \; 2>&1 || exit 0; \
	fi
	@$(RM) -rf obj
	@$(RM) -rf bin
	@$(RM) .tmp.out

check_dirs:
	@if [ ! -d obj ]; then \
	    echo "Create obj directories" ; \
	    mkdir -p bin obj/test_kv_table ; \
	fi

tests: $(TESTTARGETS)
	@for t in $(TESTTARGETS); do \
		echo -n "Running test $$t..."; \
		./$$t 2>/dev/null > .tmp.out; \
		if [ $$? != 0 ]; then echo !!!!failed!!!!; \
		else \
		    grep -i "failed" .tmp.out; \
		    if [ $$? != 0 ]; then echo succeeded; \
		    else echo !!!!failed!!!!; \
		fi fi \
	done
