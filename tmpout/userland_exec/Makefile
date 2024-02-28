all: ulexec program

ulexec: ulexec.c
	gcc -o $@ $^

program: program.c
	gcc -o $@ $^

clean:
	rm -f ulexec program
