# Cminusfc
Accompanying project repository for **Compiler Theory** taught at USTC, 2021 fall.
## Labs
* [Lab1](./Documentations/1-parser/)
* [Lab2](./Documentations/2-ir-gen-warmup/)
* [Lab3](./Documentations/3-ir-gen/)
* [Lab4](./Documentations/4-ir-opt)
* [Lab5](./Documentations/5-bonus/)
## Build
This project uses `cmake`
```
mkdir build
cd build
cmake ..
make
```
There should be no need to `make install` the io library used in lab3 and lab4.

- if you use VS Code, you can install the cmake extension `cmake-tools` that facilitates building and debugging.
## Usage
run
```
build/cminusfc
```
to see the help message, note that command line options can be abbreviated (e.g. `-mem2reg` -> `-m2r`)
## Future Works
- There are currently a number of bugs still out there (lab5 and the previous ones)
- Fix memory leak
- CRTP instead of vtables
- <del>Develop a formal semantics</del>