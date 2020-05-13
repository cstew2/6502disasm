TARGET          = disasm

CC              = gcc

CFLAGS          := -std=c11 -I./ 

DCFLAGS         := -g -ggdb3 -O0 -Wall -pedantic -Wextra -Wundef -Wshadow \
                  -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wwrite-strings \
                  -Wswitch-default -Wswitch-enum \
                  -Wunreachable-code -Winit-self

RCFLAGS         := -O2 -g -fwhole-program \
		  -Wall -pedantic -Wextra -Wundef -Wshadow \
                  -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wwrite-strings \
                  -Wswitch-default -Wswitch-enum \
                  -Wunreachable-code -Winit-self

LDFLAGS		:= 

SRC		:= $(wildcard *.c)
INC		:= $(wildcard *.h)
OBJ             := $(patsubst %.c, %.o, $(filter %.c,$(SRC)))

LIBS		:= -lm

.PHONY: debug
debug: CFLAGS += $(DCFLAGS)
debug: build

.PHONY: release
release: CFLAGS += $(RCFLAGS)
release: build


.PHONY: build
build: $(OBJ)
	@$(CC) $(LDFLAGS) $(OBJ) -o $(TARGET) $(LIBS)
	@echo [LD] Linked $^ into binary $(TARGET)

%.o:%.c
	@$(CC) $(CFLAGS) -c $^ -o $@
	@echo [CC] Compiled $^ into $@

.PHONY: clean
clean:
	@rm -f $(OBJ) $(TARGET) 
	@echo Cleaned $(OBJ) and $(TARGET)
