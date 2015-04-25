all:	./engine/*.c ./app/*.c ./include/*.h
	gcc -Wall ./engine/*.c ./app/*.c -o demo -lGL -lglut -lGLU -lm -lpng -I./include/.

run:	all
	./demo
