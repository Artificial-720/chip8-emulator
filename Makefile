NAME = chip8
CFLAGS = -std=c99 -Wall -Wextra -Werror -g
LIBFLAGS = -lGL -lglfw -lGLEW
SRC_DIR := src

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRC:$(SRC_DIR)/%.c=$(SRC_DIR)/%.o)


all: $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LIBFLAGS)


clean:
	rm $(NAME) src/*.o
