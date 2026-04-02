reloc.out: relocation
	./$< $@

relocation: relocation.c code.h evil.h

code.h: code.bin
	xxd -i -n code $< | tee $@

evil.h: evil.bin
	xxd -i -n evil $< | tee $@

code.bin evil.bin: %.bin: %.s
	nasm -f bin -o $@ $^

clean:
	rm -f evil.bin evil.h code.bin code.h relocation reloc.out

