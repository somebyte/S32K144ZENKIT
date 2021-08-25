%.o: %.c
	${CC} -c $< 

all: ttydebug uploader

ttydebug: ttyconfig.o ttydebug.o
	${CC} -o $@ $^

uploader: ttyconfig.o uploader.o
	${CC} -o $@ $^

clean:
	rm -f *.o

distclean: clean
	rm -f ttydebug uploader

.PHONY: all clean distclean

