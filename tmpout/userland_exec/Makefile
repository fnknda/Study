all: loader libulexec.so program

libulexec.so: ulexec.zig
	zig build-lib -dynamic -lc $<

loader: loader.c
	gcc -o $@ -ldl $<

program: program.o
	ld -N -o $@ $<

program.o: program.s
	as -o $@ $<

clean:
	rm -f ulexec ulexec.o program program.o
