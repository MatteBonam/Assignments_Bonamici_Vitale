; ModuleID = 'test/LFtest.optimized.bc'
source_filename = "LFtest.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_no_fusion() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  br label %3

3:                                                ; preds = %12, %0
  %.0 = phi i32 [ 0, %0 ], [ %13, %12 ]
  %4 = icmp slt i32 %.0, 100
  br i1 %4, label %5, label %14

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  %9 = add nsw i32 %.0, 5
  %10 = sext i32 %.0 to i64
  %11 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %10
  store i32 %9, ptr %11, align 4
  br label %12

12:                                               ; preds = %5
  %13 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !6

14:                                               ; preds = %3
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_fusion_direct() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  br label %3

3:                                                ; preds = %9, %0
  %.0 = phi i32 [ 0, %0 ], [ %10, %9 ]
  %4 = icmp slt i32 %.0, 100
  br i1 %4, label %5, label %11

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  br label %9

9:                                                ; preds = %5
  %10 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !8

11:                                               ; preds = %3
  br label %12

12:                                               ; preds = %18, %11
  %.01 = phi i32 [ 0, %11 ], [ %19, %18 ]
  %13 = icmp slt i32 %.01, 100
  br i1 %13, label %14, label %20

14:                                               ; preds = %12
  %15 = add nsw i32 %.01, 5
  %16 = sext i32 %.01 to i64
  %17 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %16
  store i32 %15, ptr %17, align 4
  br label %18

18:                                               ; preds = %14
  %19 = add nsw i32 %.01, 1
  br label %12, !llvm.loop !9

20:                                               ; preds = %12
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_guard_case() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  %3 = alloca i32, align 4
  store volatile i32 1, ptr %3, align 4
  %4 = load volatile i32, ptr %3, align 4
  %5 = icmp ne i32 %4, 0
  br i1 %5, label %6, label %16

6:                                                ; preds = %0
  br label %7

7:                                                ; preds = %13, %6
  %.0 = phi i32 [ 0, %6 ], [ %14, %13 ]
  %8 = icmp slt i32 %.0, 100
  br i1 %8, label %9, label %15

9:                                                ; preds = %7
  %10 = mul nsw i32 %.0, 2
  %11 = sext i32 %.0 to i64
  %12 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %11
  store i32 %10, ptr %12, align 4
  br label %13

13:                                               ; preds = %9
  %14 = add nsw i32 %.0, 1
  br label %7, !llvm.loop !10

15:                                               ; preds = %7
  br label %26

16:                                               ; preds = %0
  br label %17

17:                                               ; preds = %23, %16
  %.01 = phi i32 [ 0, %16 ], [ %24, %23 ]
  %18 = icmp slt i32 %.01, 100
  br i1 %18, label %19, label %25

19:                                               ; preds = %17
  %20 = add nsw i32 %.01, 5
  %21 = sext i32 %.01 to i64
  %22 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %21
  store i32 %20, ptr %22, align 4
  br label %23

23:                                               ; preds = %19
  %24 = add nsw i32 %.01, 1
  br label %17, !llvm.loop !11

25:                                               ; preds = %17
  br label %26

26:                                               ; preds = %25, %15
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_non_adjacent() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  br label %3

3:                                                ; preds = %9, %0
  %.0 = phi i32 [ 0, %0 ], [ %10, %9 ]
  %4 = icmp slt i32 %.0, 100
  br i1 %4, label %5, label %11

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  br label %9

9:                                                ; preds = %5
  %10 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !12

11:                                               ; preds = %3
  %12 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 0
  %13 = load i32, ptr %12, align 4
  %14 = icmp sgt i32 %13, 0
  br i1 %14, label %15, label %17

15:                                               ; preds = %11
  %16 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 1
  store i32 42, ptr %16, align 4
  br label %19

17:                                               ; preds = %11
  %18 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 1
  store i32 24, ptr %18, align 4
  br label %19

19:                                               ; preds = %17, %15
  br label %20

20:                                               ; preds = %26, %19
  %.01 = phi i32 [ 0, %19 ], [ %27, %26 ]
  %21 = icmp slt i32 %.01, 100
  br i1 %21, label %22, label %28

22:                                               ; preds = %20
  %23 = add nsw i32 %.01, 5
  %24 = sext i32 %.01 to i64
  %25 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %24
  store i32 %23, ptr %25, align 4
  br label %26

26:                                               ; preds = %22
  %27 = add nsw i32 %.01, 1
  br label %20, !llvm.loop !13

28:                                               ; preds = %20
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_different_bounds() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [50 x i32], align 4
  br label %3

3:                                                ; preds = %9, %0
  %.0 = phi i32 [ 0, %0 ], [ %10, %9 ]
  %4 = icmp slt i32 %.0, 100
  br i1 %4, label %5, label %11

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  br label %9

9:                                                ; preds = %5
  %10 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !14

11:                                               ; preds = %3
  br label %12

12:                                               ; preds = %18, %11
  %.01 = phi i32 [ 0, %11 ], [ %19, %18 ]
  %13 = icmp slt i32 %.01, 50
  br i1 %13, label %14, label %20

14:                                               ; preds = %12
  %15 = add nsw i32 %.01, 5
  %16 = sext i32 %.01 to i64
  %17 = getelementptr inbounds [50 x i32], ptr %2, i64 0, i64 %16
  store i32 %15, ptr %17, align 4
  br label %18

18:                                               ; preds = %14
  %19 = add nsw i32 %.01, 1
  br label %12, !llvm.loop !15

20:                                               ; preds = %12
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_different_types() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x double], align 8
  br label %3

3:                                                ; preds = %9, %0
  %.0 = phi i32 [ 0, %0 ], [ %10, %9 ]
  %4 = icmp slt i32 %.0, 100
  br i1 %4, label %5, label %11

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  br label %9

9:                                                ; preds = %5
  %10 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !16

11:                                               ; preds = %3
  br label %12

12:                                               ; preds = %19, %11
  %.01 = phi double [ 0.000000e+00, %11 ], [ %20, %19 ]
  %13 = fcmp olt double %.01, 1.000000e+02
  br i1 %13, label %14, label %21

14:                                               ; preds = %12
  %15 = fadd double %.01, 5.000000e+00
  %16 = fptosi double %.01 to i32
  %17 = sext i32 %16 to i64
  %18 = getelementptr inbounds [100 x double], ptr %2, i64 0, i64 %17
  store double %15, ptr %18, align 8
  br label %19

19:                                               ; preds = %14
  %20 = fadd double %.01, 1.000000e+00
  br label %12, !llvm.loop !17

21:                                               ; preds = %12
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_different_step() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  br label %3

3:                                                ; preds = %9, %0
  %.0 = phi i32 [ 0, %0 ], [ %10, %9 ]
  %4 = icmp slt i32 %.0, 100
  br i1 %4, label %5, label %11

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  br label %9

9:                                                ; preds = %5
  %10 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !18

11:                                               ; preds = %3
  br label %12

12:                                               ; preds = %18, %11
  %.01 = phi i32 [ 0, %11 ], [ %19, %18 ]
  %13 = icmp slt i32 %.01, 100
  br i1 %13, label %14, label %20

14:                                               ; preds = %12
  %15 = add nsw i32 %.01, 5
  %16 = sext i32 %.01 to i64
  %17 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %16
  store i32 %15, ptr %17, align 4
  br label %18

18:                                               ; preds = %14
  %19 = add nsw i32 %.01, 2
  br label %12, !llvm.loop !19

20:                                               ; preds = %12
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_control_flow_equivalent() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  br label %3

3:                                                ; preds = %9, %0
  %.0 = phi i32 [ 0, %0 ], [ %10, %9 ]
  %4 = icmp slt i32 %.0, 100
  br i1 %4, label %5, label %11

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  br label %9

9:                                                ; preds = %5
  %10 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !20

11:                                               ; preds = %3
  br label %12

12:                                               ; preds = %21, %11
  %.1 = phi i32 [ 0, %11 ], [ %22, %21 ]
  %13 = icmp slt i32 %.1, 100
  br i1 %13, label %14, label %23

14:                                               ; preds = %12
  %15 = sext i32 %.1 to i64
  %16 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %15
  %17 = load i32, ptr %16, align 4
  %18 = add nsw i32 %17, 5
  %19 = sext i32 %.1 to i64
  %20 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %19
  store i32 %18, ptr %20, align 4
  br label %21

21:                                               ; preds = %14
  %22 = add nsw i32 %.1, 1
  br label %12, !llvm.loop !21

23:                                               ; preds = %12
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_not_control_flow_equivalent() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  %3 = alloca i32, align 4
  store volatile i32 1, ptr %3, align 4
  br label %4

4:                                                ; preds = %10, %0
  %.0 = phi i32 [ 0, %0 ], [ %11, %10 ]
  %5 = icmp slt i32 %.0, 100
  br i1 %5, label %6, label %12

6:                                                ; preds = %4
  %7 = mul nsw i32 %.0, 2
  %8 = sext i32 %.0 to i64
  %9 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %8
  store i32 %7, ptr %9, align 4
  br label %10

10:                                               ; preds = %6
  %11 = add nsw i32 %.0, 1
  br label %4, !llvm.loop !22

12:                                               ; preds = %4
  %13 = load volatile i32, ptr %3, align 4
  %14 = icmp ne i32 %13, 0
  br i1 %14, label %15, label %28

15:                                               ; preds = %12
  br label %16

16:                                               ; preds = %25, %15
  %.1 = phi i32 [ 0, %15 ], [ %26, %25 ]
  %17 = icmp slt i32 %.1, 100
  br i1 %17, label %18, label %27

18:                                               ; preds = %16
  %19 = sext i32 %.1 to i64
  %20 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %19
  %21 = load i32, ptr %20, align 4
  %22 = add nsw i32 %21, 5
  %23 = sext i32 %.1 to i64
  %24 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %23
  store i32 %22, ptr %24, align 4
  br label %25

25:                                               ; preds = %18
  %26 = add nsw i32 %.1, 1
  br label %16, !llvm.loop !23

27:                                               ; preds = %16
  br label %28

28:                                               ; preds = %27, %12
  ret void
}

attributes #0 = { noinline nounwind ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a,+zcm,+zcz" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 15, i32 4]}
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
!23 = distinct !{!23, !7}
