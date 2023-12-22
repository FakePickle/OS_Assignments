all: launcher

#Referred from https://stackoverflow.com/questions/2481269/how-to-make-a-simple-c-makefile
launcher: launch.c ../bin/lib_simpleloader.so
	gcc -m32 -o launch launch.c -L../loader ../bin/lib_simpleloader.so -Wl,-rpath=../loader

clean:
	rm -f launcher fib
