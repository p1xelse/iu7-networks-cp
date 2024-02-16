.PHONY: build clean

build: clean
	gcc -o ./app -I./server/include ./server/src/*.c -pthread -Wpedantic 

clean:
	rm -rf ./app 