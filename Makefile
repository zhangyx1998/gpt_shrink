bin: $(patsubst %.c,build/%.o,$(wildcard *.c))
	$(CC) $(CFLAGS) $^ -o $@

build/%.o: %.c $(wildcard *.h)
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f build
