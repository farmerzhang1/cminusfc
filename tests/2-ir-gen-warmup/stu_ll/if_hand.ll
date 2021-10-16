define i32 @main() {
    %1 = alloca i32 ; return
    %2 = alloca float ; float a
    store float 0x40163851E0000000, float* %2 ;??? floating??
    %3 = load float, float* %2
    %4 = fcmp ogt float %3, 1.0
    br i1 %4, label %5, label %6
5:
    store i32 233, i32* %1
    br label %7
6:
    store i32 0, i32* %1
    br label %7
7:
    %8 = load i32, i32* %1
    ret i32 %8
}
