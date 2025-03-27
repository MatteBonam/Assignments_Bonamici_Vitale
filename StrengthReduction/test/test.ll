define i32 @multiply_by_15(i32 %x) {
entry:
  %mul = mul i32 %x, 15
  ret i32 %mul
}

define i32 @multiply_by_15_v2(i32 %x) {
entry:
  %mul = mul i32 15, %x
  ret i32 %mul
}


define i32 @divide_by_8(i32 %x) {
entry:
  %div = sdiv i32 %x, 8
  ret i32 %div
}