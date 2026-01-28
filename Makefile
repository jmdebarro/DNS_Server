run:
	gcc -Wextra -Wall test.c -o sinkhole hashmap.c
	./sinkhole

clean:
	rm ./
