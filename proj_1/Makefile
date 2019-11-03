C = gcc
CFLAGS = -Wall -Werror -Wextra

SRC = main.c serial_driver.c ll.c utils.c app_layer.c llinterpretation.c data_frame.c disconnect_frame.c statistics.c
OBJ = $(SRC:.c=.o)
EXEC = main
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