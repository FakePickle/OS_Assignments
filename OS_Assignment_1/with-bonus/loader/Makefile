all: lib_simpleloader.so

#Referred from https://stackoverflow.com/questions/2481269/how-to-make-a-simple-c-makefile
lib_simpleloader.so: loader.c
	gcc -shared -m32 -nostdlib -o lib_simpleloader.so loader.c

clean:
	rm -f lib_simpleloader.so