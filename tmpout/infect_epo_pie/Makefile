.PHONY: all
all: program_infected

program_infected: infect program
	./infect program

infect: infect.c payload.c
	gcc -o $@ $<

program: program.c
	gcc -Og -ggdb -o $@ $<

payload.c: payload.s
	as -o payload.o $<
	objcopy -j .text -O binary payload.o payload.bin
	xxd -n shellcode -i payload.bin > $@
	rm payload.o payload.bin

.PHONY: clean
clean:
	rm -f program_infected infect program payload.c
