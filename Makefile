player: main.cpp src/save_audio.cpp src/sound_data.cpp
	g++ -o player main.cpp src/* -I/usr/include -I. -D_REENTRANT -lSDL2
