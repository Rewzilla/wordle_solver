
all:
	gcc wordle.c -o wordle -g -fopenmp

clean:
	rm wordle
