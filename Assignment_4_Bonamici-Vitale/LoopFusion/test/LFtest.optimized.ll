; ModuleID = 'test/LFtest.optimized.bc'
source_filename = "LFtest.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_no_fusion() #0 {
  %1 = alloca [100 x i32], align 4
  %2 = alloca [100 x i32], align 4
  br label %3

3:                                                ; preds = %5, %0
  %.0 = phi i32 [ 0, %0 ], [ %12, %5 ]
  %4 = icmp slt i32 %.0, 100
  br i1 %4, label %5, label %13

5:                                                ; preds = %3
  %6 = mul nsw i32 %.0, 2
  %7 = sext i32 %.0 to i64
  %8 = getelementptr inbounds [100 x i32], ptr %1, i64 0, i64 %7
  store i32 %6, ptr %8, align 4
  %9 = add nsw i32 %.0, 5
  %10 = sext i32 %.0 to i64
  %11 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %10
  store i32 %9, ptr %11, align 4
  %12 = add nsw i32 %.0, 1
  br label %3, !llvm.loop !6

13:                                               ; preds = %3
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable(sync)
define void @test_fusion() #0 {
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
  br label %3, !llvm.loop !8

10:                                               ; preds = %12, %.preheader
  %.01 = phi i32 [ %16, %12 ], [ 0, %.preheader ]
  %11 = icmp slt i32 %.01, 100
  br i1 %11, label %12, label %17

12:                                               ; preds = %10
  %13 = add nsw i32 %.01, 5
  %14 = sext i32 %.01 to i64
  %15 = getelementptr inbounds [100 x i32], ptr %2, i64 0, i64 %14
  store i32 %13, ptr %15, align 4
  %16 = add nsw i32 %.01, 1
  br label %10, !llvm.loop !9

17:                                               ; preds = %10
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
