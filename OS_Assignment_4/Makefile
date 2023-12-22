#invoke make inside following directories and in this order: loader, launch, fib
all: load launch fib

#Referred from https://stackoverflow.com/questions/2481269/how-to-make-a-simple-c-makefile
load:
	cd loader && make
	cp loader/lib_simpleloader.so bin/

launch:
	cd launcher && make
	cp launcher/launch bin/

fib:
	cd test && make
	cp test/fib test/sum bin/

run:
	sudo chmod +x bin/launch bin/fib bin/sum
	cd bin && ./launch fib
	cd bin && ./launch sum

#provide the command for cleanup
clean:
	cd loader && make clean
	cd launcher && make clean
	cd test && make clean
	rm -r bin/lib_simpleloader.so bin/launcher bin/fib