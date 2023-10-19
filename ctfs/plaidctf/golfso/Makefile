.PHONY: all
all: lib.so

lib.so: compile
	./compile

compile: compile.c payload.c
	gcc $< -o $@

payload.c: payload.s
	as $< -o payload.o
	objcopy -O binary -j .text payload.o payload.bin
	xxd -i -n code payload.bin > $@
	rm -f payload.o payload.bin

.PHONY: clean
clean:
	rm -f lib.so compile payload.c
