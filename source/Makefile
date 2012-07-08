FILES=main.o readfile.o sysfonc.o video.o walkmask.o execute_script.o game.o loadgame.o animation.o character.o debug.o defaultInventory.o dialog.o fade.o fonts.o graph.o hotspot.o inventory.o load.o savegame.o scene.o script.o sound.o spline.o state.o menu.o music.o opengl_sprite.o

game-lv: $(FILES)
	g++ $(FILES) \
		-lvorbisfile -lSDL -lSDL_mixer -lSDL_image -lsmpeg -lGLU \
		-o game-lv

all: game-lv

clean:
	rm -f *.o game-lv

%.o: %.cpp
	g++ -c $< -o $@ -Os \
		-I/usr/include/SDL \
		-I/usr/include/SDL_Mixer \
		-I/usr/include/SDL_Image \
		-I/usr/include/smpeg
