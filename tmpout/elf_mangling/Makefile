code: compile
	./compile

compile: compile.c code.c
	gcc -o $@ $<

code.c: code.s
	as $< -o code.o
	objcopy -j .text -O binary code.o code.bin
	xxd -i -n code code.bin > $@
	rm code.o code.bin

.PHONY: clean
clean:
	rm -f code compile code.c
