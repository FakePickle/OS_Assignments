all: shell run clean

shell:
	gcc -o shell shell_interactive.c process_scheduler.c -Werror -Wall -Wextra

run:
	./shell 4 500

clean:
	rm -rf shell