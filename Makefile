NAME = chip8
CFLAGS = -std=c99 -Wall -Wextra
LIBFLAGS = -lGL -lglfw -lGLEW
MAINOBJS = src/main.o src/chip8.o

# all:
	# $(CC) $(CFLAGS) -g -o main main.c chip8.c -lGL -lglfw -lGLEW



all: $(MAINOBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(MAINOBJS) $(LIBFLAGS)


clean:
	rm $(NAME) src/*.o