.PHONY: all
all: program_injected

program_injected: program inject_elf
	./inject_elf $<

program: program.c
	gcc -o $@ $<

inject_elf: inject_elf.c payload.c
	gcc -o $@ $< -lelf

payload.c: payload.s
	as -o payload.o $<
	objcopy -O binary -j .text payload.o payload.bin
	xxd -i -n payload payload.bin > $@
	rm payload.o payload.bin

.PHONY: clean
clean:
	@rm -f payload.c inject_elf program_injected program
