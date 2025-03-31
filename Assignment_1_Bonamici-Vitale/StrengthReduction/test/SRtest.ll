; ModuleID = 'SRtest.c'
source_filename = "SRtest.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @strength_red(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %9 = load i32, ptr %2, align 4
  %10 = mul nsw i32 %9, 16
  store i32 %10, ptr %3, align 4
  %11 = load i32, ptr %2, align 4
  %12 = mul nsw i32 %11, 15
  store i32 %12, ptr %4, align 4
  %13 = load i32, ptr %2, align 4
  %14 = mul nsw i32 %13, 10
  store i32 %14, ptr %5, align 4
  %15 = load i32, ptr %2, align 4
  %16 = mul nsw i32 %15, 47
  store i32 %16, ptr %6, align 4
  %17 = load i32, ptr %2, align 4
  %18 = sdiv i32 %17, 16
  store i32 %18, ptr %7, align 4
  %19 = load i32, ptr %2, align 4
  %20 = sdiv i32 %19, 15
  store i32 %20, ptr %8, align 4
  %21 = load i32, ptr %3, align 4
  %22 = load i32, ptr %4, align 4
  %23 = add nsw i32 %21, %22
  %24 = load i32, ptr %5, align 4
  %25 = add nsw i32 %23, %24
  %26 = load i32, ptr %6, align 4
  %27 = add nsw i32 %25, %26
  %28 = load i32, ptr %7, align 4
  %29 = load i32, ptr %8, align 4
  %30 = add nsw i32 %28, %29
  %31 = add nsw i32 %27, %30
  ret i32 %31
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
