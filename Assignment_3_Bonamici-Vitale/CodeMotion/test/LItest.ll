; ModuleID = 'LItest.c'
source_filename = "LItest.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @test(i32 noundef %0, i32 noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  store i32 %1, ptr %4, align 4
  %10 = load i32, ptr %4, align 4
  %11 = add nsw i32 5, %10
  store i32 %11, ptr %5, align 4
  store i32 0, ptr %9, align 4
  br label %12

12:                                               ; preds = %27, %2
  %13 = load i32, ptr %9, align 4
  %14 = icmp slt i32 %13, 5
  br i1 %14, label %15, label %34

15:                                               ; preds = %12
  %16 = load i32, ptr %5, align 4
  %17 = load i32, ptr %3, align 4
  %18 = add nsw i32 %16, %17
  store i32 %18, ptr %6, align 4
  %19 = load i32, ptr %9, align 4
  %20 = icmp slt i32 %19, 3
  br i1 %20, label %21, label %24

21:                                               ; preds = %15
  %22 = load i32, ptr %5, align 4
  %23 = add nsw i32 %22, 3
  store i32 %23, ptr %8, align 4
  br label %35

24:                                               ; preds = %15
  %25 = load i32, ptr %6, align 4
  %26 = add nsw i32 %25, 4
  store i32 %26, ptr %8, align 4
  br label %27

27:                                               ; preds = %24
  %28 = load i32, ptr %6, align 4
  %29 = add nsw i32 %28, 1
  store i32 %29, ptr %7, align 4
  %30 = load i32, ptr %8, align 4
  %31 = add nsw i32 %30, 2
  store i32 %31, ptr %4, align 4
  %32 = load i32, ptr %9, align 4
  %33 = add nsw i32 %32, 1
  store i32 %33, ptr %9, align 4
  br label %12, !llvm.loop !6

34:                                               ; preds = %12
  br label %35

35:                                               ; preds = %34, %21
  %36 = load i32, ptr %8, align 4
  %37 = load i32, ptr %4, align 4
  %38 = add nsw i32 %36, %37
  ret i32 %38
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @fun(i32 noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  store i32 %0, ptr %4, align 4
  store i32 %1, ptr %5, align 4
  store i32 %2, ptr %6, align 4
  store i32 0, ptr %7, align 4
  store i32 1, ptr %10, align 4
  br label %11

11:                                               ; preds = %3, %21
  %12 = load i32, ptr %7, align 4
  %13 = icmp slt i32 %12, 5
  br i1 %13, label %14, label %18

14:                                               ; preds = %11
  %15 = load i32, ptr %5, align 4
  %16 = load i32, ptr %6, align 4
  %17 = add nsw i32 %15, %16
  store i32 %17, ptr %4, align 4
  br label %21

18:                                               ; preds = %11
  %19 = load i32, ptr %5, align 4
  %20 = add nsw i32 %19, 1
  store i32 %20, ptr %9, align 4
  br label %26

21:                                               ; preds = %14
  %22 = load i32, ptr %4, align 4
  %23 = add nsw i32 %22, 1
  store i32 %23, ptr %8, align 4
  %24 = load i32, ptr %7, align 4
  %25 = add nsw i32 %24, 1
  store i32 %25, ptr %7, align 4
  br label %11

26:                                               ; preds = %18
  %27 = load i32, ptr %4, align 4
  store i32 %27, ptr %6, align 4
  %28 = load i32, ptr %9, align 4
  store i32 %28, ptr %8, align 4
  %29 = load i32, ptr %6, align 4
  ret i32 %29
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  %3 = call i32 @fun(i32 noundef 1, i32 noundef 2, i32 noundef 3)
  store i32 %3, ptr %2, align 4
  %4 = load i32, ptr %2, align 4
  ret i32 %4
}

attributes #0 = { noinline nounwind ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a,+zcm,+zcz" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 15, i32 5]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!"Homebrew clang version 19.1.7"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
