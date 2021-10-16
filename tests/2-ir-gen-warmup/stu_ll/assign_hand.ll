define i32 @main () {
    %a = alloca[10 x i32]
    %1 = getelementptr [10 x i32], [10 x i32]* %a, i64 0, i64 0 ; start from 1
    store i32 10, i32* %1
    %2 = getelementptr [10 x i32], [10 x i32]* %a, i64 0, i64 1
    %3 = load i32, i32* %1
    %4 = mul i32 %3, 2
    store i32 %4, i32* %2
    %r = load i32, i32* %2
    ret i32 %r
}