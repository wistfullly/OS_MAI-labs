run:
	mkdir build
	clang -o build/main src/main.c -fsanitize=thread
	./build/main 32 8

clean:
	rm -r ./build

all: run clean