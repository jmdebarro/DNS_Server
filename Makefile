run:
	gcc -Wextra -Wall main.c -o sinkhole hashmap.c
	./sinkhole

build:
	gcc -Wextra -Wall main.c -o sinkhole hashmap.c

clean:
	rm ./sinkhole

testing:
	gcc -Wextra -Wall test.c -o testing hashmap.c
	./testing
