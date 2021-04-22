%.o: %.c
	gcc -c $<

all: ttydebug uploader

ttydebug: ttyspeed.o ttydebug.o
	gcc -o $@ $^

uploader: ttyspeed.o uploader.o
	gcc -o $@ $^

clean:
	rm -f *.o

distclean:
	rm -f ttydebug uploader

.PHONY: clean distclean

