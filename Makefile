all:
	g++ -I src/include -L src/lib -o Pokemon Pokemon.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
