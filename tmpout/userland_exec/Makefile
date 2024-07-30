all: loader libculexec.so libulexec.so program1 program2

libculexec.so: culexec.c
	gcc -Og -shared -o $@ $<

libulexec.so: ulexec.zig
	zig build-lib -dynamic -lc $<

loader: loader.c
	gcc -Og -o $@ -ldl $<

program2: program2.c
	gcc -static -o $@ $<

program1: program1.o
	ld -o $@ $<

program1.o: program1.s
	as -o $@ $<

clean:
	rm -f loader libulexec.so.o libulexec.so libculexec.so program1.o program1 program2
