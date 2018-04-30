# 2018 SP Project2
- Author : 20131575 유승재

## Program Purpose
The purpose of this project is to make SIC/XE machine. (virtual version)

## How to use...
1. Make execution file
```
git/SP/project1$ make all
gcc -Wall	-g   -c -o 20131575.o 20131575.c
gcc -o 20131575.out -Wall	-g 20131575.o
```

2. Run execution file
```
git/SP/project1$ ./20131575.out
sicsim>
```
3. Clean object file
```
git/SP/project1$ make clean
rm -f 20131575.o 20131575.out
```
