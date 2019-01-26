DIRS=bin obj

all: twd twc

twd: obj/twd.o
	gcc -o bin/twd obj/twd.o

obj/twd.o: src/twd.c
	gcc -c src/twd.c -o obj/twd.o

twc: obj/twc.o
	gcc -o bin/twc obj/twc.o

obj/twc.o: src/twc.c
	gcc -c src/twc.c -o obj/twc.o

clean:
	rm bin/twd bin/twc obj/*.o; \
	rmdir $(DIRS)

$(info $(shell mkdir -p $(DIRS)))
