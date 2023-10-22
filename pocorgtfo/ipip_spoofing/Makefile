check_ipip: check_ipip.c
	gcc -o $@ $<
	sudo setcap cap_net_raw+ep $@

.PHONY: clean
clean:
	rm -f check_ipip
