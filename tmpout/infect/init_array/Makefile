.PHONY: all
all: program_infected

program_infected: infect_elf program
	./infect_elf ./program

payload.c: payload.s
	as -o payload.o $<
	objcopy -j .text -O binary payload.o payload.bin
	xxd -i -n shellcode payload.bin > $@
	rm payload.o payload.bin

infect_elf: infect_elf.c payload.c
	gcc -Og -ggdb -o $@ $<

program: program.s
	gcc -no-pie -o $@ $<

.PHONY: clean
clean:
	rm -f program_infected payload.c infect_elf program
