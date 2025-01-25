all: dos.exe

dos.exe: mkpe
	./$^
	chmod 0755 $@

mkpe: mkpe.zig pe.zig dos_code pe_code
	zig build-exe $<

dos_code pe_code: %_code: %_code.asm
	nasm -f bin $^ -o $@

clean:
	rm -f dos_code pe_code mkpe mkpe.o dos.exe
