all: swim_mill fish pellet

swim_mill: swim_mill.c
	gcc -pthread -o swim_mill swim_mill.c -I.

fish: fish.c
	gcc -pthread -o fish fish.c -I.

pellet: pellet.c
	gcc -pthread -o pellet pellet.c -I.