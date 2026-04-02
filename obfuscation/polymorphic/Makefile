CC = clang
CFLAGS = -O2 -Wall -Wextra -Wpedantic

poly: compile
	./$<

compile: compile.c staging.h payload.h
	$(CC) $(CFLAGS) -o $@ $<

payload.h staging.h: %.h: %.bin
	xxd -i -n$* $< $@
	sed -i 's/^unsigned int \($*_len\) = \([0-9]*\);$$/#define \1 \2/' $@

payload.o staging.o: %.o: %.s
	nasm -felf64 -o $@ $<

payload.bin staging.bin: %.bin: %.s
	nasm -fbin -o $@ $<

.PHONY: clean
clean:
	rm -f payload.bin staging.bin payload.h staging.h payload.o staging.o payload staging compile poly
