#!/bin/bash

./main_googleCells Inputs/ B hyper-Ts4-Tb256-N 2048 exp 30000000 40
./main_googleCells Inputs/ B hyper-Ts8-Tb256-N 2048 exp 30000000 40
./main_googleCells Inputs/ B hyper-Ts16-Tb512-N 2048 exp 30000000 40
./main_googleCells Inputs/ B hyper-Ts32-Tb1024-N 2048 exp 30000000 40
./main_googleCells Inputs/ B hyper-UpperPowOfTwo-N 2048 exp 30000000 40
./main_googleCells Inputs/ B hyper-LowerPowOfTwo-N 2048 exp 30000000 40
./main_googleCells Inputs/ B VarServiceTimes-N 2048 exp 30000000 40
./main_googleCells Inputs/ B Sorted_ 2048 exp 30000000 40
./main_googleCells Inputs/ B Sorted_ 2048 bpar 100000000 60
./main_googleCells Inputs/ B Sorted_ 2048 det 30000000 40
./main_googleCells Inputs/ B Sorted_ 2048 uni 30000000 40

./main_googleCells Inputs/ A hyper-Ts4-Tb256-N 3072 exp 30000000 40
./main_googleCells Inputs/ A hyper-Ts8-Tb256-N 3072 exp 30000000 40
./main_googleCells Inputs/ A hyper-Ts16-Tb512-N 3072 exp 30000000 40
./main_googleCells Inputs/ A hyper-Ts32-Tb1024-N 3072 exp 30000000 40
./main_googleCells Inputs/ A hyper-UpperPowOfTwo-N 3072 exp 30000000 40
./main_googleCells Inputs/ A hyper-LowerPowOfTwo-N 3072 exp 30000000 40
./main_googleCells Inputs/ A Sorted_ 3072 exp 30000000 40
./main_googleCells Inputs/ A Sorted_ 3072 bpar 100000000 60
./main_googleCells Inputs/ A Sorted_ 3072 det 30000000 40
./main_googleCells Inputs/ A Sorted_ 3072 uni 30000000 40
