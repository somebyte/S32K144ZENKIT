%.o: %.c
	gcc -c $<

all: ttydebug uploader

ttydebug: ttyconfig.o ttydebug.o
	gcc -o $@ $^

uploader: ttyconfig.o uploader.o
	gcc -o $@ $^

clean:
	rm -f *.o

distclean: clean
	rm -f ttydebug uploader

.PHONY: all clean distclean

