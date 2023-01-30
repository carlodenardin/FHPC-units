# Game of Life - Hybrid MPI and OMP implementation

## How to compile
To compile the executable
```
make all
```

To cancel the executable:
```
make clean
```


## How to inizialize the game

| Arguments | Description | Default |
| ------- | --- | --- |
| -i | initialize a random pmg image | required|
| -k (number) | dimension of the random square image | 1000 |
| -f (name)| name of the random pmg image  |  required|

```
mpi -np 1 gol.x -i -k 100 -f pattern_random
```

This code will generate the `pattern_random.pmg` image which as a dimension `100x100` in the main folder. The number of processes is indifferent since the generation will be executed in a serial way.

## How to run the game

| Arguments | Description | Default |
| ------- | --- | --- |
| -r | run the game | required |
| -f (name) | name of the random pmg image  |  required |
| -n (number) | number of evolution to perform  | 100 | 
| -e (0, 1, 2) | types of evolution (0: ordered, 1: static, 2: BW static) | 1: static |
| -s (number) | how many evolutions save the image | 0: only at the end |

### Run 1:
```
mpi -np 4 gol.x -r -f pattern_random -n 100 -e 1 -s 0
```

This code will perform the game evolutions (`static evolution`), for the input `pattern_random.pgm`, for `100 steps` and it will save only the last evolution state.

### Run 2:
```
mpi -np 4 gol.x -r -f pattern_random -n 1000 -e 1 -s 1
```

This code will perform the game evolutions (`static evolution`), for the input `pattern_random.pgm`, for `1000 steps` and it will save an image of the state for each step.


## Examples of common patterns

| Input | Result |
| ------- | --- |
| pattern_blinker | [Blinker GIF](https://imgur.com/ndWD6tX) |
| pattern_pulsare | [Pulsar GIF](https://imgur.com/9OQdXyP) |
| pattern_ship | name of the random pmg image  |

## Code details
The source code is divided among 3 different files:
- [gol.c](https://github.com/carlodenardin/FHPC-units/blob/main/exercise1/src/gol.c): main file that manages everything
- [game.c](https://github.com/carlodenardin/FHPC-units/blob/main/exercise1/src/game.c): functions that are used to provide the evolutions of the game. The header file [game.h](https://github.com/carlodenardin/FHPC-units/blob/main/exercise1/src/game.h) contains the documentations of the functions
- [rw.c](https://github.com/carlodenardin/FHPC-units/blob/main/exercise1/src/rw.c): functions that are used to read and write pgm files. The header file [rw.h](https://github.com/carlodenardin/FHPC-units/blob/main/exercise1/src/rw.h) contains the documentations of the functions

