; ModuleID = 'cminus'
source_filename = "testcase-1.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  %op13 = add i32 20401, 32303
  %op15 = add i32 %op13, 23956
  %op17 = sitofp i32 %op15 to float
  %op18 = fmul float %op17, 0x3ff3c0c200000000
  %op19 = fmul float %op18, 0x4016f06a20000000
  %op20 = fmul float %op19, 0x4002aa9940000000
  %op21 = fmul float %op20, 0x4011781d80000000
  %op22 = fmul float %op21, 0x401962ac40000000
  %op23 = fptosi float %op22 to i32
  br label %label5
label5:                                                ; preds = %label_entry, %label10
  %op28 = phi i32 [ 0, %label_entry ], [ %op25, %label10 ]
  %op29 = phi i32 [ %op23, %label10 ], [ undef, %label_entry ]
  %op7 = icmp slt i32 %op28, 100000000
  %op8 = zext i1 %op7 to i32
  %op9 = icmp ne i32 %op8, 0
  br i1 %op9, label %label10, label %label26
label10:                                                ; preds = %label5
  %op25 = add i32 %op28, 1
  br label %label5
label26:                                                ; preds = %label5
  call void @output(i32 %op29)
  ret void
}
