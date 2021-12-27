; ModuleID = 'cminus'
source_filename = "testcase-3.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  %op13 = icmp sgt i32 1, 2
  %op14 = zext i1 %op13 to i32
  %op17 = mul i32 1, 2
  %op19 = mul i32 %op17, 3
  %op20 = mul i32 %op19, 123
  %op21 = sitofp i32 %op20 to float
  %op22 = fmul float %op21, 0x3ff3c0c200000000
  %op23 = fmul float %op22, 0x4016f06a20000000
  %op24 = fmul float %op23, 0x4002aa9940000000
  %op25 = fmul float %op24, 0x4011781d80000000
  %op26 = fmul float %op25, 0x401962ac40000000
  %op27 = sitofp i32 %op14 to float
  %op28 = fcmp ult float %op27,%op26
  %op29 = zext i1 %op28 to i32
  br label %label5
label5:                                                ; preds = %label_entry, %label10
  %op34 = phi i32 [ 0, %label_entry ], [ %op31, %label10 ]
  %op35 = phi i32 [ %op29, %label10 ], [ undef, %label_entry ]
  %op7 = icmp slt i32 %op34, 100000000
  %op8 = zext i1 %op7 to i32
  %op9 = icmp ne i32 %op8, 0
  br i1 %op9, label %label10, label %label32
label10:                                                ; preds = %label5
  %op31 = add i32 %op34, 1
  br label %label5
label32:                                                ; preds = %label5
  call void @output(i32 %op35)
  ret void
}
