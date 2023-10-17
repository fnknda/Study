.PHONY: all
all: program_injected

program_injected: inject_elf program
	./inject_elf ./program

payload.c: payload.s
	as -o payload.o $<
	objcopy -j .text -O binary payload.o payload.bin
	xxd -i -n shellcode payload.bin > $@
	rm payload.o payload.bin

inject_elf: inject_elf.c payload.c
	gcc -Og -ggdb -o $@ $<

program: program.s
	gcc -no-pie -o $@ $<

.PHONY: clean
clean:
	rm -f program_injected payload.c inject_elf program
