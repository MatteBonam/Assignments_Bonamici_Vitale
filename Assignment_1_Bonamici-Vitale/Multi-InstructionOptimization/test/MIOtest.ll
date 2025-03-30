; ModuleID = 'MIOtest.c'
source_filename = "MIOtest.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @MIOoptimization(i32 noundef %0, i32 noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  %12 = alloca i32, align 4
  %13 = alloca i32, align 4
  %14 = alloca i32, align 4
  %15 = alloca i32, align 4
  %16 = alloca i32, align 4
  %17 = alloca i32, align 4
  %18 = alloca i32, align 4
  %19 = alloca i32, align 4
  %20 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  store i32 %1, ptr %4, align 4
  %21 = load i32, ptr %3, align 4
  %22 = add nsw i32 %21, 1
  store i32 %22, ptr %5, align 4
  %23 = load i32, ptr %5, align 4
  %24 = sub nsw i32 %23, 1
  store i32 %24, ptr %6, align 4
  %25 = load i32, ptr %3, align 4
  %26 = sub nsw i32 %25, 1
  store i32 %26, ptr %7, align 4
  %27 = load i32, ptr %7, align 4
  %28 = add nsw i32 %27, 1
  store i32 %28, ptr %8, align 4
  %29 = load i32, ptr %4, align 4
  %30 = add nsw i32 %29, 2
  store i32 %30, ptr %9, align 4
  %31 = load i32, ptr %9, align 4
  %32 = add nsw i32 %31, -2
  store i32 %32, ptr %10, align 4
  %33 = load i32, ptr %3, align 4
  %34 = load i32, ptr %4, align 4
  %35 = add nsw i32 %33, %34
  store i32 %35, ptr %11, align 4
  %36 = load i32, ptr %11, align 4
  %37 = load i32, ptr %4, align 4
  %38 = sub nsw i32 0, %37
  %39 = add nsw i32 %36, %38
  store i32 %39, ptr %12, align 4
  %40 = load i32, ptr %4, align 4
  %41 = sub nsw i32 %40, 2
  store i32 %41, ptr %13, align 4
  %42 = load i32, ptr %13, align 4
  %43 = sub nsw i32 %42, -2
  store i32 %43, ptr %14, align 4
  %44 = load i32, ptr %3, align 4
  %45 = load i32, ptr %4, align 4
  %46 = sub nsw i32 %44, %45
  store i32 %46, ptr %15, align 4
  %47 = load i32, ptr %15, align 4
  %48 = load i32, ptr %4, align 4
  %49 = sub nsw i32 0, %48
  %50 = sub nsw i32 %47, %49
  store i32 %50, ptr %16, align 4
  %51 = load i32, ptr %3, align 4
  %52 = mul nsw i32 %51, 2
  store i32 %52, ptr %17, align 4
  %53 = load i32, ptr %17, align 4
  %54 = sdiv i32 %53, 2
  store i32 %54, ptr %18, align 4
  %55 = load i32, ptr %3, align 4
  %56 = sdiv i32 %55, 2
  store i32 %56, ptr %19, align 4
  %57 = load i32, ptr %19, align 4
  %58 = mul nsw i32 %57, 2
  store i32 %58, ptr %20, align 4
  %59 = load i32, ptr %6, align 4
  %60 = load i32, ptr %8, align 4
  %61 = add nsw i32 %59, %60
  %62 = load i32, ptr %10, align 4
  %63 = add nsw i32 %61, %62
  %64 = load i32, ptr %12, align 4
  %65 = add nsw i32 %63, %64
  %66 = load i32, ptr %14, align 4
  %67 = add nsw i32 %65, %66
  %68 = load i32, ptr %16, align 4
  %69 = add nsw i32 %67, %68
  %70 = load i32, ptr %18, align 4
  %71 = add nsw i32 %69, %70
  %72 = load i32, ptr %20, align 4
  %73 = add nsw i32 %71, %72
  ret i32 %73
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
