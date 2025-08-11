reloc.out: relocation
	./$<

relocation: relocation.c code.h

code.h: code.bin
	xxd -i -n code $< | tee $@

code.bin: code.out
	objcopy -j .text -O binary $< $@

code.out: code.s
	nasm -f elf64 -o $@ $^

clean:
	rm -f code.out code.bin code.h relocation reloc.out

