
OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o,$(wildcard *.c))

.PHONY: all clean

all: $(OBJ)

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
