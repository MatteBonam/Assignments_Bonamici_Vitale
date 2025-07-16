; ModuleID = 'LFtest.c'
source_filename = "LFtest.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_fusion() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  br label %5

5:                                                ; preds = %14, %0
  %6 = load i32, ptr %3, align 4
  %7 = icmp slt i32 %6, 100
  br i1 %7, label %8, label %17

8:                                                ; preds = %5
  %9 = load i32, ptr %3, align 4
  %10 = mul nsw i32 %9, 2
  %11 = load i32, ptr %3, align 4
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %12
  store i32 %10, ptr %13, align 4
  br label %14

14:                                               ; preds = %8
  %15 = load i32, ptr %3, align 4
  %16 = add nsw i32 %15, 1
  store i32 %16, ptr %3, align 4
  br label %5, !llvm.loop !6

17:                                               ; preds = %5
  store i32 0, ptr %4, align 4
  br label %18

18:                                               ; preds = %27, %17
  %19 = load i32, ptr %4, align 4
  %20 = icmp slt i32 %19, 100
  br i1 %20, label %21, label %30

21:                                               ; preds = %18
  %22 = load i32, ptr %4, align 4
  %23 = add nsw i32 %22, 5
  %24 = load i32, ptr %4, align 4
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %25
  store i32 %23, ptr %26, align 4
  br label %27

27:                                               ; preds = %21
  %28 = load i32, ptr %4, align 4
  %29 = add nsw i32 %28, 1
  store i32 %29, ptr %4, align 4
  br label %18, !llvm.loop !8

30:                                               ; preds = %18
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_non_adjacent() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  br label %5

5:                                                ; preds = %14, %0
  %6 = load i32, ptr %3, align 4
  %7 = icmp slt i32 %6, 100
  br i1 %7, label %8, label %17

8:                                                ; preds = %5
  %9 = load i32, ptr %3, align 4
  %10 = mul nsw i32 %9, 2
  %11 = load i32, ptr %3, align 4
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %12
  store i32 %10, ptr %13, align 4
  br label %14

14:                                               ; preds = %8
  %15 = load i32, ptr %3, align 4
  %16 = add nsw i32 %15, 1
  store i32 %16, ptr %3, align 4
  br label %5, !llvm.loop !9

17:                                               ; preds = %5
  %18 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 0
  %19 = load i32, ptr %18, align 4
  %20 = icmp sgt i32 %19, 0
  br i1 %20, label %21, label %23

21:                                               ; preds = %17
  %22 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 1
  store i32 42, ptr %22, align 4
  br label %25

23:                                               ; preds = %17
  %24 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 1
  store i32 24, ptr %24, align 4
  br label %25

25:                                               ; preds = %23, %21
  store i32 0, ptr %4, align 4
  br label %26

26:                                               ; preds = %35, %25
  %27 = load i32, ptr %4, align 4
  %28 = icmp slt i32 %27, 100
  br i1 %28, label %29, label %38

29:                                               ; preds = %26
  %30 = load i32, ptr %4, align 4
  %31 = add nsw i32 %30, 5
  %32 = load i32, ptr %4, align 4
  %33 = sext i32 %32 to i64
  %34 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %33
  store i32 %31, ptr %34, align 4
  br label %35

35:                                               ; preds = %29
  %36 = load i32, ptr %4, align 4
  %37 = add nsw i32 %36, 1
  store i32 %37, ptr %4, align 4
  br label %26, !llvm.loop !10

38:                                               ; preds = %26
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_different_bounds() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [50 x i32], align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  br label %5

5:                                                ; preds = %14, %0
  %6 = load i32, ptr %3, align 4
  %7 = icmp slt i32 %6, 100
  br i1 %7, label %8, label %17

8:                                                ; preds = %5
  %9 = load i32, ptr %3, align 4
  %10 = mul nsw i32 %9, 2
  %11 = load i32, ptr %3, align 4
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %12
  store i32 %10, ptr %13, align 4
  br label %14

14:                                               ; preds = %8
  %15 = load i32, ptr %3, align 4
  %16 = add nsw i32 %15, 1
  store i32 %16, ptr %3, align 4
  br label %5, !llvm.loop !11

17:                                               ; preds = %5
  store i32 0, ptr %4, align 4
  br label %18

18:                                               ; preds = %30, %17
  %19 = load i32, ptr %4, align 4
  %20 = icmp slt i32 %19, 50
  br i1 %20, label %21, label %33

21:                                               ; preds = %18
  %22 = load i32, ptr %4, align 4
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %23
  %25 = load i32, ptr %24, align 4
  %26 = add nsw i32 %25, 4
  %27 = load i32, ptr %4, align 4
  %28 = sext i32 %27 to i64
  %29 = getelementptr inbounds [50 x i32], ptr %2, i64 0, i64 %28
  store i32 %26, ptr %29, align 4
  br label %30

30:                                               ; preds = %21
  %31 = load i32, ptr %4, align 4
  %32 = add nsw i32 %31, 1
  store i32 %32, ptr %4, align 4
  br label %18, !llvm.loop !12

33:                                               ; preds = %18
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_different_types() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x double], align 8
  %3 = alloca i32, align 4
  %4 = alloca double, align 8
  store i32 0, ptr %3, align 4
  br label %5

5:                                                ; preds = %14, %0
  %6 = load i32, ptr %3, align 4
  %7 = icmp slt i32 %6, 100
  br i1 %7, label %8, label %17

8:                                                ; preds = %5
  %9 = load i32, ptr %3, align 4
  %10 = mul nsw i32 %9, 2
  %11 = load i32, ptr %3, align 4
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %12
  store i32 %10, ptr %13, align 4
  br label %14

14:                                               ; preds = %8
  %15 = load i32, ptr %3, align 4
  %16 = add nsw i32 %15, 1
  store i32 %16, ptr %3, align 4
  br label %5, !llvm.loop !13

17:                                               ; preds = %5
  store double 0.000000e+00, ptr %4, align 8
  br label %18

18:                                               ; preds = %28, %17
  %19 = load double, ptr %4, align 8
  %20 = fcmp olt double %19, 1.000000e+02
  br i1 %20, label %21, label %31

21:                                               ; preds = %18
  %22 = load double, ptr %4, align 8
  %23 = fadd double %22, 5.000000e+00
  %24 = load double, ptr %4, align 8
  %25 = fptosi double %24 to i32
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds [100 x double], ptr %2, i64 0, i64 %26
  store double %23, ptr %27, align 8
  br label %28

28:                                               ; preds = %21
  %29 = load double, ptr %4, align 8
  %30 = fadd double %29, 1.000000e+00
  store double %30, ptr %4, align 8
  br label %18, !llvm.loop !14

31:                                               ; preds = %18
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_different_step() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  br label %5

5:                                                ; preds = %14, %0
  %6 = load i32, ptr %3, align 4
  %7 = icmp slt i32 %6, 100
  br i1 %7, label %8, label %17

8:                                                ; preds = %5
  %9 = load i32, ptr %3, align 4
  %10 = mul nsw i32 %9, 2
  %11 = load i32, ptr %3, align 4
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %12
  store i32 %10, ptr %13, align 4
  br label %14

14:                                               ; preds = %8
  %15 = load i32, ptr %3, align 4
  %16 = add nsw i32 %15, 1
  store i32 %16, ptr %3, align 4
  br label %5, !llvm.loop !15

17:                                               ; preds = %5
  store i32 0, ptr %4, align 4
  br label %18

18:                                               ; preds = %30, %17
  %19 = load i32, ptr %4, align 4
  %20 = icmp slt i32 %19, 100
  br i1 %20, label %21, label %33

21:                                               ; preds = %18
  %22 = load i32, ptr %4, align 4
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %23
  %25 = load i32, ptr %24, align 4
  %26 = add nsw i32 %25, 5
  %27 = load i32, ptr %4, align 4
  %28 = sext i32 %27 to i64
  %29 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %28
  store i32 %26, ptr %29, align 4
  br label %30

30:                                               ; preds = %21
  %31 = load i32, ptr %4, align 4
  %32 = add nsw i32 %31, 2
  store i32 %32, ptr %4, align 4
  br label %18, !llvm.loop !16

33:                                               ; preds = %18
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_not_control_flow_equivalent() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store volatile i32 1, ptr %4, align 4
  store i32 0, ptr %3, align 4
  br label %5

5:                                                ; preds = %14, %0
  %6 = load i32, ptr %3, align 4
  %7 = icmp slt i32 %6, 100
  br i1 %7, label %8, label %17

8:                                                ; preds = %5
  %9 = load i32, ptr %3, align 4
  %10 = mul nsw i32 %9, 2
  %11 = load i32, ptr %3, align 4
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %12
  store i32 %10, ptr %13, align 4
  br label %14

14:                                               ; preds = %8
  %15 = load i32, ptr %3, align 4
  %16 = add nsw i32 %15, 1
  store i32 %16, ptr %3, align 4
  br label %5, !llvm.loop !17

17:                                               ; preds = %5
  %18 = load volatile i32, ptr %4, align 4
  %19 = icmp ne i32 %18, 0
  br i1 %19, label %20, label %37

20:                                               ; preds = %17
  store i32 0, ptr %3, align 4
  br label %21

21:                                               ; preds = %33, %20
  %22 = load i32, ptr %3, align 4
  %23 = icmp slt i32 %22, 100
  br i1 %23, label %24, label %36

24:                                               ; preds = %21
  %25 = load i32, ptr %3, align 4
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %26
  %28 = load i32, ptr %27, align 4
  %29 = add nsw i32 %28, 5
  %30 = load i32, ptr %3, align 4
  %31 = sext i32 %30 to i64
  %32 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %31
  store i32 %29, ptr %32, align 4
  br label %33

33:                                               ; preds = %24
  %34 = load i32, ptr %3, align 4
  %35 = add nsw i32 %34, 1
  store i32 %35, ptr %3, align 4
  br label %21, !llvm.loop !18

36:                                               ; preds = %21
  br label %37

37:                                               ; preds = %36, %17
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_negative_distance() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  br label %5

5:                                                ; preds = %14, %0
  %6 = load i32, ptr %3, align 4
  %7 = icmp slt i32 %6, 98
  br i1 %7, label %8, label %17

8:                                                ; preds = %5
  %9 = load i32, ptr %3, align 4
  %10 = mul nsw i32 %9, 2
  %11 = load i32, ptr %3, align 4
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %12
  store i32 %10, ptr %13, align 4
  br label %14

14:                                               ; preds = %8
  %15 = load i32, ptr %3, align 4
  %16 = add nsw i32 %15, 1
  store i32 %16, ptr %3, align 4
  br label %5, !llvm.loop !19

17:                                               ; preds = %5
  store i32 0, ptr %4, align 4
  br label %18

18:                                               ; preds = %31, %17
  %19 = load i32, ptr %4, align 4
  %20 = icmp slt i32 %19, 98
  br i1 %20, label %21, label %34

21:                                               ; preds = %18
  %22 = load i32, ptr %4, align 4
  %23 = add nsw i32 %22, 3
  %24 = sext i32 %23 to i64
  %25 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %24
  %26 = load i32, ptr %25, align 4
  %27 = add nsw i32 %26, 1
  %28 = load i32, ptr %4, align 4
  %29 = sext i32 %28 to i64
  %30 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %29
  store i32 %27, ptr %30, align 4
  br label %31

31:                                               ; preds = %21
  %32 = load i32, ptr %4, align 4
  %33 = add nsw i32 %32, 1
  store i32 %33, ptr %4, align 4
  br label %18, !llvm.loop !20

34:                                               ; preds = %18
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_good_distance() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  br label %5

5:                                                ; preds = %14, %0
  %6 = load i32, ptr %3, align 4
  %7 = icmp slt i32 %6, 98
  br i1 %7, label %8, label %17

8:                                                ; preds = %5
  %9 = load i32, ptr %3, align 4
  %10 = mul nsw i32 %9, 2
  %11 = load i32, ptr %3, align 4
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %12
  store i32 %10, ptr %13, align 4
  br label %14

14:                                               ; preds = %8
  %15 = load i32, ptr %3, align 4
  %16 = add nsw i32 %15, 1
  store i32 %16, ptr %3, align 4
  br label %5, !llvm.loop !21

17:                                               ; preds = %5
  store i32 0, ptr %4, align 4
  br label %18

18:                                               ; preds = %30, %17
  %19 = load i32, ptr %4, align 4
  %20 = icmp slt i32 %19, 98
  br i1 %20, label %21, label %33

21:                                               ; preds = %18
  %22 = load i32, ptr %4, align 4
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %23
  %25 = load i32, ptr %24, align 4
  %26 = add nsw i32 %25, 1
  %27 = load i32, ptr %4, align 4
  %28 = sext i32 %27 to i64
  %29 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %28
  store i32 %26, ptr %29, align 4
  br label %30

30:                                               ; preds = %21
  %31 = load i32, ptr %4, align 4
  %32 = add nsw i32 %31, 1
  store i32 %32, ptr %4, align 4
  br label %18, !llvm.loop !22

33:                                               ; preds = %18
  ret void
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
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
!10 = distinct !{!10, !7}
!11 = distinct !{!11, !7}
!12 = distinct !{!12, !7}
!13 = distinct !{!13, !7}
!14 = distinct !{!14, !7}
!15 = distinct !{!15, !7}
!16 = distinct !{!16, !7}
!17 = distinct !{!17, !7}
!18 = distinct !{!18, !7}
!19 = distinct !{!19, !7}
!20 = distinct !{!20, !7}
!21 = distinct !{!21, !7}
!22 = distinct !{!22, !7}
