; ModuleID = 'cminus'
source_filename = "testcase-2.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  %op10 = fmul float 0x3ff0000000000000, 0x4002666660000000
  %op11 = fmul float %op10, 0x3ff6666660000000
  %op12 = fptosi float %op11 to i32
  %op13 = sitofp i32 3 to float
  %op14 = fmul float %op13, 0x3ff0000000000000
  %op15 = fptosi float %op14 to i32
  %op18 = add i32 %op12, %op15
  %op19 = add i32 %op18, 123
  %op21 = mul i32 %op19, %op12
  %op23 = sdiv i32 %op21, %op12
  %op25 = mul i32 %op23, %op12
  %op27 = sdiv i32 %op25, %op12
  %op29 = mul i32 %op27, %op12
  %op31 = sdiv i32 %op29, %op12
  %op33 = mul i32 %op31, %op12
  br label %label2
label2:                                                ; preds = %label_entry, %label7
  %op38 = phi i32 [ %op15, %label7 ], [ undef, %label_entry ]
  %op39 = phi i32 [ %op12, %label7 ], [ undef, %label_entry ]
  %op40 = phi i32 [ 0, %label_entry ], [ %op35, %label7 ]
  %op41 = phi i32 [ 0, %label_entry ], [ %op33, %label7 ]
  %op4 = icmp slt i32 %op40, 100000000
  %op5 = zext i1 %op4 to i32
  %op6 = icmp ne i32 %op5, 0
  br i1 %op6, label %label7, label %label36
label7:                                                ; preds = %label2
  %op35 = add i32 %op40, 1
  br label %label2
label36:                                                ; preds = %label2
  call void @output(i32 %op41)
  ret void
}
