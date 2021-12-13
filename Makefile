llvm: test.c
	build/cminusfc -emit-llvm -mem2reg test.c
asm: llvm
	llc -march=riscv64 test.ll -O0
bin: asm
	riscv64-linux-gnu-gcc test.s build/io.o -o test
emu: bin
	qemu-riscv64  -L /usr/riscv64-linux-gnu/ ./test
