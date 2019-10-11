C = gcc
CFLAGS = -Wall -Werror -Wextra

SRC_EM = emitter.c serial_driver.c ll.c utils.c app_layer.c llinterpretation.c
SRC_REC = receptor.c serial_driver.c ll.c utils.c app_layer.c llinterpretation.c
OBJ_EM = $(SRC_EM:.c=.o)
OBJ_REC = $(SRC_REC:.c=.o)
EXEC1 = receptor
EXEC2 = emitter
DEPS = $(patsubst %.c,%.d,$(wildcard *.c))

.PHONY: all clean

all: $(EXEC1) $(EXEC2)

%.o: %.c
	$(C) $(CFLAGS) -MMD -c $< -o $@


$(EXEC1): $(OBJ_REC)
	$(C) $(CFLAGS) -o $@ $(OBJ_REC) $(LBLIBS)

$(EXEC2): $(OBJ_EM)
	$(C) $(CFLAGS) -o $@ $(OBJ_EM) $(LBLIBS)

clean:
	rm -rf $(EXEC1) $(EXEC2) *.o *.d

-include $(DEPS)