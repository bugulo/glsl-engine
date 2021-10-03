all: clean build

build:
	mkdir bin
	g++ source/main.cpp -g -o bin/engine

clean: 
	rm -rf bin