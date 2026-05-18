.PHONY: all
all: c.out asm.out

asm.out: relocation.asm
	nasm -fbin -o $@ $<
	chmod +x $@

c.out: relocation
	./$< $@

relocation: relocation.c code.h evil.h

code.h evil.h: %.h: %.bin
	xxd -i -n $* $< | tee $@

code.bin evil.bin: %.bin: %.s
	nasm -f bin -o $@ $^

clean:
	rm -f evil.bin evil.h code.bin code.h relocation c.out asm.out

