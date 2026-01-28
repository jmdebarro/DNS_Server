run:
	gcc -Wextra -Wall main.c -o sinkhole hashmap.c
	./sinkhole

build:
	gcc -Wextra -Wall main.c -o sinkhole hashmap.c

clean:
	rm ./
