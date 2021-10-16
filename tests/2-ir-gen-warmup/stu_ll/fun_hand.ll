define i32 @callee (i32 %a) {
    %r = mul i32 2, %a
    ret i32 %r
}
define i32 @main () {
    %r = call i32 @callee(i32 110)
    ret i32 %r
}
