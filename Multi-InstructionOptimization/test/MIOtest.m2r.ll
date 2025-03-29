; ModuleID = 'MIOtest.m2r.bc'
source_filename = "MIOtest.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @MIOoptimization(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 %0, 1
  %4 = sub nsw i32 %3, 1
  %5 = sub nsw i32 %0, 1
  %6 = add nsw i32 %5, 1
  %7 = add nsw i32 %1, 2
  %8 = add nsw i32 %7, -2
  %9 = add nsw i32 %0, %1
  %10 = sub nsw i32 0, %1
  %11 = add nsw i32 %9, %10
  %12 = sub nsw i32 %1, 2
  %13 = sub nsw i32 %12, -2
  %14 = sub nsw i32 %0, %1
  %15 = sub nsw i32 0, %1
  %16 = sub nsw i32 %14, %15
  %17 = mul nsw i32 %0, 2
  %18 = sdiv i32 %17, 2
  %19 = sdiv i32 %0, 2
  %20 = mul nsw i32 %19, 2
  %21 = add nsw i32 %4, %6
  %22 = add nsw i32 %21, %8
  %23 = add nsw i32 %22, %11
  %24 = add nsw i32 %23, %13
  %25 = add nsw i32 %24, %16
  %26 = add nsw i32 %25, %18
  %27 = add nsw i32 %26, %20
  ret i32 %27
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
