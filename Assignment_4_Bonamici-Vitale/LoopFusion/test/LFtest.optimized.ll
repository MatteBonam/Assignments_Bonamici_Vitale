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
define void @test_fusion() #0 {
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
  br label %3, !llvm.loop !10

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
  br label %20, !llvm.loop !11

28:                                               ; preds = %20
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
