llvm: test.cminus
	build/cminusfc -emit-llvm -mem2reg test.cminus
asm: test.ll
	llc -march=riscv64 test.ll -O0 -o llc.s
rv64: test.c
	riscv64-linux-gnu-gcc -S test.c
bin: test.s
	riscv64-linux-gnu-gcc -ggdb test.s build/io.o -o test
emu: bin
	qemu-riscv64  -L /usr/riscv64-linux-gnu/ ./test
