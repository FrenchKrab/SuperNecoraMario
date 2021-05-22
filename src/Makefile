O_FILES = sdl_helper.o kmath.o level.o entities.o player.o goomba.o block.o bonus.o kds.o input.o resources.o kengine.o render.o

super_cancer_mario : main.o $(O_FILES)
	gcc -o super_cancer_mario main.o $(O_FILES) -lSDL2 -lSDL2_image -lm

level_editor: level_editor.o $(O_FILES)
	gcc -o level_editor level_editor.o $(O_FILES) -lSDL2 -lSDL2_image -lm


main.o: main.c sdl_helper.h entities.h kds.h input.h resources.h level.h
	gcc -o main.o -c main.c -Wall -O -lSDL2 -lSDL2_image -g
level_editor.o: level_editor.c sdl_helper.h entities.h kds.h input.h resources.h level.h
	gcc -o level_editor.o -c level_editor.c -Wall -O -lSDL2 -lSDL2_image -g


sdl_helper.o : sdl_helper.c
	gcc -o sdl_helper.o -c sdl_helper.c -Wall -O -lSDL2 -lSDL2_image
kmath.o: kmath.c
	gcc -o kmath.o -c kmath.c -Wall -O -lSDL2 -lSDL2_image
kds.o: kds.c
	gcc -o kds.o -c kds.c -Wall -O

kengine.o: kengine.c kds.o input.o resources.o entities.o level.o
	gcc -o kengine.o -c kengine.c -Wall -O -lSDL2 -lSDL2_image
render.o: render.c level.o
	gcc -o render.o -c render.c -Wall -O -lSDL2 -lSDL2_image
level.o: level.c kds.o resources.o entities.o
	gcc -o level.o -c level.c -O -lSDL2 -lSDL2_image -Wno-unused-result 
input.o: input.c kds.o
	gcc -o input.o -c input.c -Wall -O -lSDL2 -lSDL2_image
resources.o: resources.c kds.o
	gcc -o resources.o -c resources.c -Wall -O -lSDL2 -lSDL2_image

entities.o: entities.c goomba.o player.o block.o bonus.o kds.o
	gcc -o entities.o -c entities.c -Wall -O -lSDL2 -lSDL2_image -Ientities

player.o: entities/player.c entities.h
	gcc -o player.o -c entities/player.c -Wall -O
goomba.o: entities/goomba.c entities.h
	gcc -o goomba.o -c entities/goomba.c -Wall -O
block.o: entities/block.c entities.h
	gcc -o block.o -c entities/block.c -Wall -O
bonus.o: entities/bonus.c entities.h
	gcc -o bonus.o -c entities/bonus.c -Wall -O


clean:
	rm -rf *.o