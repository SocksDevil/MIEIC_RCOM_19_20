C = gcc
CFLAGS = -Wall -Werror -Wextra

SRC = download.c hostname.c 
OBJ = $(SRC:.c=.o)
EXEC = download
DEPS = $(patsubst %.c,%.d,$(wildcard *.c))

.PHONY: all clean

all: $(EXEC)

%.o: %.c
	$(C) $(CFLAGS) -MMD -c $< -o $@


$(EXEC): $(OBJ)
	$(C) $(CFLAGS) -o $@ $(OBJ) $(LBLIBS)

clean:
	rm -rf $(EXEC) *.o *.d

-include $(DEPS)