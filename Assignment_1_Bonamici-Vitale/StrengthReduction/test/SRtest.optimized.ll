; ModuleID = 'SRtest.optimized.bc'
source_filename = "SRtest.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @strength_red(i32 noundef %0) #0 {
  %2 = mul nsw i32 %0, 16
  %shiftLeft = shl i32 %0, 4
  %3 = mul nsw i32 %0, 15
  %shiftP = shl i32 %0, 4
  %sub = sub i32 %shiftP, %0
  %4 = mul nsw i32 %0, 10
  %shiftQ = shl i32 %0, 1
  %shiftP1 = shl i32 %0, 3
  %add = add i32 %shiftP1, %shiftQ
  %5 = mul nsw i32 %0, 47
  %6 = sdiv i32 %0, 16
  %shiftRight = lshr i32 %0, 4
  %7 = sdiv i32 %0, 15
  %8 = add nsw i32 %shiftLeft, %sub
  %9 = add nsw i32 %8, %add
  %10 = add nsw i32 %9, %5
  %11 = add nsw i32 %shiftRight, %7
  %12 = add nsw i32 %10, %11
  ret i32 %12
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 19.1.1 (1ubuntu1~24.04.2)"}
