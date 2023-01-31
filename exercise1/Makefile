SRC_DIR=./src

all: gol.x

gol.x: $(SRC_DIR)/gol.c $(SRC_DIR)/game.c $(SRC_DIR)/rw.c
	mpicc -fopenmp $(SRC_DIR)/gol.c $(SRC_DIR)/game.c $(SRC_DIR)/rw.c -o gol.x

clean:
	rm -f gol.x