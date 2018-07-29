sample2D: game.cpp glad.c
	g++ -g -o sample2D game.cpp glad.c -lpthread -lGL -lglfw -ldl -lao

clean:
	rm sample2D
