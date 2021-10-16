define i32 @main () {
    %1 = alloca i32 ; 好像忘记用这个了，不过删掉还要把数字都全改一遍，好麻烦
    %2 = alloca i32 ; a
    %3 = alloca i32 ; i
    store i32 10, i32* %2
    store i32 0, i32* %3
    %4 = load i32, i32* %3 ; 哎呀，这个也是
    %5 = load i32, i32* %2
    br label %6
6:
    %7 = load i32, i32* %3
    %8 = load i32, i32* %2
    %9 = add i32 %7, 1
    store i32 %9, i32* %3
    %10 = add i32 %8, %9
    store i32 %10, i32* %2
    %11 = icmp slt i32 %9, 10
    br i1 %11, label %6, label %12
12:
    ret i32 %10
}
