.PHONY: all
all: program_infected

program_infected: program infect_elf
	./infect_elf $<

program: program.c
	gcc -o $@ $<

infect_elf: infect_elf.c payload.c
	gcc -o $@ $< -lelf

payload.c: payload.s
	as -o payload.o $<
	objcopy -O binary -j .text payload.o payload.bin
	xxd -i -n payload payload.bin > $@
	rm payload.o payload.bin

.PHONY: clean
clean:
	@rm -f payload.c infect_elf program_infected program
