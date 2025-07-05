; ModuleID = 'test/LFtest.optimized.bc'
source_filename = "LFtest.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_fusion() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  br label %3

3:                                                ; preds = %6, %0
  %.0 = phi i32 [ 0, %0 ], [ %10, %6 ]
  %4 = icmp slt i32 %.0, 100
  %5 = icmp slt i32 %.0, 100
  br i1 %4, label %12, label %17

6:                                                ; preds = %11
  %7 = mul nsw i32 %.0, 2
  %8 = sext i32 %.0 to i64
  %9 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %8
  store i32 %7, ptr %9, align 4
  %10 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !6

11:                                               ; preds = %12
  br i1 %5, label %6, label %17

12:                                               ; preds = %3
  %13 = add nsw i32 %.0, 5
  %14 = sext i32 %.0 to i64
  %15 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %14
  store i32 %13, ptr %15, align 4
  %16 = add nsw i32 %.0, 1
  br label %11, !llvm.loop !8

17:                                               ; preds = %3, %11
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_non_adjacent() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  br label %3

3:                                                ; preds = %5, %0
  %.0 = phi i32 [ 0, %0 ], [ %9, %5 ]
  %4 = icmp slt i32 %.0, 100
  br i1 %4, label %5, label %10

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  %9 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !9

10:                                               ; preds = %3
  %11 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 0
  %12 = load i32, ptr %11, align 4
  %13 = icmp sgt i32 %12, 0
  br i1 %13, label %14, label %16

14:                                               ; preds = %10
  %15 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 1
  store i32 42, ptr %15, align 4
  br label %18

16:                                               ; preds = %10
  %17 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 1
  store i32 24, ptr %17, align 4
  br label %18

18:                                               ; preds = %16, %14
  br label %19

19:                                               ; preds = %21, %18
  %.01 = phi i32 [ 0, %18 ], [ %25, %21 ]
  %20 = icmp slt i32 %.01, 100
  br i1 %20, label %21, label %26

21:                                               ; preds = %19
  %22 = add nsw i32 %.01, 5
  %23 = sext i32 %.01 to i64
  %24 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %23
  store i32 %22, ptr %24, align 4
  %25 = add nsw i32 %.01, 1
  br label %19, !llvm.loop !10

26:                                               ; preds = %19
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_different_bounds() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [50 x i32], align 4
  br label %3

3:                                                ; preds = %5, %0
  %.0 = phi i32 [ 0, %0 ], [ %9, %5 ]
  %4 = icmp slt i32 %.0, 100
  br i1 %4, label %5, label %.preheader

.preheader:                                       ; preds = %3
  br label %10

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  %9 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !11

10:                                               ; preds = %12, %.preheader
  %.01 = phi i32 [ %16, %12 ], [ 0, %.preheader ]
  %11 = icmp slt i32 %.01, 50
  br i1 %11, label %12, label %17

12:                                               ; preds = %10
  %13 = add nsw i32 %.01, 5
  %14 = sext i32 %.01 to i64
  %15 = getelementptr inbounds [50 x i32], ptr %2, i64 0, i64 %14
  store i32 %13, ptr %15, align 4
  %16 = add nsw i32 %.01, 1
  br label %10, !llvm.loop !12

17:                                               ; preds = %10
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_different_types() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x double], align 8
  br label %3

3:                                                ; preds = %5, %0
  %.0 = phi i32 [ 0, %0 ], [ %9, %5 ]
  %4 = icmp slt i32 %.0, 100
  br i1 %4, label %5, label %.preheader

.preheader:                                       ; preds = %3
  br label %10

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  %9 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !13

10:                                               ; preds = %12, %.preheader
  %.01 = phi double [ %17, %12 ], [ 0.000000e+00, %.preheader ]
  %11 = fcmp olt double %.01, 1.000000e+02
  br i1 %11, label %12, label %18

12:                                               ; preds = %10
  %13 = fadd double %.01, 5.000000e+00
  %14 = fptosi double %.01 to i32
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds [100 x double], ptr %2, i64 0, i64 %15
  store double %13, ptr %16, align 8
  %17 = fadd double %.01, 1.000000e+00
  br label %10, !llvm.loop !14

18:                                               ; preds = %10
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_different_step() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  br label %3

3:                                                ; preds = %5, %0
  %.0 = phi i32 [ 0, %0 ], [ %9, %5 ]
  %4 = icmp slt i32 %.0, 100
  br i1 %4, label %5, label %.preheader

.preheader:                                       ; preds = %3
  br label %10

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  %9 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !15

10:                                               ; preds = %12, %.preheader
  %.01 = phi i32 [ %16, %12 ], [ 0, %.preheader ]
  %11 = icmp slt i32 %.01, 100
  br i1 %11, label %12, label %17

12:                                               ; preds = %10
  %13 = add nsw i32 %.01, 5
  %14 = sext i32 %.01 to i64
  %15 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %14
  store i32 %13, ptr %15, align 4
  %16 = add nsw i32 %.01, 2
  br label %10, !llvm.loop !16

17:                                               ; preds = %10
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_not_control_flow_equivalent() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  %3 = alloca i32, align 4
  store volatile i32 1, ptr %3, align 4
  br label %4

4:                                                ; preds = %6, %0
  %.0 = phi i32 [ 0, %0 ], [ %10, %6 ]
  %5 = icmp slt i32 %.0, 100
  br i1 %5, label %6, label %11

6:                                                ; preds = %4
  %7 = mul nsw i32 %.0, 2
  %8 = sext i32 %.0 to i64
  %9 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %8
  store i32 %7, ptr %9, align 4
  %10 = add nsw i32 %.0, 1
  br label %4, !llvm.loop !17

11:                                               ; preds = %4
  %12 = load volatile i32, ptr %3, align 4
  %13 = icmp ne i32 %12, 0
  br i1 %13, label %.preheader, label %24

.preheader:                                       ; preds = %11
  br label %14

14:                                               ; preds = %16, %.preheader
  %.1 = phi i32 [ %23, %16 ], [ 0, %.preheader ]
  %15 = icmp slt i32 %.1, 100
  br i1 %15, label %16, label %.loopexit

16:                                               ; preds = %14
  %17 = sext i32 %.1 to i64
  %18 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %17
  %19 = load i32, ptr %18, align 4
  %20 = add nsw i32 %19, 5
  %21 = sext i32 %.1 to i64
  %22 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %21
  store i32 %20, ptr %22, align 4
  %23 = add nsw i32 %.1, 1
  br label %14, !llvm.loop !18

.loopexit:                                        ; preds = %14
  br label %24

24:                                               ; preds = %.loopexit, %11
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_negative_distance() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  br label %3

3:                                                ; preds = %5, %0
  %.0 = phi i32 [ 0, %0 ], [ %9, %5 ]
  %4 = icmp slt i32 %.0, 98
  br i1 %4, label %5, label %.preheader

.preheader:                                       ; preds = %3
  br label %10

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  %9 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !19

10:                                               ; preds = %12, %.preheader
  %.01 = phi i32 [ %20, %12 ], [ 0, %.preheader ]
  %11 = icmp slt i32 %.01, 98
  br i1 %11, label %12, label %21

12:                                               ; preds = %10
  %13 = add nsw i32 %.01, 3
  %14 = sext i32 %13 to i64
  %15 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %14
  %16 = load i32, ptr %15, align 4
  %17 = add nsw i32 %16, 1
  %18 = sext i32 %.01 to i64
  %19 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %18
  store i32 %17, ptr %19, align 4
  %20 = add nsw i32 %.01, 1
  br label %10, !llvm.loop !20

21:                                               ; preds = %10
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_good_distance() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  br label %3

3:                                                ; preds = %6, %0
  %.0 = phi i32 [ 0, %0 ], [ %10, %6 ]
  %4 = icmp slt i32 %.0, 98
  %5 = icmp slt i32 %.0, 98
  br i1 %4, label %12, label %20

6:                                                ; preds = %11
  %7 = mul nsw i32 %.0, 2
  %8 = sext i32 %.0 to i64
  %9 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %8
  store i32 %7, ptr %9, align 4
  %10 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !21

11:                                               ; preds = %12
  br i1 %5, label %6, label %20

12:                                               ; preds = %3
  %13 = sext i32 %.0 to i64
  %14 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %13
  %15 = load i32, ptr %14, align 4
  %16 = add nsw i32 %15, 1
  %17 = sext i32 %.0 to i64
  %18 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %17
  store i32 %16, ptr %18, align 4
  %19 = add nsw i32 %.0, 1
  br label %11, !llvm.loop !22

20:                                               ; preds = %3, %11
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
