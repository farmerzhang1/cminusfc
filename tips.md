# 一些 tips
## 如何 debug
1. 安装cmake tools
2. 选择下方一栏的target
3. 如果需要设置命令行参数，在[workspace的设置](.vscode/settings.json)中加入一行
    ```json
    "cmake.debugConfig": {
        "args": ["arg1", "arg2"]
    }
    ```
4. 设置断点，点击下方小虫子，launch debugger
## 观察修改了啥
打开侧边栏，展开最底下的timeline
## opt
run
```
clang -S -emit-llvm -O -Xclang -disable-llvm-passes <source>
opt <your passes> <source.ll> -S -o <output-file>
```
比如
```
clang -S -emit-llvm -O -Xclang -disable-llvm-passes test.c
opt -mem2reg -sccp test.ll -S -o result.ll
```
## RV64
终于！
```
riscv64-linux-gnu-gcc -c -o io.o io.c
build/cmi ...
llc -march=riscv64 test.ll
riscv64-linux-gnu-gcc test.s io.o -o test
qemu-riscv64  -L /usr/riscv64-linux-gnu/ ./test
```
