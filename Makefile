DIRS=bin obj

all: places concert achat

places: obj/places.o obj/common.o
	gcc -o bin/places obj/places.o obj/common.o

obj/places.o: src/places.c src/common.h
	gcc -c src/places.c -o obj/places.o

concert: obj/concert.o obj/common.o
	gcc -o bin/concert obj/concert.o obj/common.o

obj/concert.o: src/concert.c src/common.h
	gcc -c src/concert.c -o obj/concert.o

achat: obj/achat.o obj/common.o
	gcc -o bin/achat obj/achat.o obj/common.o

obj/achat.o: src/achat.c src/common.h
	gcc -c src/achat.c -o obj/achat.o

obj/common.o: src/common.c src/common.h
	gcc -c src/common.c -o obj/common.o

clean:
	rm bin/places bin/concert bin/achat obj/*.o; \
	rmdir $(DIRS)

$(info $(shell mkdir -p $(DIRS)))
