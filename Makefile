DIRS=bin obj

all: places concert achat

places: obj/places.o
	gcc -o bin/places obj/places.o

obj/places.o: src/places.c
	gcc -c src/places.c -o obj/places.o

concert: obj/concert.o
	gcc -o bin/concert obj/concert.o

obj/concert.o: src/concert.c
	gcc -c src/concert.c -o obj/concert.o

achat: obj/achat.o
	gcc -o bin/achat obj/achat.o

obj/achat.o: src/achat.c
	gcc -c src/achat.c -o obj/achat.o

clean:
	rm bin/places bin/concert bin/achat obj/*.o; \
	rmdir $(DIRS)

$(info $(shell mkdir -p $(DIRS)))
