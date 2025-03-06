run:
	mkdir build
	clang -o build/parent src/parent.c
	clang -o build/child src/child.c
	./build/parent

clean:
	rm -r ./build

all: run clean