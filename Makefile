run:
	gcc -Wextra -Wall main.c -o sinkhole
	./sinkhole

clean:
	rm ./sinkhole