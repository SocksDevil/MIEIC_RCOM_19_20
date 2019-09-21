C = gcc
CFLAGS = -Wall -Werror -Wextra

SRC = main.c
OBJ = $(SRC:.c=.o)
EXEC = proj1
DEPS = $(patsubst %.c,%.d,$(wildcard *.c))

.PHONY: all clean

all: $(EXEC)

%.o: %.c
	$(C) $(CFLAGS) -MMD -c $< -o $@


$(EXEC): $(OBJ)
	$(C) $(CFLAGS) -o $@ $(OBJ) $(LBLIBS)

clean:
	rm -rf $(EXEC1) $(EXEC2) *.o *.d

-include $(DEPS)